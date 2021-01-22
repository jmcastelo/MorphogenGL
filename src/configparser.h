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

#include "generator.h"
#include "graphwidget.h"
#include "heart.h"
#include <QObject>
#include <QString>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

class ConfigurationParser : public QObject
{
    Q_OBJECT

public:
    ConfigurationParser(GeneratorGL* theGenerator, GraphWidget* graph, Heart* theHeart) :
        generator { theGenerator },
        graphWidget { graph },
        heart { theHeart }
    {}

    void setFilename(QString theFilename) { filename = theFilename; }
    void write();
    void read();

private:
    GeneratorGL* generator;
    GraphWidget* graphWidget;
    Heart* heart;
    QString filename;
    QXmlStreamWriter stream;
    QXmlStreamReader xml;

    void writeSeedNode(QUuid id, Seed* seed);
    void writeOperationNode(ImageOperationNode* node);
    ImageOperation* readImageOperation();

signals:
    void updateImageSize(int width, int height);
};
