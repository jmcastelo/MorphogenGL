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

#pragma once

#include "morphowidget.h"
#include "controlwidget.h"
#include "rgbwidget.h"
#include "ffmpegencoder.h"
#include <chrono>
#include <QObject>
#include <QTimer>
#include <QImage>
#include <QString>

class Heart : public QObject
{
    Q_OBJECT

public:
    Heart();
    ~Heart();

    GLuint getOutputTextureID();

    void setStartTime();
    
    int getTimerInterval();
    void setTimerInterval(int interval);

    int getMorphoWidgetWidth();
    int getMorphoWidgetHeight();
    void resizeMorphoWidget(int width, int height);

    QImage grabMorphoWidgetFramebuffer();

    void startRecording(QString recordFilename, int framesPerSecond, QString preset, int crf);
    void stopRecording();
    int getFrameCount();

    void setRGBWidgetVisibility(bool visible);

signals:
    void iterationPerformed();
    void iterationTimeMeasured(long int iterationTime);
    void frameRecorded();
    void updateImageSize(int width, int height);
    void imageSizeChanged();
    void screenSizeChanged();

private:
    MorphoWidget* morphoWidget;
    ControlWidget* controlWidget;
    RGBWidget* rgbWidget;
    FFmpegEncoder* encoder = nullptr;

    QTimer* timer;

    std::chrono::time_point<std::chrono::steady_clock> start;
    std::chrono::time_point<std::chrono::steady_clock> end;

    void beat();
};
