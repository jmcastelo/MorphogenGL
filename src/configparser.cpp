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
#include "parameters/uniformparameter.h"
#include "parameters/number.h"

#include <vector>
#include <QFile>

void ConfigurationParser::write()
{
    QFile outFile(filename);
    outFile.open(QIODevice::WriteOnly);

    stream.setDevice(&outFile);
    stream.setAutoFormatting(true);

    stream.writeStartDocument();

    stream.writeStartElement("morphogengl");

    stream.writeAttribute("version", mNodeManager->version);

    // Display

    stream.writeStartElement("display");

    stream.writeStartElement("image");
    stream.writeStartElement("width");
    stream.writeCharacters(QString::number(mRenderManager->texWidth()));
    stream.writeEndElement();
    stream.writeStartElement("height");
    stream.writeCharacters(QString::number(mRenderManager->texHeight()));
    stream.writeEndElement();
    stream.writeEndElement();

    stream.writeStartElement("outputnode");
    stream.writeCharacters(mNodeManager->getOutput().toString());
    stream.writeEndElement();

    stream.writeEndElement();

    // Nodes: seeds and operation nodes

    stream.writeStartElement("nodes");

    for (auto [id, seed] : mNodeManager->getSeeds().asKeyValueRange())
        writeSeedNode(id, seed);

    foreach (ImageOperationNode* node, mNodeManager->getOperationNodes())
        writeOperationNode(node);

    stream.writeEndElement();

    stream.writeEndElement();

    stream.writeEndDocument();
}

void ConfigurationParser::writeSeedNode(QUuid id, Seed* seed)
{
    stream.writeStartElement("seednode");

    stream.writeAttribute("id", id.toString());

    stream.writeStartElement("type");
    stream.writeCharacters(QString::number(seed->type()));
    stream.writeEndElement();

    stream.writeStartElement("fixed");
    stream.writeCharacters(QString::number(seed->fixed()));
    stream.writeEndElement();

    /*QPointF position = graphWidget->nodePosition(id);

    stream.writeStartElement("position");

    stream.writeStartElement("x");
    stream.writeCharacters(QString::number(position.x()));
    stream.writeEndElement();

    stream.writeStartElement("y");
    stream.writeCharacters(QString::number(position.y()));
    stream.writeEndElement();

    stream.writeEndElement();*/

    stream.writeEndElement();
}

void ConfigurationParser::writeOperationNode(ImageOperationNode* node)
{
    stream.writeStartElement("operationnode");

    stream.writeAttribute("id", node->id().toString());

    stream.writeStartElement("operation");

    stream.writeAttribute("name", node->operation()->name());
    stream.writeAttribute("enabled", QString::number(node->operation()->enabled()));

    foreach (auto parameter, node->operation()->uniformParameters<float>())
    {
        stream.writeStartElement("parameter");
        stream.writeAttribute("name", parameter->name());
        stream.writeAttribute("type", "float");
        foreach (auto number, parameter->numbers())
        {
            stream.writeStartElement("number");
            stream.writeCharacters(QString::number(number->value()));
            stream.writeEndElement();
        }
        stream.writeEndElement();
    }

    foreach (auto parameter, node->operation()->uniformParameters<int>())
    {
        stream.writeStartElement("parameter");
        stream.writeAttribute("name", parameter->name());
        stream.writeAttribute("type", "int");
        foreach (auto number, parameter->numbers())
        {
            stream.writeStartElement("number");
            stream.writeCharacters(QString::number(number->value()));
            stream.writeEndElement();
        }
        stream.writeEndElement();
    }

    foreach (auto parameter, node->operation()->uniformParameters<unsigned int>())
    {
        stream.writeStartElement("parameter");
        stream.writeAttribute("name", parameter->name());
        stream.writeAttribute("type", "unsigned int");
        foreach (auto number, parameter->numbers())
        {
            stream.writeStartElement("number");
            stream.writeCharacters(QString::number(number->value()));
            stream.writeEndElement();
        }
        stream.writeEndElement();
    }

    stream.writeEndElement();

    stream.writeStartElement("inputs");

    QMap<QUuid, InputData*>::const_iterator i = node->inputs().constBegin();
    while (i != node->inputs().constEnd())
    {
        stream.writeStartElement("input");

        stream.writeAttribute("id", i.key().toString());

        stream.writeStartElement("type");
        stream.writeCharacters(QString::number(static_cast<int>(i.value()->type())));
        stream.writeEndElement();

        stream.writeStartElement("blendfactor");
        stream.writeCharacters(QString::number(i.value()->blendFactor()));
        stream.writeEndElement();

        stream.writeEndElement();
        i++;
    }

    stream.writeEndElement();

    /*QPointF position = graphWidget->nodePosition(node->id);

    stream.writeStartElement("position");

    stream.writeStartElement("x");
    stream.writeCharacters(QString::number(position.x()));
    stream.writeEndElement();

    stream.writeStartElement("y");
    stream.writeCharacters(QString::number(position.y()));
    stream.writeEndElement();

    stream.writeEndElement();*/

    stream.writeEndElement();
}

void ConfigurationParser::read()
{
    QFile inFile(filename);
    inFile.open(QIODevice::ReadOnly);

    xml.setDevice(&inFile);

    if (xml.readNextStartElement() && xml.name() == "morphogengl")
    {
        QUuid outputNodeId;
        int imageWidth = mRenderManager->texWidth();
        int imageHeight = mRenderManager->texHeight();

        while (xml.readNextStartElement())
        {
            if (xml.name() == "nodes")
            {
                mNodeManager->clearLoadedSeeds();
                mNodeManager->clearLoadedOperations();

                QMap<QUuid, QPointF> seedNodePositions;
                QMap<QUuid, QPair<QString, QPointF>> operationNodeData;
                QMap<QUuid, QMap<QUuid, InputData*>> connections;

                while (xml.readNextStartElement())
                {
                    if (xml.name() == "seednode")
                    {
                        QUuid id = QUuid(xml.attributes().value("id").toString());
                        int type = 0;
                        bool fixed = false;
                        QPointF position(0.0, 0.0);

                        while (xml.readNextStartElement())
                        {
                            if (xml.name() == "type")
                            {
                                type = xml.readElementText().toInt();
                            }
                            else if (xml.name() == "fixed")
                            {
                                fixed = xml.readElementText().toInt();
                            }
                            else if (xml.name() == "position")
                            {
                                while (xml.readNextStartElement())
                                {
                                    if (xml.name() == "x")
                                    {
                                        position.setX(xml.readElementText().toFloat());
                                    }
                                    else if (xml.name() == "y")
                                    {
                                        position.setY(xml.readElementText().toFloat());
                                    }
                                    else
                                    {
                                        xml.skipCurrentElement();
                                    }
                                }
                            }
                            else
                            {
                                xml.skipCurrentElement();
                            }
                        }

                        mNodeManager->loadSeed(id, type, fixed);
                        seedNodePositions.insert(id, position);
                    }
                    else if (xml.name() == "operationnode")
                    {
                        QUuid id = QUuid(xml.attributes().value("id").toString());
                        ImageOperation* operation = nullptr;
                        QMap<QUuid, InputData*> inputs;
                        QPointF position(0.0, 0.0);

                        while (xml.readNextStartElement())
                        {
                            if (xml.name() == "operation")
                            {
                                operation = readImageOperation();
                            }
                            else if (xml.name() == "inputs")
                            {
                                while (xml.readNextStartElement())
                                {
                                    if (xml.name() == "input")
                                    {
                                        QUuid srcId = QUuid(xml.attributes().value("id").toString());
                                        InputType type = InputType::Normal;
                                        float blendFactor = 1.0;

                                        while (xml.readNextStartElement())
                                        {
                                            if (xml.name() == "type")
                                            {
                                                type = static_cast<InputType>(xml.readElementText().toInt());
                                            }
                                            else if (xml.name() == "blendfactor")
                                            {
                                                blendFactor = xml.readElementText().toFloat();
                                            }
                                            else
                                            {
                                                xml.skipCurrentElement();
                                            }
                                        }

                                        inputs.insert(srcId, new InputData(type, 0, blendFactor));
                                    }
                                }
                            }
                            else if (xml.name() == "position")
                            {
                                while (xml.readNextStartElement())
                                {
                                    if (xml.name() == "x")
                                    {
                                        position.setX(xml.readElementText().toFloat());
                                    }
                                    else if (xml.name() == "y")
                                    {
                                        position.setY(xml.readElementText().toFloat());
                                    }
                                    else
                                    {
                                        xml.skipCurrentElement();
                                    }
                                }
                            }
                        }

                        if (operation)
                        {
                            mNodeManager->loadOperation(id, operation);
                            connections.insert(id, inputs);
                            operationNodeData.insert(id, QPair<QString, QPointF>(operation->name(), position));
                        }
                    }
                    else
                    {
                        xml.skipCurrentElement();
                    }
                }

                mNodeManager->connectLoadedOperations(connections);

                mNodeManager->swapLoadedSeeds();
                mNodeManager->swapLoadedOperations();

                mNodeManager->sortOperations();

                /*graphWidget->clearScene();

                QMap<QUuid, QPointF>::const_iterator i = seedNodePositions.constBegin();
                while (i != seedNodePositions.constEnd())
                {
                    graphWidget->loadSeedNode(i.key(), i.value());
                    i++;
                }

                QMap<QUuid, QPair<QString, QPointF>>::const_iterator j = operationNodeData.constBegin();
                while (j != operationNodeData.constEnd())
                {
                    graphWidget->loadOperationNode(j.key(), j.value().first, j.value().second);
                    j++;
                }

                graphWidget->connectNodes(connections);*/
            }
            else if (xml.name() == "display")
            {
                while (xml.readNextStartElement())
                {
                    if (xml.name() == "image")
                    {
                        while (xml.readNextStartElement())
                        {
                            if (xml.name() == "width")
                                imageWidth = xml.readElementText().toInt();
                            else if (xml.name() == "height")
                                imageHeight = xml.readElementText().toInt();
                            else
                                xml.skipCurrentElement();
                        }

                        emit newImageSizeRead(imageWidth, imageHeight);
                    }
                    else if (xml.name() == "outputnode")
                    {
                        outputNodeId = QUuid(xml.readElementText());
                    }
                    else
                    {
                        xml.skipCurrentElement();
                    }
                }
            }
            else
            {
                xml.skipCurrentElement();
            }   
        }

        if (!outputNodeId.isNull())
            mNodeManager->setOutput(outputNodeId);

        //emit newImageSizeRead(imageWidth, imageHeight);
    }

    if (xml.tokenType() == QXmlStreamReader::Invalid)
        xml.readNext();

    if (xml.hasError())
        xml.raiseError();

    inFile.close();
}

ImageOperation* ConfigurationParser::readImageOperation()
{
    QString operationName = xml.attributes().value("name").toString();

    bool enabled = xml.attributes().value("enabled").toInt();

    std::vector<bool> boolParameters;
    std::vector<int> intParameters;
    std::vector<float> floatParameters;
    std::vector<int> interpolationFlagParameters;
    std::vector<float> kernelElements;
    std::vector<float> matrixElements;

    while (xml.readNextStartElement())
    {
        if (xml.name() == "parameter")
        {
            QString parameterType = xml.attributes().value("type").toString();

            if (parameterType == "bool")
                boolParameters.push_back(xml.readElementText().toInt());
            else if (parameterType == "int")
                intParameters.push_back(xml.readElementText().toInt());
            else if (parameterType == "float")
                floatParameters.push_back(xml.readElementText().toFloat());
            else if (parameterType == "interpolationflag")
                interpolationFlagParameters.push_back(xml.readElementText().toInt());
            else if (parameterType == "kernel")
            {
                while (xml.readNextStartElement())
                {
                    if (xml.name() == "element")
                        kernelElements.push_back(xml.readElementText().toFloat());
                     else
                        xml.skipCurrentElement();
                }
            }
            else if (parameterType == "matrix")
            {
                while (xml.readNextStartElement())
                {
                    if (xml.name() == "element")
                        matrixElements.push_back(xml.readElementText().toFloat());
                    else
                        xml.skipCurrentElement();
                }
            }
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    return mNodeManager->loadImageOperation(
                operationName,
                enabled,
                boolParameters,
                intParameters,
                floatParameters,
                interpolationFlagParameters,
                kernelElements,
                matrixElements);
}
