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

    morphoWidget = new MorphoWidget(this);

    // ControlWidget

    controlWidget = new ControlWidget(this);

    // Arrange both widgets side by side
    
    morphoWidget->setGeometry(morphoWidget->geometry().x() - controlWidget->width() / 2, morphoWidget->geometry().y(), morphoWidget->width(), morphoWidget->height());
    controlWidget->setGeometry(morphoWidget->geometry().x() + morphoWidget->width(), morphoWidget->geometry().y(), controlWidget->width(), controlWidget->height());

    // RGBWidget

    rgbWidget = new RGBWidget(this);

    // Signals + Slots

    connect(timer, &QTimer::timeout, morphoWidget, QOverload<>::of(&MorphoWidget::update));
    connect(timer, &QTimer::timeout, this, &Heart::beat);

    connect(morphoWidget, &MorphoWidget::initGenerator, [&controlWidget = this->controlWidget, &morphoWidget = this->morphoWidget](){ controlWidget->generator->init(morphoWidget->context()); });
    connect(morphoWidget, &MorphoWidget::initHeartBeat, [&timer = this->timer]() { timer->start(); });
    connect(morphoWidget, &MorphoWidget::stopHeartBeat, [&timer = this->timer]() { timer->stop(); });
    connect(morphoWidget, &MorphoWidget::screenSizeChanged, controlWidget, &ControlWidget::updateWindowSizeLineEdits);
    connect(morphoWidget, &MorphoWidget::closing, [&controlWidget = this->controlWidget, &rgbWidget = this->rgbWidget]()
    {
        controlWidget->close();
        rgbWidget->close();
    });

    connect(controlWidget, &ControlWidget::imageSizeChanged, morphoWidget, &MorphoWidget::resetZoom);
    connect(controlWidget, &ControlWidget::imageSizeChanged, rgbWidget, &RGBWidget::allocatePixelsArray);
    connect(controlWidget, &ControlWidget::seedDrawn, rgbWidget, [&rgbWidget = this->rgbWidget]()
    {
        if (rgbWidget->isVisible())
        {
            rgbWidget->getPixels();
            rgbWidget->update();
        }
    });
    connect(controlWidget, &ControlWidget::closing, [&morphoWidget = this->morphoWidget, &rgbWidget = this->rgbWidget]()
    {
        morphoWidget->close();
        rgbWidget->close();
    });

    connect(rgbWidget, &RGBWidget::closing, controlWidget, &ControlWidget::uncheckRGBGraphAction);

    // Show widgets

    morphoWidget->show();
    controlWidget->show();
}

Heart::~Heart()
{
    delete rgbWidget;
    delete morphoWidget;
    delete controlWidget;
    if (encoder) delete encoder;
}

GLuint Heart::getGeneratorOutputTextureID()
{
    return controlWidget->generator->getOutputTextureID();
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
    
    // Update rgbWidget (making all its pixel computations) only if visible

    if (rgbWidget->isVisible())
    {
        if (controlWidget->generator->active)
            rgbWidget->getPixels();
        
        rgbWidget->update();
    }
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
        morphoWidget->context());
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

void Heart::setRGBWidgetVisibility(bool visible)
{
    if (visible)
    {
        rgbWidget->show();
        rgbWidget->getPixels();
        rgbWidget->update();
    }
    else
        rgbWidget->hide();
}
