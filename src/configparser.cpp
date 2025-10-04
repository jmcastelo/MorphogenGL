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



#include "configparser.h"
#include "operationparser.h"

#include <QFile>



void ConfigurationParser::write()
{
    QFile outFile(filename);
    outFile.open(QIODevice::WriteOnly);

    QXmlStreamWriter mStream;
    mStream.setDevice(&outFile);
    mStream.setAutoFormatting(true);

    mStream.writeStartDocument();

    mStream.writeStartElement("fosforo");

    mStream.writeAttribute("version", mNodeManager->version);

    // Display

    mStream.writeStartElement("display");

    mStream.writeStartElement("image");
    mStream.writeStartElement("width");
    mStream.writeCharacters(QString::number(mRenderManager->texWidth()));
    mStream.writeEndElement();
    mStream.writeStartElement("height");
    mStream.writeCharacters(QString::number(mRenderManager->texHeight()));
    mStream.writeEndElement();
    mStream.writeEndElement();

    mStream.writeStartElement("output_node");
    mStream.writeCharacters(mNodeManager->outputId().toString());
    mStream.writeEndElement();

    mStream.writeEndElement();

    // Nodes: seeds and operation nodes

    mStream.writeStartElement("nodes");

    for (auto [id, seed] : mNodeManager->seedsMap().asKeyValueRange()) {
        writeSeedNode(id, seed, mStream);
    }

    foreach (ImageOperationNode* node, mNodeManager->operationNodesMap()) {
        writeOperationNode(node, mStream);
    }

    mStream.writeEndElement();

    mStream.writeEndElement();

    mStream.writeEndDocument();
}



void ConfigurationParser::writeSeedNode(QUuid id, Seed* seed, QXmlStreamWriter &stream)
{
    stream.writeStartElement("seed_node");

    stream.writeAttribute("id", id.toString());

    stream.writeStartElement("type");
    stream.writeCharacters(QString::number(seed->type()));
    stream.writeEndElement();

    stream.writeStartElement("fixed");
    stream.writeCharacters(QString::number(seed->fixed()));
    stream.writeEndElement();

    stream.writeStartElement("image_filename");
    stream.writeCharacters(seed->imageFilename());
    stream.writeEndElement();

    QPointF position = mGraphWidget->nodePosition(id);

    stream.writeStartElement("position");

    stream.writeStartElement("x");
    stream.writeCharacters(QString::number(position.x()));
    stream.writeEndElement();

    stream.writeStartElement("y");
    stream.writeCharacters(QString::number(position.y()));
    stream.writeEndElement();

    stream.writeEndElement();

    stream.writeEndElement();
}



void ConfigurationParser::writeOperationNode(ImageOperationNode* node, QXmlStreamWriter &stream)
{
    stream.writeStartElement("operation_node");

    stream.writeAttribute("id", node->id().toString());

    OperationParser opParser;

    opParser.writeOperation(node->operation(), stream, true);

    stream.writeStartElement("inputs");

    for (auto [id, inData]: node->inputs().asKeyValueRange())
    {
        stream.writeStartElement("input");

        stream.writeAttribute("id", id.toString());

        stream.writeStartElement("type");
        stream.writeCharacters(QString::number(static_cast<int>(inData->type())));
        stream.writeEndElement();

        stream.writeStartElement("blendfactor");
        stream.writeCharacters(QString::number(inData->blendFactor()));
        stream.writeEndElement();

        stream.writeEndElement();
    }

    stream.writeEndElement();

    QPointF position = mGraphWidget->nodePosition(node->id());

    stream.writeStartElement("position");

    stream.writeStartElement("x");
    stream.writeCharacters(QString::number(position.x()));
    stream.writeEndElement();

    stream.writeStartElement("y");
    stream.writeCharacters(QString::number(position.y()));
    stream.writeEndElement();

    stream.writeEndElement();

    stream.writeEndElement();
}



void ConfigurationParser::read()
{
    QFile inFile(filename);
    inFile.open(QIODevice::ReadOnly);

    QXmlStreamReader mStream;
    mStream.setDevice(&inFile);

    OperationParser opParser;

    if (mStream.readNextStartElement() && mStream.name() == "fosforo")
    {
        mFactory->clear();

        QUuid outputNodeId;
        int imageWidth = mRenderManager->texWidth();
        int imageHeight = mRenderManager->texHeight();

        while (mStream.readNextStartElement())
        {
            if (mStream.name() == "nodes")
            {
                QMap<QUuid, QMap<QUuid, InputData*>> connections;

                while (mStream.readNextStartElement())
                {
                    if (mStream.name() == "seed_node")
                    {
                        QUuid id = QUuid(mStream.attributes().value("id").toString());
                        int type = 0;
                        bool fixed = false;
                        QString imageFilename;
                        QPointF position(0.0, 0.0);

                        while (mStream.readNextStartElement())
                        {
                            if (mStream.name() == "type")
                            {
                                type = mStream.readElementText().toInt();
                            }
                            else if (mStream.name() == "fixed")
                            {
                                fixed = mStream.readElementText().toInt();
                            }
                            else if (mStream.name() == "image_filename")
                            {
                                imageFilename = mStream.readElementText();
                            }
                            else if (mStream.name() == "position")
                            {
                                while (mStream.readNextStartElement())
                                {
                                    if (mStream.name() == "x")
                                    {
                                        position.setX(mStream.readElementText().toFloat());
                                    }
                                    else if (mStream.name() == "y")
                                    {
                                        position.setY(mStream.readElementText().toFloat());
                                    }
                                    else
                                    {
                                        mStream.skipCurrentElement();
                                    }
                                }
                            }
                            else
                            {
                                mStream.skipCurrentElement();
                            }
                        }

                        Seed* seed = new Seed(type, fixed, imageFilename);
                        mFactory->addSeed(id, seed);
                        mGraphWidget->setNodePosition(id, position);
                    }
                    else if (mStream.name() == "operation_node")
                    {
                        QUuid id = QUuid(mStream.attributes().value("id").toString());
                        ImageOperation* operation = new ImageOperation();
                        QMap<QUuid, InputData*> inputs;
                        QPointF position(0.0, 0.0);

                        while (mStream.readNextStartElement())
                        {
                            if (mStream.name() == "operation")
                            {
                                opParser.readOperation(operation, mStream, true);
                            }
                            else if (mStream.name() == "inputs")
                            {
                                while (mStream.readNextStartElement())
                                {
                                    if (mStream.name() == "input")
                                    {
                                        QUuid srcId = QUuid(mStream.attributes().value("id").toString());
                                        InputType type = InputType::Normal;
                                        float blendFactor = 1.0;

                                        while (mStream.readNextStartElement())
                                        {
                                            if (mStream.name() == "type")
                                            {
                                                type = static_cast<InputType>(mStream.readElementText().toInt());
                                            }
                                            else if (mStream.name() == "blendfactor")
                                            {
                                                blendFactor = mStream.readElementText().toFloat();
                                            }
                                            else
                                            {
                                                mStream.skipCurrentElement();
                                            }
                                        }

                                        inputs.insert(srcId, new InputData(type, 0, blendFactor));
                                    }
                                }

                                connections.insert(id, inputs);
                            }
                            else if (mStream.name() == "position")
                            {
                                while (mStream.readNextStartElement())
                                {
                                    if (mStream.name() == "x")
                                    {
                                        position.setX(mStream.readElementText().toFloat());
                                    }
                                    else if (mStream.name() == "y")
                                    {
                                        position.setY(mStream.readElementText().toFloat());
                                    }
                                    else
                                    {
                                        mStream.skipCurrentElement();
                                    }
                                }
                            }
                        }

                        mFactory->addOperation(id, operation);
                        mGraphWidget->setNodePosition(id, position);
                    }
                    else
                    {
                        mStream.skipCurrentElement();
                    }
                }

                mNodeManager->connectOperations(connections);
                mNodeManager->sortOperations();
            }
            else if (mStream.name() == "display")
            {
                while (mStream.readNextStartElement())
                {
                    if (mStream.name() == "image")
                    {
                        while (mStream.readNextStartElement())
                        {
                            if (mStream.name() == "width")
                                imageWidth = mStream.readElementText().toInt();
                            else if (mStream.name() == "height")
                                imageHeight = mStream.readElementText().toInt();
                            else
                                mStream.skipCurrentElement();
                        }
                    }
                    else if (mStream.name() == "output_node")
                    {
                        outputNodeId = QUuid(mStream.readElementText());
                    }
                    else
                    {
                        mStream.skipCurrentElement();
                    }
                }
            }
            else
            {
                mStream.skipCurrentElement();
            }   
        }

        if (!outputNodeId.isNull())
            mNodeManager->setOutput(outputNodeId);

        emit newImageSizeRead(imageWidth, imageHeight);
    }

    if (mStream.tokenType() == QXmlStreamReader::Invalid)
        mStream.readNext();

    if (mStream.hasError())
        mStream.raiseError();

    inFile.close();
}
