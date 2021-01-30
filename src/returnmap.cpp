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

#include "returnmap.h"

ReturnMap::ReturnMap(int width, int height, QWidget* parent) :
    QWidget(parent),
    imageWidth { width },
    imageHeight { height }
{
    // Point

    point = QPoint(width / 2, height / 2);

    // Red plot

    redPlot = new QCustomPlot;

    redPlot->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    redPlot->xAxis->setLabel("Red intensity (N)");
    redPlot->yAxis->setLabel("Red intensity (N + 1)");

    redPlot->xAxis->setRange(0.0, 1.0);
    redPlot->yAxis->setRange(0.0, 1.0);

    redPlot->axisRect()->setupFullAxesBox(true);

    // Green plot

    greenPlot = new QCustomPlot;

    greenPlot->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    greenPlot->xAxis->setLabel("Green intensity (N)");
    greenPlot->yAxis->setLabel("Green intensity (N + 1)");

    greenPlot->xAxis->setRange(0.0, 1.0);
    greenPlot->yAxis->setRange(0.0, 1.0);

    greenPlot->axisRect()->setupFullAxesBox(true);

    // Blue plot

    bluePlot = new QCustomPlot;

    bluePlot->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    bluePlot->xAxis->setLabel("Blue intensity (N)");
    bluePlot->yAxis->setLabel("Blue intensity (N + 1)");

    bluePlot->xAxis->setRange(0.0, 1.0);
    bluePlot->yAxis->setRange(0.0, 1.0);

    bluePlot->axisRect()->setupFullAxesBox(true);

    // Gray plot

    grayPlot = new QCustomPlot;

    grayPlot->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    grayPlot->xAxis->setLabel("Gray intensity (N)");
    grayPlot->yAxis->setLabel("Gray intensity (N + 1)");

    grayPlot->xAxis->setRange(0.0, 1.0);
    grayPlot->yAxis->setRange(0.0, 1.0);

    grayPlot->axisRect()->setupFullAxesBox(true);

    // Red channel

    redPoints = new QCPCurve(redPlot->xAxis, redPlot->yAxis);
    redPoints->setPen(QColor(255, 0, 0, 255));
    redPoints->setLineStyle(QCPCurve::lsNone);
    redPoints->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));

    // Green channel

    greenPoints = new QCPCurve(greenPlot->xAxis, greenPlot->yAxis);
    greenPoints->setPen(QColor(0, 255, 0, 255));
    greenPoints->setLineStyle(QCPCurve::lsNone);
    greenPoints->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));

    // Blue channel

    bluePoints = new QCPCurve(bluePlot->xAxis, bluePlot->yAxis);
    bluePoints->setPen(QColor(0, 0, 255, 255));
    bluePoints->setLineStyle(QCPCurve::lsNone);
    bluePoints->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));

    // Grayscale

    grayPoints = new QCPCurve(grayPlot->xAxis, grayPlot->yAxis);
    grayPoints->setPen(QColor(128, 128, 128, 255));
    grayPoints->setLineStyle(QCPCurve::lsNone);
    grayPoints->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));

    // Controls

    QCheckBox* statusCheckBox = new QCheckBox("Active");
    statusCheckBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    statusCheckBox->setChecked(false);
    connect(statusCheckBox, &QCheckBox::stateChanged, this, &ReturnMap::setStatus);

    QPushButton* resetPushButton = new QPushButton("Reset");
    resetPushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(resetPushButton, &QPushButton::pressed, this, &ReturnMap::reset);

    FocusLineEdit* numPointsLineEdit = new FocusLineEdit;
    numPointsLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QIntValidator* validator = new QIntValidator(1, 1000000, numPointsLineEdit);
    numPointsLineEdit->setValidator(validator);
    numPointsLineEdit->setText(QString::number(numPoints));

    connect(numPointsLineEdit, &FocusLineEdit::returnPressed, [=](){ numPoints = numPointsLineEdit->text().toInt(); });
    connect(numPointsLineEdit, &FocusLineEdit::focusOut, [=](){ numPointsLineEdit->setText(QString::number(numPoints)); });

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setAlignment(Qt::AlignLeft);
    formLayout->addRow("Points:", numPointsLineEdit);

    // Layouts

    QGridLayout* plotsLayout = new QGridLayout;
    plotsLayout->addWidget(redPlot, 0, 0);
    plotsLayout->addWidget(greenPlot, 0, 1);
    plotsLayout->addWidget(bluePlot, 1, 0);
    plotsLayout->addWidget(grayPlot, 1, 1);

    QHBoxLayout* controlsLayout = new QHBoxLayout;
    controlsLayout->setAlignment(Qt::AlignLeft);
    controlsLayout->addWidget(statusCheckBox);
    controlsLayout->addWidget(resetPushButton);
    controlsLayout->addLayout(formLayout);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(controlsLayout);
    layout->addLayout(plotsLayout);

    setLayout(layout);
}

ReturnMap::~ReturnMap()
{
    delete redPlot;
    delete greenPlot;
    delete bluePlot;
    delete grayPlot;
}

void ReturnMap::setPixels(GLfloat *pixels)
{
    // Three channels

    int row = point.y() * imageWidth * 3;
    int col = point.x() * 3;

    double red = pixels[row + col];
    double green = pixels[row + col + 1];
    double blue = pixels[row + col + 2];
    double gray = (red + green + blue) / 3.0;

    if (iteration > 0)
    {
        redPoints->addData(iteration, prevRed, red);
        greenPoints->addData(iteration, prevGreen, green);
        bluePoints->addData(iteration, prevBlue, blue);
        grayPoints->addData(iteration, prevGray, gray);

        if (iteration > numPoints)
        {
            redPoints->data()->removeBefore(iteration - numPoints);
            greenPoints->data()->removeBefore(iteration - numPoints);
            bluePoints->data()->removeBefore(iteration - numPoints);
            grayPoints->data()->removeBefore(iteration - numPoints);
        }

        redPlot->replot(QCustomPlot::rpQueuedReplot);
        greenPlot->replot(QCustomPlot::rpQueuedReplot);
        bluePlot->replot(QCustomPlot::rpQueuedReplot);
        grayPlot->replot(QCustomPlot::rpQueuedReplot);
    }

    prevRed = red;
    prevGreen = green;
    prevBlue = blue;
    prevGray = gray;

    iteration++;
}

void ReturnMap::reset()
{
    iteration = 0;

    redPoints->data()->clear();
    greenPoints->data()->clear();
    bluePoints->data()->clear();
    grayPoints->data()->clear();

    redPlot->replot();
    greenPlot->replot();
    bluePlot->replot();
    grayPlot->replot();
}

void ReturnMap::checkPoint()
{
    if (point.x() < 0)
        point.setX(0);
    if (point.y() < 0)
        point.setY(0);
    if (point.x() >= imageWidth)
        point.setX(imageWidth - 1);
    if (point.y() >= imageHeight)
        point.setY(imageHeight - 1);
}

void ReturnMap::setPoint(QPoint thePoint)
{
    point = thePoint;

    checkPoint();

    reset();
}

void ReturnMap::setImageSize(int width, int height)
{
    imageWidth = width;
    imageHeight = height;

    checkPoint();

    reset();
}
