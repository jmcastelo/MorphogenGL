/*
*  Copyright 2020 José María Castelo Ares
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

#include "fbo.h"
#include "controlwidget.h"
#include "generator.h"
#include <chrono>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QSurfaceFormat>
#include <QCloseEvent>
#include <QTimer>

class ControlWidget;

// MorphoWidget: OpenGL widget displaying MorphogenGL's output

class MorphoWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    QTimer* timer;

    MorphoWidget();
    virtual ~MorphoWidget() override;

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    void setStartTime();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    GLuint fbo = 0;
    ControlWidget* controlWidget = nullptr;
    GeneratorGL* generator = nullptr;
    std::chrono::time_point<std::chrono::steady_clock> start;
    std::chrono::time_point<std::chrono::steady_clock> end;

signals:
    void screenSizeChanged(int width, int height);
    void iterationPerformed();
    void iterationTimeMeasured(long int iterationTime);
    void frameRecorded();
};
