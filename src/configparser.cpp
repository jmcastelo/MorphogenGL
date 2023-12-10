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
#include "parameter.h"
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

    stream.writeAttribute("version", generator->version);

    // Nodes: seeds and operation nodes

    stream.writeStartElement("nodes");

    QMap<QUuid, Seed*>::const_iterator i = generator->getSeeds().constBegin();
    while (i != generator->getSeeds().constEnd())
    {
        writeSeedNode(i.key(), i.value());
        i++;
    }

    foreach (ImageOperationNode* node, generator->getOperationNodes())
    {
        writeOperationNode(node);
    }

    stream.writeEndElement();

    // Display

    stream.writeStartElement("display");

    stream.writeStartElement("image");
    stream.writeStartElement("width");
    stream.writeCharacters(QString::number(generator->getWidth()));
    stream.writeEndElement();
    stream.writeStartElement("height");
    stream.writeCharacters(QString::number(generator->getHeight()));
    stream.writeEndElement();
    stream.writeEndElement();

    stream.writeStartElement("window");
    stream.writeStartElement("width");
    stream.writeCharacters(QString::number(heart->getMorphoWidgetWidth()));
    stream.writeEndElement();
    stream.writeStartElement("height");
    stream.writeCharacters(QString::number(heart->getMorphoWidgetHeight()));
    stream.writeEndElement();
    stream.writeEndElement();

    stream.writeStartElement("outputnode");
    stream.writeCharacters(generator->getOutput().toString());
    stream.writeEndElement();

    stream.writeEndElement();

    stream.writeEndElement();

    stream.writeEndDocument();
}

void ConfigurationParser::writeSeedNode(QUuid id, Seed* seed)
{
    stream.writeStartElement("seednode");

    stream.writeAttribute("id", id.toString());

    stream.writeStartElement("type");
    stream.writeCharacters(QString::number(seed->getType()));
    stream.writeEndElement();

    stream.writeStartElement("fixed");
    stream.writeCharacters(QString::number(seed->isFixed()));
    stream.writeEndElement();

    QPointF position = graphWidget->nodePosition(id);

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

void ConfigurationParser::writeOperationNode(ImageOperationNode* node)
{
    stream.writeStartElement("operationnode");

    stream.writeAttribute("id", node->id.toString());

    stream.writeStartElement("operation");

    stream.writeAttribute("name", node->operation->getName());
    stream.writeAttribute("enabled", QString::number(node->operation->isEnabled()));

    for (auto parameter: node->operation->getIntParameters())
    {
        stream.writeStartElement("parameter");
        stream.writeAttribute("name", parameter->name);
        stream.writeAttribute("type", "int");
        stream.writeCharacters(QString::number(parameter->number->value));
        stream.writeEndElement();
    }

    for (auto parameter: node->operation->getFloatParameters())
    {
        stream.writeStartElement("parameter");
        stream.writeAttribute("name", parameter->name);
        stream.writeAttribute("type", "float");
        stream.writeCharacters(QString::number(parameter->number->value));
        stream.writeEndElement();
    }

    for (auto parameter: node->operation->getOptionsIntParameters())
    {
        stream.writeStartElement("parameter");
        stream.writeAttribute("name", parameter->name);
        stream.writeAttribute("type", "int");
        stream.writeCharacters(QString::number(parameter->value));
        stream.writeEndElement();
    }

    for (auto parameter: node->operation->getOptionsGLenumParameters())
    {
        stream.writeStartElement("parameter");
        stream.writeAttribute("name", parameter->name);
        stream.writeAttribute("type", "interpolationflag");
        stream.writeCharacters(QString::number(parameter->value));
        stream.writeEndElement();
    }

    if (node->operation->getKernelParameter())
    {
        stream.writeStartElement("parameter");
        stream.writeAttribute("name", node->operation->getKernelParameter()->name);
        stream.writeAttribute("type", "kernel");

        for (auto element: node->operation->getKernelParameter()->numbers)
        {
            stream.writeStartElement("element");
            stream.writeCharacters(QString::number(element->value));
            stream.writeEndElement();
        }

        stream.writeEndElement();
    }

    if (node->operation->getMatrixParameter())
    {
        stream.writeStartElement("parameter");
        stream.writeAttribute("name", node->operation->getMatrixParameter()->name);
        stream.writeAttribute("type", "matrix");

        for (auto element : node->operation->getMatrixParameter()->numbers)
        {
            stream.writeStartElement("element");
            stream.writeCharacters(QString::number(element->value));
            stream.writeEndElement();
        }

        stream.writeEndElement();
    }

    if (node->operation->getPolarKernelParameter())
    {
        stream.writeStartElement("parameter");
        stream.writeAttribute("name", node->operation->getPolarKernelParameter()->name);
        stream.writeAttribute("type", "polarkernel");

        stream.writeStartElement("centerelement");
        stream.writeCharacters(QString::number(node->operation->getPolarKernelParameter()->centerElement));
        stream.writeEndElement();

        for (auto polarKernel : node->operation->getPolarKernelParameter()->polarKernels)
        {
            stream.writeStartElement("polarkernel");

            stream.writeStartElement("numelements");
            stream.writeCharacters(QString::number(polarKernel->numElements));
            stream.writeEndElement();

            stream.writeStartElement("radius");
            stream.writeCharacters(QString::number(polarKernel->radius));
            stream.writeEndElement();

            stream.writeStartElement("initialangle");
            stream.writeCharacters(QString::number(polarKernel->initialAngle));
            stream.writeEndElement();

            stream.writeStartElement("frequency");
            stream.writeCharacters(QString::number(polarKernel->frequency));
            stream.writeEndElement();

            stream.writeStartElement("phase");
            stream.writeCharacters(QString::number(polarKernel->phase));
            stream.writeEndElement();

            stream.writeStartElement("minimum");
            stream.writeCharacters(QString::number(polarKernel->minimum));
            stream.writeEndElement();

            stream.writeStartElement("maximum");
            stream.writeCharacters(QString::number(polarKernel->maximum));
            stream.writeEndElement();

            stream.writeEndElement();
        }

        stream.writeEndElement();
    }

    stream.writeEndElement();

    stream.writeStartElement("inputs");

    QMap<QUuid, InputData*>::const_iterator i = node->inputs.constBegin();
    while (i != node->inputs.constEnd())
    {
        stream.writeStartElement("input");

        stream.writeAttribute("id", i.key().toString());

        stream.writeStartElement("type");
        stream.writeCharacters(QString::number(static_cast<int>(i.value()->type)));
        stream.writeEndElement();

        stream.writeStartElement("blendfactor");
        stream.writeCharacters(QString::number(i.value()->blendFactor));
        stream.writeEndElement();

        stream.writeEndElement();
        i++;
    }

    stream.writeEndElement();

    QPointF position = graphWidget->nodePosition(node->id);

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

    xml.setDevice(&inFile);

    if (xml.readNextStartElement() && xml.name() == "morphogengl")
    {
        QUuid outputNodeId;

        while (xml.readNextStartElement())
        {
            if (xml.name() == "nodes")
            {
                generator->clearLoadedSeeds();
                generator->clearLoadedOperations();

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

                        generator->loadSeed(id, type, fixed);
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

                        generator->loadOperation(id, operation);
                        connections.insert(id, inputs);
                        operationNodeData.insert(id, QPair<QString, QPointF>(operation->getName(), position));
                    }
                    else
                    {
                        xml.skipCurrentElement();
                    }
                }

                generator->connectLoadedOperations(connections);

                generator->swapLoadedSeeds();
                generator->swapLoadedOperations();

                generator->sortOperations();

                graphWidget->clearScene();

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

                graphWidget->connectNodes(connections);
            }
            else if (xml.name() == "display")
            {
                while (xml.readNextStartElement())
                {
                    if (xml.name() == "image")
                    {
                        int imageWidth = generator->getWidth();
                        int imageHeight = generator->getHeight();

                        while (xml.readNextStartElement())
                        {
                            if (xml.name() == "width")
                                imageWidth = xml.readElementText().toInt();
                            else if (xml.name() == "height")
                                imageHeight = xml.readElementText().toInt();
                            else
                                xml.skipCurrentElement();
                        }

                        emit updateImageSize(imageWidth, imageHeight);
                    }
                    else if (xml.name() == "window")
                    {
                        int windowWidth = heart->getMorphoWidgetWidth();
                        int windowHeight = heart->getMorphoWidgetHeight();

                        while (xml.readNextStartElement())
                        {
                            if (xml.name() == "width")
                                windowWidth = xml.readElementText().toInt();
                            else if (xml.name() == "height")
                                windowHeight = xml.readElementText().toInt();
                            else
                                xml.skipCurrentElement();
                        }

                        heart->resizeMorphoWidget(windowWidth, windowHeight);
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
            generator->setOutput(outputNodeId);
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
    std::vector<PolarKernel*> polarKernels;

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
            else if (parameterType == "polarkernel")
            {
                while (xml.readNextStartElement())
                {
                    if (xml.name() == "centerelement")
                    {
                        floatParameters.push_back(xml.readElementText().toFloat());
                    }
                    else if (xml.name() == "polarkernel")
                    {
                        PolarKernel* polarKernel = new PolarKernel(8, 0.01f, 0.0f, 1.0f, 0.0f, -1.0f, 1.0f);

                        while (xml.readNextStartElement())
                        {
                            if (xml.name() == "numelements")
                                polarKernel->numElements = xml.readElementText().toInt();
                            else if (xml.name() == "radius")
                                polarKernel->radius = xml.readElementText().toFloat();
                            else if (xml.name() == "initialangle")
                                polarKernel->initialAngle = xml.readElementText().toFloat();
                            else if (xml.name() == "frequency")
                                polarKernel->frequency = xml.readElementText().toFloat();
                            else if (xml.name() == "phase")
                                polarKernel->phase = xml.readElementText().toFloat();
                            else if (xml.name() == "minimum")
                                polarKernel->minimum = xml.readElementText().toFloat();
                            else if (xml.name() == "maximum")
                                polarKernel->maximum = xml.readElementText().toFloat();
                            else
                                xml.skipCurrentElement();
                        }

                        polarKernels.push_back(polarKernel);
                    }
                    else
                    {
                        xml.skipCurrentElement();
                    }
                }
            }
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    return generator->loadImageOperation(
                operationName,
                enabled,
                boolParameters,
                intParameters,
                floatParameters,
                interpolationFlagParameters,
                kernelElements,
                matrixElements,
                polarKernels);
}
