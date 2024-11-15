#include "mainwindow.h"



MainWindow::MainWindow()
{
    timer = new QChronoTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    timer->setSingleShot(true);
    //timer->setInterval(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>{1.0 / fps}));
    setTimerInterval(fps);

    generator = new GeneratorGL();

    morphoWidget = new MorphoWidget(FBO::width, FBO::height);
    morphoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    morphoWidget->setMinimumSize(0, 0);

    plotsWidget = new PlotsWidget(FBO::width, FBO::height);
    plotsWidget->setVisible(false);

    controlWidget = new ControlWidget(fps, generator, plotsWidget);
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

    connect(timer, &QChronoTimer::timeout, this, &MainWindow::beat);
    //connect(morphoWidget, &QOpenGLWidget::frameSwapped, timer, &QChronoTimer::start);
    //connect(morphoWidget, &QOpenGLWidget::frameSwapped, this, &MainWindow::beat);

    connect(this, &MainWindow::iterationPerformed, controlWidget, &ControlWidget::updateIterationNumberLabel);
    connect(this, &MainWindow::iterationPerformed, plotsWidget, &PlotsWidget::updatePlots);
    connect(this, &MainWindow::iterationTimeMeasured, controlWidget, &ControlWidget::updateMetricsLabels);
    connect(this, &MainWindow::closing, controlWidget, &ControlWidget::close);

    connect(morphoWidget, &MorphoWidget::openGLInitialized, this, [=]()
    {
        controlWidget->generator->init(morphoWidget->context());
        plotsWidget->init(morphoWidget->context());

        start = std::chrono::steady_clock::now();
        timer->start();
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
    //connect(controlWidget, &ControlWidget::closing, this, [&](){ close(); });

    connect(generator, &GeneratorGL::outputTextureChanged, morphoWidget, &MorphoWidget::updateOutputTextureID);
    connect(generator, &GeneratorGL::outputTextureChanged, plotsWidget, &PlotsWidget::setTextureID);

    //connect(controlWidget, &ControlWidget::timerIntervalChanged, this, &MainWindow::setTimerInterval);
    connect(controlWidget, &ControlWidget::fpsChanged, this, &MainWindow::setTimerInterval);
    connect(controlWidget, &ControlWidget::startRecording, this, &MainWindow::startRecording);
    connect(controlWidget, &ControlWidget::stopRecording, this, &MainWindow::stopRecording);
    connect(controlWidget, &ControlWidget::takeScreenshot, this, &MainWindow::takeScreenshot);
    connect(controlWidget, &ControlWidget::imageSizeChanged, this, &MainWindow::setSize);
    //connect(controlWidget, &ControlWidget::imageSizeChanged, morphoWidget, &MorphoWidget::resetZoom);

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

    delete timer;
}



void MainWindow::beat()
{
    //if (numBeats == -1)
        //start = std::chrono::steady_clock::now();

    if (recorder)
    {
        if(recorder->isRecording())
        {
            iterate();
            recorder->sendVideoFrame(generator->outputImage());
        }
    }
    else
    {
        iterate();
    }

    morphoWidget->update();

    numBeats++;

    // Compute beat time

    if (numBeats >= numBeatsTrigger)
    {
        end = std::chrono::steady_clock::now();

        beatTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        emit iterationTimeMeasured(beatTime.count() / numBeats, numBeats * 1000000.0 / beatTime.count());

        numBeats = 0;

        start = std::chrono::steady_clock::now();
    }

    timer->start();
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



void MainWindow::setIterationState(bool state)
{
    generator->setState(state);
}



void MainWindow::setTimerInterval(double newFPS)
{
    fps = newFPS;
    numBeatsTrigger = static_cast<int>(newFPS);

    qDebug() << fps << numBeatsTrigger;

    std::chrono::nanoseconds nanos = std::chrono::nanoseconds{static_cast<std::chrono::nanoseconds::rep>(fps > 0 ? 1000000000.0 / fps : 0)};
    timer->setInterval(nanos);
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
    timer->stop();
    generator->setState(false);
    emit closing();
    event->accept();
}
