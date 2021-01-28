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

    timer = new QTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    timer->setInterval(20);

    // MorphoWidget

    morphoWidget = new MorphoWidget(FBO::width, FBO::height);

    // ControlWidget

    controlWidget = new ControlWidget(this);

    // Arrange both widgets side by side
    
    morphoWidget->setGeometry(morphoWidget->geometry().x() - controlWidget->width() / 2, morphoWidget->geometry().y(), morphoWidget->width(), morphoWidget->height());
    controlWidget->setGeometry(morphoWidget->geometry().x() + morphoWidget->width(), morphoWidget->geometry().y(), controlWidget->width(), morphoWidget->height());

    // Plots widget

    plotsWidget = new PlotsWidget(FBO::width, FBO::height);

    // Signals + Slots

    connect(timer, &QTimer::timeout, morphoWidget, QOverload<>::of(&MorphoWidget::update));
    connect(timer, &QTimer::timeout, this, &Heart::beat);

    connect(morphoWidget, &MorphoWidget::openGLInitialized, [=]()
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
    });

    connect(controlWidget->generator, &GeneratorGL::outputTextureChanged, plotsWidget, &PlotsWidget::setTextureID);
    connect(controlWidget->generator, &GeneratorGL::outputTextureChanged, morphoWidget, &MorphoWidget::updateOutputTextureID);
    connect(controlWidget->generator, &GeneratorGL::outputTextureChanged, [=](GLuint id){ if (encoder) encoder->setTextureID(id); });
    connect(controlWidget->generator, &GeneratorGL::imageSizeChanged, morphoWidget, &MorphoWidget::resetZoom);
    connect(controlWidget->generator, &GeneratorGL::imageSizeChanged, plotsWidget, &PlotsWidget::setImageSize);
    connect(controlWidget, &ControlWidget::plotsActionTriggered, this, &Heart::togglePlotsWidget);
    connect(controlWidget, &ControlWidget::closing, [=]()
    {
        morphoWidget->close();
        plotsWidget->close();
    });

    // Show widgets

    morphoWidget->show();
    controlWidget->show();
}

Heart::~Heart()
{
    delete plotsWidget;
    delete morphoWidget;
    delete controlWidget;
    if (encoder) delete encoder;
}

void Heart::beat()
{
    // Perform one iteration

    controlWidget->generator->iterate();

    // Record frame

    if (encoder)
    {
        encoder->recordFrame();
        emit frameRecorded();
    }

    // Compute iteration time

    if (controlWidget->generator->active && controlWidget->generator->getIterationNumber() % 10 == 0)
    {
        end = std::chrono::steady_clock::now();
        auto iterationTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        start = std::chrono::steady_clock::now();

        emit iterationTimeMeasured(iterationTime);
    }

    if (controlWidget->generator->active)
        emit iterationPerformed();
    
    // Compute plots

    if (controlWidget->generator->active && plotsWidget->plotsActive())
        plotsWidget->getPixels();
}

void Heart::setStartTime()
{
    start = std::chrono::steady_clock::now();
}

int Heart::getTimerInterval()
{
    return timer->interval();
}

void Heart::setTimerInterval(int interval)
{
    timer->setInterval(interval);
}

int Heart::getMorphoWidgetWidth()
{
    return morphoWidget->width();
}

int Heart::getMorphoWidgetHeight()
{
    return morphoWidget->height();
}

void Heart::resizeMorphoWidget(int width, int height)
{
    morphoWidget->resize(width, height);
}

QImage Heart::grabMorphoWidgetFramebuffer()
{
    return morphoWidget->grabFramebuffer();
}

void Heart::startRecording(QString recordFilename, int framesPerSecond, QString preset, int crf)
{
    encoder = new FFmpegEncoder(
        recordFilename.toStdString().c_str(),
        morphoWidget->width(),
        morphoWidget->height(),
        framesPerSecond,
        preset.toStdString().c_str(),
        QString::number(crf).toStdString().c_str(),
        morphoWidget->context(),
        **controlWidget->generator->getOutputTextureID());
}

void Heart::stopRecording()
{
    delete encoder;
    encoder = nullptr;
}

int Heart::getFrameCount()
{
    return encoder ? encoder->frameNumber : 0;
}

void Heart::togglePlotsWidget()
{
    plotsWidget->setVisible(!plotsWidget->isVisible());
}
