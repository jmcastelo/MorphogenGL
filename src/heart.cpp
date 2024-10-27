/*
*  Copyright 2021 Jose Maria Castelo Ares
*
*  Contact: <jose.maria.castelo@gmail.com>
*  Repository: <https://github.com/jmcastelo/MorphogenGL>
*
*  This file is part of MorphogenGL.
*
*  MorphogenGL is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  MorphogenGL is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with MorphogenGL.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "heart.h"

Heart::Heart()
{
    // Heart beat!

    timer = new QChronoTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    timer->setInterval(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>{1.0 / fps}));

    // MorphoWidget

    //morphoWidget = new MorphoWidget(FBO::width, FBO::height);

    // ControlWidget

    //controlWidget = new ControlWidget(this);

    // Arrange both widgets side by side
    
    //morphoWidget->setGeometry(morphoWidget->geometry().x() - controlWidget->width() / 2, morphoWidget->geometry().y(), morphoWidget->width(), morphoWidget->height());
    //controlWidget->setGeometry(morphoWidget->geometry().x() + morphoWidget->width(), morphoWidget->geometry().y(), controlWidget->width(), morphoWidget->height());

    // Plots widget

    //plotsWidget = new PlotsWidget(FBO::width, FBO::height);

    // Main widget

    mainWidget = new MainWidget(this);

    // Signals + Slots

    connect(timer, &QChronoTimer::timeout, mainWidget, &MainWidget::updateMorphoWidget);
    //connect(timer, &QTimer::timeout, morphoWidget, QOverload<>::of(&MorphoWidget::update));
    connect(timer, &QChronoTimer::timeout, this, &Heart::beat);

    /*connect(morphoWidget, &MorphoWidget::openGLInitialized, [=]()
    {
        controlWidget->generator->init(morphoWidget->context());
        plotsWidget->init(morphoWidget->context());
        timer->start();
    });
    connect(morphoWidget, &MorphoWidget::screenSizeChanged, controlWidget, &ControlWidget::updateWindowSizeLineEdits);
    connect(morphoWidget, &MorphoWidget::selectedPointChanged, plotsWidget, &PlotsWidget::setSelectedPoint);
    connect(morphoWidget, &MorphoWidget::closing, [=]()
    {
        timer->stop();
        controlWidget->close();
        plotsWidget->close();
    });*/

    /*connect(controlWidget->generator, &GeneratorGL::outputTextureChanged, plotsWidget, &PlotsWidget::setTextureID);
    connect(controlWidget->generator, &GeneratorGL::outputTextureChanged, morphoWidget, &MorphoWidget::updateOutputTextureID);
    connect(controlWidget->generator, &GeneratorGL::outputTextureChanged, [=](GLuint id){ if (encoder) encoder->setTextureID(id); });
    connect(controlWidget->generator, &GeneratorGL::imageSizeChanged, morphoWidget, &MorphoWidget::resetZoom);
    connect(controlWidget->generator, &GeneratorGL::imageSizeChanged, plotsWidget, &PlotsWidget::setImageSize);
    connect(controlWidget, &ControlWidget::plotsActionTriggered, this, &Heart::togglePlotsWidget);
    connect(controlWidget, &ControlWidget::closing, [=]()
    {
        morphoWidget->close();
        plotsWidget->close();
    });*/

    connect(mainWidget, &MainWidget::morphoWidgetInitialized, this, [&](){
        timer->start();
    });

    connect(mainWidget, &MainWidget::closing, this, [&](){
        timer->stop();
    });

    /*connect(mainWidget, &MainWidget::outputTextureChanged, this, [=](GLuint id){
        if (encoder)
            encoder->setTextureID(id);
    });*/

    // Show widgets

    //morphoWidget->show();
    //controlWidget->show();

    mainWidget->resize(1024, 1024);
    mainWidget->show();
}

Heart::~Heart()
{
    /*delete plotsWidget;
    delete morphoWidget;
    delete controlWidget;*/
    delete mainWidget;
    if (recorder) delete recorder;
}

void Heart::beat()
{
    // Record frame

    if (recorder)
    {
        if(recorder->isRecording())
        {
            iterate();
            recorder->sendVideoFrame(mainWidget->generator()->outputImage());
        }
    }
    else
    {
        iterate();
    }
}

void Heart::iterate()
{
    // Perform one iteration

    mainWidget->generator()->iterate();

    // Compute iteration time

    if (mainWidget->generator()->active && mainWidget->generator()->getIterationNumber() % numIterations == 0)
    {
        end = std::chrono::steady_clock::now();
        auto iterationTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        start = std::chrono::steady_clock::now();

        emit iterationTimeMeasured(iterationTime, numIterations);
    }

    if (mainWidget->generator()->active)
        emit iterationPerformed();

    // Compute plots

    /*if (controlWidget->generator->active && plotsWidget->plotsActive())
            plotsWidget->getPixels();*/

    mainWidget->computePlots();
}

void Heart::setStartTime()
{
    start = std::chrono::steady_clock::now();
}

std::chrono::nanoseconds Heart::getTimerInterval()
{
    return timer->interval();
}

void Heart::setTimerInterval(std::chrono::nanoseconds interval)
{
    timer->setInterval(interval);
}

int Heart::getMorphoWidgetWidth()
{
    return mainWidget->morphoWidgetWidth();
}

int Heart::getMorphoWidgetHeight()
{
    return mainWidget->morphoWidgetHeight();
}

void Heart::resizeMorphoWidget(int width, int height)
{
    mainWidget->resize(width, height);
}

QImage Heart::grabMorphoWidgetFramebuffer()
{
    //return morphoWidget->grabFramebuffer();
    return mainWidget->grabMorphoWidgetFramebuffer();
}

/*void Heart::startRecording(QString recordFilename, int framesPerSecond, QString preset, int crf)
{
    encoder = new FFmpegEncoder(
        recordFilename.toStdString().c_str(),
        mainWidget->width(),
        mainWidget->height(),
        framesPerSecond,
        preset.toStdString().c_str(),
        QString::number(crf).toStdString().c_str(),
        mainWidget->morphoWidgetContext(),
        **mainWidget->generator()->getOutputTextureID());
}*/

void Heart::startRecording(QString recordFilename, int framesPerSecond, QMediaFormat format)
{
    //disconnect(timer, &QChronoTimer::timeout, this, &Heart::beat);
    recorder = new Recorder(recordFilename, framesPerSecond, format);
    connect(recorder, &Recorder::frameRecorded, this, &Heart::frameRecorded);
    recorder->startRecording();
}

void Heart::stopRecording()
{
    recorder->stopRecording();
    disconnect(recorder, &Recorder::frameRecorded, this, &Heart::frameRecorded);
    delete recorder;
    recorder = nullptr;
    //connect(timer, &QChronoTimer::timeout, this, &Heart::beat);
}

int Heart::getFrameCount()
{
    return recorder ? recorder->frameNumber : 0;
}
