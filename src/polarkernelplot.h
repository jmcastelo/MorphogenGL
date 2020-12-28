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

#include "parameter.h"
#include <qcustomplot.h>
#include <cmath>
#include <vector>
#include <QWidget>
#include <QVector>
#include <QString>
#include <QHBoxLayout>
#include <QResizeEvent>

class PolarKernelPlot : public QWidget
{
    Q_OBJECT

public:
    QHBoxLayout* layout;

    PolarKernelPlot(QString title, QWidget* parent = nullptr);
    ~PolarKernelPlot();

    void setGeometryData(std::vector<PolarKernel*> kernels);
    void setKernelValuesData(PolarKernel* kernel);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QCustomPlot* geometryPlot;
    QCustomPlot* kernelValuesPlot;
};