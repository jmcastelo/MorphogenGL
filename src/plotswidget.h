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

#include "rgbwidget.h"
#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <QOffscreenSurface>

class PlotsWidget : public QWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    PlotsWidget(int width, int height, QWidget* parent = nullptr);
    ~PlotsWidget();

    void init(QOpenGLContext* mainContext);

    bool plotsActive();
    void getPixels();

signals:
    void setSelectedPoint(QPoint point);

public slots:
    void setTextureID(GLuint id);
    void setImageSize(int width, int height);

private:
    RGBWidget* rgbWidget;

    QTabWidget* tabWidget;

    QOpenGLContext* context;
    QOffscreenSurface* surface;

    GLuint textureID = 0;

    GLfloat* pixels = nullptr;

    void allocatePixelsArray(int width, int height);
};
