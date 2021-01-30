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

#include "focuslineedit.h"
#include <qcustomplot.h>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QIntValidator>
#include <QPoint>

class ReturnMap : public QWidget
{
    Q_OBJECT

public:
    ReturnMap(int width, int height, QWidget* parent = nullptr);
    ~ReturnMap();

    void setPixels(GLfloat* pixels);
    void setImageSize(int width, int height);

    bool active() { return status; }

public slots:
    void setPoint(QPoint thePoint);

private:
    QCustomPlot* redPlot;
    QCustomPlot* greenPlot;
    QCustomPlot* bluePlot;
    QCustomPlot* grayPlot;

    QCPCurve* redPoints;
    QCPCurve* greenPoints;
    QCPCurve* bluePoints;
    QCPCurve* grayPoints;

    int imageWidth;
    int imageHeight;

    QPoint point;

    int iteration = 0;
    int numPoints = 50;

    double prevRed;
    double prevGreen;
    double prevBlue;
    double prevGray;

    bool status = false;

    void checkPoint();

private slots:
    void setStatus(int state) { status = (state == Qt::Checked); }
    void reset();
};
