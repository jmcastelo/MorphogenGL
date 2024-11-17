#include "mainwindow.h"



MainWindow::MainWindow()
{
    iterationTimer = new TimerThread(iterationFPS, this);
    updateTimer = new TimerThread(updateFPS, this);

    generator = new GeneratorGL();

    morphoWidget = new MorphoWidget(FBO::width, FBO::height);
    morphoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    morphoWidget->setMinimumSize(0, 0);

    plotsWidget = new PlotsWidget(FBO::width, FBO::height);
    plotsWidget->setVisible(false);

    controlWidget = new ControlWidget(iterationFPS, updateFPS, generator, plotsWidget);
    controlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    controlWidget->setMinimumSize(0, 0);

    opacityEffect = new QGraphicsOpacityEffect(controlWidget);
    opacityEffect->setOpacity(controlWidgetOpacity);
    controlWidget->setGraphicsEffect(opacityEffect);

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

    connect(iterationTimer, &TimerThread::timeout, this, &MainWindow::beat);
    connect(iterationTimer, &TimerThread::timeout, this, &MainWindow::computeIterationFPS);

    connect(updateTimer, &TimerThread::timeout, morphoWidget, QOverload<>::of(&MorphoWidget::update));
    connect(updateTimer, &TimerThread::timeout, this, &MainWindow::computeUpdateFPS);

    connect(this, &MainWindow::iterationPerformed, controlWidget, &ControlWidget::updateIterationNumberLabel);
    connect(this, &MainWindow::iterationPerformed, plotsWidget, &PlotsWidget::updatePlots);
    connect(this, &MainWindow::iterationTimeMeasured, controlWidget, &ControlWidget::updateIterationMetricsLabels);
    connect(this, &MainWindow::updateTimeMeasured, controlWidget, &ControlWidget::updateUpdateMetricsLabels);
    //connect(this, &MainWindow::closing, controlWidget, &ControlWidget::close);

    connect(morphoWidget, &MorphoWidget::openGLInitialized, this, [=]()
    {
        controlWidget->generator->init(morphoWidget->context());
        plotsWidget->init(morphoWidget->context());

        numIterations = 0;
        numUpdates = 0;

        iterationStart = std::chrono::steady_clock::now();
        updateStart = std::chrono::steady_clock::now();

        iterationTimer->start();
        updateTimer->start();
    });
    connect(morphoWidget, &MorphoWidget::supportedTexFormats, controlWidget, &ControlWidget::populateTexFormatComboBox);
    connect(morphoWidget, &MorphoWidget::sizeChanged, generator, &GeneratorGL::resize);
    connect(morphoWidget, &MorphoWidget::sizeChanged, controlWidget, &ControlWidget::updateWindowSizeLineEdits);
    connect(morphoWidget, &MorphoWidget::sizeChanged, plotsWidget, &PlotsWidget::setImageSize);

    connect(morphoWidget, &MorphoWidget::scaleTransformChanged, plotsWidget, &PlotsWidget::transformSources);
    connect(morphoWidget, &MorphoWidget::selectedPointChanged, plotsWidget, &PlotsWidget::setSelectedPoint);
    connect(plotsWidget, &PlotsWidget::selectedPointChanged, morphoWidget, &MorphoWidget::setCursor);
    connect(plotsWidget, &PlotsWidget::drawCursor, morphoWidget, &MorphoWidget::setDrawingCursor);

    connect(controlWidget, &ControlWidget::iterateStateChanged, this, &MainWindow::setIterationState);
    connect(controlWidget, &ControlWidget::updateStateChanged, morphoWidget, &MorphoWidget::setUpdate);

    connect(generator, &GeneratorGL::outputTextureChanged, morphoWidget, &MorphoWidget::updateOutputTextureID);
    connect(generator, &GeneratorGL::outputTextureChanged, plotsWidget, &PlotsWidget::setTextureID);

    connect(controlWidget, &ControlWidget::iterationFPSChanged, this, &MainWindow::setIterationTimerInterval);
    connect(controlWidget, &ControlWidget::updateFPSChanged, this, &MainWindow::setUpdateTimerInterval);
    connect(controlWidget, &ControlWidget::startRecording, this, &MainWindow::startRecording);
    connect(controlWidget, &ControlWidget::stopRecording, this, &MainWindow::stopRecording);
    connect(controlWidget, &ControlWidget::takeScreenshot, this, &MainWindow::takeScreenshot);
    connect(controlWidget, &ControlWidget::imageSizeChanged, this, &MainWindow::setSize);

    setWindowTitle("Morphogen");
    setWindowIcon(QIcon(":/icons/morphogengl.png"));
    resize(FBO::width, FBO::height);
}



MainWindow::~MainWindow()
{
    if (recorder)
        delete recorder;

    delete plotsWidget;
    delete controlWidget;
    delete generator;
    delete morphoWidget;

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
            recorder->sendVideoFrame(generator->outputImage());
        }
    }
    else
    {
        iterate();
    }
}



void MainWindow::iterate()
{
    if (generator->isActive())
    {
        generator->iterate();

        if (plotsWidget->isEnabled())
            plotsWidget->setPixelRGB(generator->pixelRGB(plotsWidget->pixelSources()));

        emit iterationPerformed();
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
    generator->setState(state);
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
            }
            else
            {
                stackedLayout->setCurrentWidget(controlWidget);
            }
        }
        else if (event->key() == Qt::Key_PageUp)
        {
            controlWidgetOpacity += 0.1;
            if (controlWidgetOpacity > 1.0)
                controlWidgetOpacity = 1.0;
            opacityEffect->setOpacity(controlWidgetOpacity);
            morphoWidget->update();
        }
        else if (event->key() == Qt::Key_PageDown)
        {
            controlWidgetOpacity -= 0.1;
            if (controlWidgetOpacity < 0.0)
                controlWidgetOpacity = 0.0;
            opacityEffect->setOpacity(controlWidgetOpacity);
            morphoWidget->update();
        }
    }

    event->accept();
}



void MainWindow::closeEvent(QCloseEvent* event)
{
    iterationTimer->stop();
    updateTimer->stop();

    generator->setState(false);

    event->accept();
}