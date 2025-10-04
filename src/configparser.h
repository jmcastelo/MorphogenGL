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



#include "factory.h"
#include "nodemanager.h"
#include "rendermanager.h"
#include "graphwidget.h"

#include <QObject>
#include <QString>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>



class ConfigurationParser: public QObject
{
    Q_OBJECT

public:
    ConfigurationParser(Factory* factory, NodeManager* nodeManager, RenderManager* renderManager, GraphWidget* graphWidget) :
        mFactory { factory },
        mNodeManager { nodeManager },
        mRenderManager { renderManager},
        mGraphWidget { graphWidget }
    {}

    void setFilename(QString theFilename) { filename = theFilename; }
    void write();
    void read();

signals:
    void newImageSizeRead(int width, int height);

private:
    Factory* mFactory;
    NodeManager* mNodeManager;
    RenderManager* mRenderManager;
    GraphWidget* mGraphWidget;
    QString filename;

    void writeSeedNode(QUuid id, Seed* seed, QXmlStreamWriter& stream);
    void writeOperationNode(ImageOperationNode* node, QXmlStreamWriter& stream);
    // ImageOperation* readImageOperation();
};
