#include "mainwindow.h"



MainWindow::MainWindow()
{
    iterationTimer = new TimerThread(iterationFPS, this);
    updateTimer = new TimerThread(updateFPS, this);

    factory = new Factory();

    renderManager = new RenderManager(factory);

    nodeManager = new NodeManager(factory);

    overlay = new Overlay();

    morphoWidget = new MorphoWidget(renderManager->texWidth(), renderManager->texHeight(), overlay);
    morphoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    morphoWidget->setMinimumSize(0, 0);

    plotsWidget = new PlotsWidget(renderManager->texWidth(), renderManager->texHeight());
    plotsWidget->setVisible(false);

    plotsWidgetOpacityEffect = new QGraphicsOpacityEffect(plotsWidget);
    plotsWidgetOpacityEffect->setOpacity(opacity);
    plotsWidget->setGraphicsEffect(plotsWidgetOpacityEffect);

    controlWidget = new ControlWidget(iterationFPS, updateFPS, factory, nodeManager, renderManager, plotsWidget);
    controlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    controlWidget->setMinimumSize(0, 0);

    controlWidgetOpacityEffect = new QGraphicsOpacityEffect(controlWidget);
    controlWidgetOpacityEffect->setOpacity(opacity);
    controlWidget->setGraphicsEffect(controlWidgetOpacityEffect);

    stackedWidget = new QWidget(this);
    stackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    stackedLayout = new QStackedLayout(stackedWidget);
    stackedLayout->setSizeConstraint(QLayout::SetNoConstraint);
    stackedLayout->setStackingMode(QStackedLayout::StackAll);
    stackedLayout->addWidget(controlWidget);
    stackedLayout->addWidget(morphoWidget);

    stackedWidget->setLayout(stackedLayout);

    setCentralWidget(stackedWidget);

    stackedLayout->setCurrentWidget(controlWidget);

    midiWidget = new MidiWidget();

    connect(&midiControl, &MidiControl::inputPortsChanged, midiWidget, &MidiWidget::populatePortsTable);
    connect(&midiControl, &MidiControl::inputPortOpen, controlWidget, &ControlWidget::setupMidi);
    connect(&midiControl, &MidiControl::ccInputMessageReceived, controlWidget, &ControlWidget::updateMidiLinks);
    connect(midiWidget, &MidiWidget::portSelected, &midiControl, &MidiControl::openPort);

    midiControl.setInputPorts();

    connect(iterationTimer, &TimerThread::timeout, this, &MainWindow::beat);
    connect(iterationTimer, &TimerThread::timeout, this, &MainWindow::computeIterationFPS);

    connect(updateTimer, &TimerThread::timeout, morphoWidget, QOverload<>::of(&MorphoWidget::update));
    connect(updateTimer, &TimerThread::timeout, this, &MainWindow::computeUpdateFPS);

    connect(this, &MainWindow::iterationPerformed, controlWidget, &ControlWidget::updateIterationNumberLabel);
    connect(this, &MainWindow::iterationTimeMeasured, controlWidget, &ControlWidget::updateIterationMetricsLabels);
    connect(this, &MainWindow::updateTimeMeasured, controlWidget, &ControlWidget::updateUpdateMetricsLabels);

    connect(morphoWidget, &MorphoWidget::openGLInitialized, this, [&](){
        renderManager->init(morphoWidget->context());
        nodeManager->init(morphoWidget->context());
        plotsWidget->init(morphoWidget->context());

        numIterations = 0;
        numUpdates = 0;

        iterationStart = std::chrono::steady_clock::now();
        updateStart = std::chrono::steady_clock::now();

        iterationTimer->start();
        updateTimer->start();
    });
    connect(morphoWidget, &MorphoWidget::supportedTexFormats, controlWidget, &ControlWidget::populateTexFormatComboBox);
    connect(morphoWidget, &MorphoWidget::sizeChanged, renderManager, &RenderManager::resize);
    connect(morphoWidget, &MorphoWidget::sizeChanged, controlWidget, &ControlWidget::updateWindowSizeLineEdits);
    connect(morphoWidget, &MorphoWidget::sizeChanged, plotsWidget, &PlotsWidget::setImageSize);

    connect(morphoWidget, &MorphoWidget::scaleTransformChanged, plotsWidget, &PlotsWidget::transformSources);
    connect(morphoWidget, &MorphoWidget::selectedPointChanged, plotsWidget, &PlotsWidget::setSelectedPoint);
    connect(plotsWidget, &PlotsWidget::selectedPointChanged, morphoWidget, &MorphoWidget::setCursor);
    connect(plotsWidget, &PlotsWidget::drawCursor, morphoWidget, &MorphoWidget::setDrawingCursor);

    connect(controlWidget, &ControlWidget::iterateStateChanged, this, &MainWindow::setIterationState);
    connect(controlWidget, &ControlWidget::updateStateChanged, morphoWidget, &MorphoWidget::setUpdate);

    connect(renderManager, &RenderManager::texturesChanged, nodeManager, &NodeManager::onTexturesChanged);

    connect(nodeManager, &NodeManager::outputTextureChanged, renderManager, &RenderManager::setOutputTextureId);
    connect(nodeManager, &NodeManager::outputTextureChanged, morphoWidget, &MorphoWidget::setOutputTextureId);
    connect(nodeManager, &NodeManager::outputTextureChanged, plotsWidget, &PlotsWidget::setTextureID);
    connect(nodeManager, &NodeManager::outputFBOChanged, plotsWidget, &PlotsWidget::setFBO);
    connect(nodeManager, &NodeManager::sortedOperationsChanged, renderManager, &RenderManager::setSortedOperations);

    connect(controlWidget, &ControlWidget::iterationFPSChanged, this, &MainWindow::setIterationTimerInterval);
    connect(controlWidget, &ControlWidget::updateFPSChanged, this, &MainWindow::setUpdateTimerInterval);
    connect(controlWidget, &ControlWidget::startRecording, this, &MainWindow::startRecording);
    connect(controlWidget, &ControlWidget::stopRecording, this, &MainWindow::stopRecording);
    connect(controlWidget, &ControlWidget::takeScreenshot, this, &MainWindow::takeScreenshot);
    connect(controlWidget, &ControlWidget::imageSizeChanged, this, &MainWindow::setSize);
    connect(controlWidget, &ControlWidget::showMidiWidget, this, &MainWindow::showMidiWidget);
    connect(controlWidget, &ControlWidget::overlayToggled, overlay, &Overlay::enable);
    connect(controlWidget, &ControlWidget::parameterValueChanged, overlay, &Overlay::addMessage);

    setWindowTitle("Fosforo");
    setWindowIcon(QIcon(":/icons/morphogengl.png"));
    resize(renderManager->texWidth(), renderManager->texHeight());
}



MainWindow::~MainWindow()
{
    if (recorder)
        delete recorder;

    delete plotsWidget;
    delete controlWidget;
    delete nodeManager;
    delete renderManager;
    delete morphoWidget;
    delete overlay;

    delete midiWidget;

    delete iterationTimer;
    delete updateTimer;
}



void MainWindow::beat()
{
    if (recorder)
    {
        if (recorder->isRecording())
        {
            iterate();
            recorder->sendVideoFrame(renderManager->outputImage());
        }
    }
    else
    {
        iterate();
    }
}



void MainWindow::iterate()
{
    if (nodeManager->isActive())
    {
        renderManager->iterate();
        plotsWidget->updatePlots();
    }
}



void MainWindow::computeIterationFPS()
{
    iterationEnd = std::chrono::steady_clock::now();
    iterationTime = std::chrono::duration_cast<std::chrono::microseconds>(iterationEnd - iterationStart);

    numIterations++;

    if (iterationTime.count() >= 1'000'000)
    {
        double uspf = static_cast<double>(iterationTime.count()) / numIterations;
        double fps = numIterations * 1'000'000.0 / iterationTime.count();

        emit iterationTimeMeasured(uspf, fps);
        emit iterationPerformed();

        numIterations = 0;
        iterationStart = std::chrono::steady_clock::now();
    }
}



void MainWindow::computeUpdateFPS()
{
    updateEnd = std::chrono::steady_clock::now();
    updateTime = std::chrono::duration_cast<std::chrono::microseconds>(updateEnd - updateStart);

    numUpdates++;

    if (updateTime.count() >= 1'000'000)
    {
        double uspf = static_cast<double>(updateTime.count()) / numUpdates;
        double fps = numUpdates * 1'000'000.0 / updateTime.count();

        emit updateTimeMeasured(uspf, fps);

        numUpdates = 0;
        updateStart = std::chrono::steady_clock::now();
    }
}



void MainWindow::setIterationState(bool state)
{
    nodeManager->setState(state);
}



void MainWindow::setIterationTimerInterval(double newFPS)
{
    iterationFPS = newFPS;

    numIterations = 0;
    iterationStart = std::chrono::steady_clock::now();

    iterationTimer->setTimerInterval(newFPS);
}



void MainWindow::setUpdateTimerInterval(double newFPS)
{
    updateFPS = newFPS;

    numUpdates = 0;
    updateStart = std::chrono::steady_clock::now();

    updateTimer->setTimerInterval(newFPS);
}



void MainWindow::startRecording(QString recordFilename, int framesPerSecond, QMediaFormat format)
{
    recorder = new Recorder(recordFilename, framesPerSecond, format);
    connect(recorder, &Recorder::frameRecorded, controlWidget, &ControlWidget::setVideoCaptureElapsedTimeLabel);
    recorder->startRecording();
}



void MainWindow::stopRecording()
{
    recorder->stopRecording();
    disconnect(recorder, &Recorder::frameRecorded, controlWidget, &ControlWidget::setVideoCaptureElapsedTimeLabel);
    delete recorder;
    recorder = nullptr;
}



int MainWindow::getFrameCount()
{
    return recorder ? recorder->frameNumber : 0;
}



void MainWindow::takeScreenshot(QString filename)
{
    QImage screenshot = morphoWidget->grabFramebuffer();
    screenshot.save(filename);
}



void MainWindow::showMidiWidget()
{
    midiWidget->setVisible(!midiWidget->isVisible());
}



void MainWindow::setSize(int width, int height)
{
    if (stackedLayout->currentWidget() == controlWidget)
        morphoWidget->resize(width, height);
    else
        controlWidget->resize(width, height);

    updateGeometry();
    stackedWidget->updateGeometry();
    QApplication::processEvents();

    resize(width, height);

    updateGeometry();
    stackedWidget->updateGeometry();
    QApplication::processEvents();
}



void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    if (stackedLayout->currentWidget() == controlWidget)
        morphoWidget->resize(event->size());
    else
        controlWidget->resize(event->size());
}



void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->modifiers() == Qt::ControlModifier)
    {
        if (event->key() == Qt::Key_Tab)
        {
            if (stackedLayout->currentWidget() == controlWidget)
            {
                stackedLayout->setCurrentWidget(morphoWidget);
                morphoWidget->update();
            }
            else
                stackedLayout->setCurrentWidget(controlWidget);
        }
        else if (event->key() == Qt::Key_PageUp)
        {
            opacity += 0.1;
            if (opacity > 1.0)
                opacity = 1.0;
            controlWidgetOpacityEffect->setOpacity(opacity);
            plotsWidgetOpacityEffect->setOpacity(opacity);
            morphoWidget->update();
        }
        else if (event->key() == Qt::Key_PageDown)
        {
            opacity -= 0.1;
            if (opacity < 0.0)
                opacity = 0.0;
            controlWidgetOpacityEffect->setOpacity(opacity);
            plotsWidgetOpacityEffect->setOpacity(opacity);
            morphoWidget->update();
        }
    }
    else if (event->key() == Qt::Key_Space)
    {
        controlWidget->reset();
    }

    event->accept();
}



void MainWindow::closeEvent(QCloseEvent* event)
{
    iterationTimer->stop();
    updateTimer->stop();

    nodeManager->setState(false);

    midiWidget->close();

    controlWidget->close();

    event->accept();
}
