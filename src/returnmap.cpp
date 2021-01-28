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

    // Plot

    plot = new QCustomPlot;

    plot->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    plot->xAxis->setLabel("Color intensity (N)");
    plot->yAxis->setLabel("Color intensity (N + 1)");

    plot->xAxis->setRange(0.0, 1.0);
    plot->yAxis->setRange(0.0, 1.0);

    plot->axisRect()->setupFullAxesBox(true);

    // Red channel

    redPoints = new QCPCurve(plot->xAxis, plot->yAxis);
    redPoints->setPen(QColor(255, 0, 0, 85));
    redPoints->setLineStyle(QCPCurve::lsNone);
    redPoints->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));

    // Green channel

    greenPoints = new QCPCurve(plot->xAxis, plot->yAxis);
    greenPoints->setPen(QColor(0, 255, 0, 85));
    greenPoints->setLineStyle(QCPCurve::lsNone);
    greenPoints->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));

    // Blue channel

    bluePoints = new QCPCurve(plot->xAxis, plot->yAxis);
    bluePoints->setPen(QColor(0, 0, 255, 85));
    bluePoints->setLineStyle(QCPCurve::lsNone);
    bluePoints->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));

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

    // Layout

    QHBoxLayout* controlsLayout = new QHBoxLayout;
    controlsLayout->setAlignment(Qt::AlignLeft);
    controlsLayout->addWidget(statusCheckBox);
    controlsLayout->addWidget(resetPushButton);
    controlsLayout->addLayout(formLayout);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(controlsLayout);
    layout->addWidget(plot);

    setLayout(layout);
}

ReturnMap::~ReturnMap()
{
    delete plot;
}

void ReturnMap::setPixels(GLfloat *pixels)
{
    // Three channels

    int row = point.y() * imageWidth * 3;
    int col = point.x() * 3;

    double red = pixels[row + col];
    double green = pixels[row + col + 1];
    double blue = pixels[row + col + 2];

    if (iteration > 0)
    {
        redPoints->addData(iteration, prevRed, red);
        greenPoints->addData(iteration, prevGreen, green);
        bluePoints->addData(iteration, prevBlue, blue);

        if (iteration > numPoints)
        {
            redPoints->data()->removeBefore(iteration - numPoints);
            greenPoints->data()->removeBefore(iteration - numPoints);
            bluePoints->data()->removeBefore(iteration - numPoints);
        }

        plot->replot(QCustomPlot::rpQueuedReplot);
    }

    prevRed = red;
    prevGreen = green;
    prevBlue = blue;

    iteration++;
}

void ReturnMap::reset()
{
    iteration = 0;

    redPoints->data()->clear();
    greenPoints->data()->clear();
    bluePoints->data()->clear();

    plot->replot();
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
