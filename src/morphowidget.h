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

#include <cmath>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QIcon>
#include <QGuiApplication>
#include <QScreen>
#include <QCloseEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QStyle>

// MorphoWidget: OpenGL widget displaying MorphogenGL's output

class MorphoWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    //MorphoWidget(int width, int height, QWidget* parent = nullptr);
    MorphoWidget(QWidget* parent = nullptr);
    virtual ~MorphoWidget() override;

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

signals:
    void openGLInitialized();
    void screenSizeChanged(int width, int height);
    void selectedPointChanged(QPoint point);
    //void closing();

public slots:
    void updateOutputTextureID(GLuint id);
    void resetZoom(int width, int height);

protected:
    //void closeEvent(QCloseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    GLuint outputTextureID = 0;

    GLuint fbo = 0;

    QRect image;
    QRect frame;
    QTransform frameTransform;
    QPointF prevPos;

    QPointF selectedPoint;
    QTransform selectedPointTransform;
    QPointF cursor;
    bool drawingCursor = false;

    QRect prevFrame;

    QOpenGLShaderProgram* program = nullptr;
    QOpenGLVertexArrayObject* vao = nullptr;
    QOpenGLBuffer* vbo = nullptr;

    void setSelectedPoint(QPointF pos);
    void updateCursor();
};
