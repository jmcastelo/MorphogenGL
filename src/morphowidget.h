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


#include "fbo.h"
#include <cmath>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QSurfaceFormat>
#include <QGuiApplication>
#include <QScreen>
#include <QCloseEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QStyle>

class Heart;

// MorphoWidget: OpenGL widget displaying MorphogenGL's output

class MorphoWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    MorphoWidget(Heart* theHeart, QWidget* parent = nullptr);
    virtual ~MorphoWidget() override;

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

signals:
    void initGenerator();
    void initHeartBeat();
    void stopHeartBeat();
    void screenSizeChanged(int width, int height);
    void closing();

public slots:
    void resetZoom();

protected:
    void closeEvent(QCloseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    Heart* heart;

    GLuint fbo = 0;

    GLint srcX0 = 0;
    GLint srcY0 = 0;
    GLint srcX1 = FBO::width;
    GLint srcY1 = FBO::height;
    int xPrev = -1;
    int yPrev = -1;
    float deltaX = 0.0f;
    float deltaY = 0.0f;
};
