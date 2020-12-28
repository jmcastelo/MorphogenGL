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

#include "polarkernelplot.h"

PolarKernelPlot::PolarKernelPlot(QString title, QWidget* parent) : QWidget(parent)
{
    // Geometry plot

    geometryPlot = new QCustomPlot(this);
    geometryPlot->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    geometryPlot->setFixedSize(QSize(100, 100));

    geometryPlot->xAxis->setLabel("");
    geometryPlot->yAxis->setLabel("");

    geometryPlot->xAxis->setTickLabels(false);
    geometryPlot->yAxis->setTickLabels(false);

    geometryPlot->axisRect()->setupFullAxesBox(true);

    // Kernel values plot

    kernelValuesPlot = new QCustomPlot(this);
    kernelValuesPlot->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    kernelValuesPlot->setFixedSize(QSize(200, 100));

    kernelValuesPlot->xAxis->setLabel("");
    kernelValuesPlot->yAxis->setLabel("");

    kernelValuesPlot->xAxis->setTickLabels(false);

    kernelValuesPlot->axisRect()->setupFullAxesBox(true);

    kernelValuesPlot->addGraph();
    kernelValuesPlot->addGraph();

    kernelValuesPlot->graph(1)->setPen(QColor(50, 50, 50, 255));
    kernelValuesPlot->graph(1)->setLineStyle(QCPGraph::lsNone);
    kernelValuesPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));

    // Add plots to layout

    layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(geometryPlot);
    layout->addWidget(kernelValuesPlot);
}

PolarKernelPlot::~PolarKernelPlot()
{
    delete geometryPlot;
    delete kernelValuesPlot;
}

void PolarKernelPlot::setGeometryData(std::vector<PolarKernel*> kernels)
{
    geometryPlot->clearPlottables();

    double maxRadius = 0.0;

    for (size_t n = 0; n < kernels.size(); n++)
    {
        QCPCurve* circle = new QCPCurve(geometryPlot->xAxis, geometryPlot->yAxis);
        
        QVector<double> tData;
        QVector<double> xData;
        QVector<double> yData;

        int numPoints = 300;

        for (int i = 0; i < numPoints; i++)
        {
            double t = (6.28318530718 * i) / numPoints;
            
            tData.push_back(t);
            xData.push_back(kernels[n]->radius * cos(t));
            yData.push_back(kernels[n]->radius * sin(t));
        }

        circle->setData(tData, xData, yData);

        if (kernels[n]->radius > maxRadius)
            maxRadius = kernels[n]->radius;
    }

    geometryPlot->addGraph();

    QVector<double> x;
    QVector<double> y;

    for (auto& kernel : kernels)
    {
        for (int i = 0; i < kernel->numElements; i++)
        {
            double angle = (6.28318530718 * i) / kernel->numElements;
            x.push_back(kernel->radius * cos(3.14159265359 * kernel->initialAngle / 180.0 + angle));
            y.push_back(kernel->radius * sin(3.14159265359 * kernel->initialAngle / 180.0 + angle));
        }
    }

    geometryPlot->graph(0)->setData(x, y, true);

    geometryPlot->graph(0)->setPen(QColor(50, 50, 50, 255));
    geometryPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    geometryPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));

    geometryPlot->xAxis->setRange(-maxRadius, maxRadius);
    geometryPlot->yAxis->setRange(-maxRadius, maxRadius);

    geometryPlot->yAxis->setScaleRatio(geometryPlot->xAxis, 1.0);

    geometryPlot->replot();
}

void PolarKernelPlot::setKernelValuesData(PolarKernel* kernel)
{
    QVector<double> indices;
    QVector<double> kernelElementValues;
    
    // Curve

    int numPoints = 300;

    for (int i = 0; i < numPoints; i++)
    {
        indices.push_back(kernel->numElements * static_cast<double>(i) / numPoints);

        double angle = (6.28318530718 * i) / numPoints;
                
        kernelElementValues.push_back(kernel->minimum + (kernel->maximum - kernel->minimum) * (1.0 + sin(kernel->phase * 3.14159265359 / 180.0  + kernel->frequency * angle)) * 0.5);
    }

    kernelValuesPlot->graph(0)->setData(indices, kernelElementValues, true);

    // Points

    indices.clear();
    kernelElementValues.clear();

    for (int i = 0; i < kernel->numElements; i++)
    {
        indices.push_back(i);

        double angle = (6.28318530718 * i) / kernel->numElements;

        kernelElementValues.push_back(kernel->minimum + (kernel->maximum - kernel->minimum) * (1.0 + sin(kernel->phase * 3.14159265359 / 180.0 + kernel->frequency * angle)) * 0.5);
    }

    kernelValuesPlot->graph(1)->setData(indices, kernelElementValues, true);

    // Set ranges

    kernelValuesPlot->xAxis->setRange(0.0, kernel->numElements);
    kernelValuesPlot->yAxis->setRange(kernel->minimum, kernel->maximum);

    kernelValuesPlot->replot();
}

void PolarKernelPlot::resizeEvent(QResizeEvent* event)
{
    geometryPlot->xAxis->setScaleRatio(geometryPlot->yAxis, 1.0);
    geometryPlot->replot();
    QWidget::resizeEvent(event);
}