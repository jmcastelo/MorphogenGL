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
#include "colorpath.h"

#include <QWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QComboBox>
#include <QLineEdit>
#include <QIntValidator>
#include <QTransform>



class PlotsWidget : public QWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    PlotsWidget(GLuint width, GLuint height, QWidget* parent = nullptr);
    ~PlotsWidget();

    void init(QOpenGLContext* mainContext);
    bool isEnabled(){ return enabled; }
    QList<QPoint> pixelSources() { return sources; }
    void setPixelRGB(QList<QVector3D> rgb);

signals:
    void selectedPointChanged(QPoint point);
    void drawCursor(bool on);

public slots:
    void updatePlots();
    void setTextureID(GLuint id);
    void setImageSize(int width, int height);
    void setSelectedPoint(QPoint point);
    void transformSources(QTransform transform);

private:
    RGBWidget* rgbWidget;

    QOpenGLContext* context;
    QOffscreenSurface* surface;

    GLuint textureID = 0;

    GLfloat* pixels = nullptr;
    GLfloat* vertices = nullptr;

    GLuint imageWidth, imageHeight;

    QPoint cursor;

    bool enabled = false;

    QList<ColorPath> colorPaths;
    QList<QPoint> sources;
    QList<GLfloat> allVertices;
    QList<GLuint> numVertices;

    int numIts = 100;

    QComboBox* selectPathComboBox;
    QLineEdit* xCoordLineEdit;
    QLineEdit* yCoordLineEdit;
    QIntValidator* xCoordValidator;
    QIntValidator* yCoordValidator;
    QLineEdit* numItsLineEdit;

    void checkPoint(QPoint &point);
    void setVertices();

private slots:
    void addColorPath();
    void removeColorPath();
    void setControls(int index);
    void setNumIts();
};
