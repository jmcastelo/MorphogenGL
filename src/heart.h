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

//#include "fbo.h"
#include "mainwidget.h"
//#include "ffmpegencoder.h"
#include "recorder.h"
#include <chrono>
#include <QObject>
#include <QChronoTimer>
#include <QImage>
#include <QString>

class Heart : public QObject
{
    Q_OBJECT

public:
    Heart();
    ~Heart();

    unsigned int numIterations = 100;
    double fps = 60.0;

    void setStartTime();

    std::chrono::nanoseconds getTimerInterval();
    void setTimerInterval(std::chrono::nanoseconds interval);

    int getMorphoWidgetWidth();
    int getMorphoWidgetHeight();
    void resizeMorphoWidget(int width, int height);

    QImage grabMorphoWidgetFramebuffer();

    void startRecording(QString recordFilename, int framesPerSecond, QMediaFormat format);
    void stopRecording();
    int getFrameCount();

signals:
    void iterationPerformed();
    void iterationTimeMeasured(std::chrono::microseconds iterationTime, int numIterations);
    void frameRecorded();
    void updateImageSize(int width, int height);
    void imageSizeChanged();
    void screenSizeChanged();

private:
    //MorphoWidget* morphoWidget;
    //ControlWidget* controlWidget;
    //PlotsWidget* plotsWidget;
    MainWidget* mainWidget;
    //FFmpegEncoder* encoder = nullptr;
    Recorder* recorder = nullptr;

    QChronoTimer* timer;

    std::chrono::time_point<std::chrono::steady_clock> start;
    std::chrono::time_point<std::chrono::steady_clock> end;

    void beat();
    void iterate();
};
