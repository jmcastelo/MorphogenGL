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

void ConfigurationParser::write()
{
    QFile outFile(filename);
    outFile.open(QIODevice::WriteOnly);

    stream.setDevice(&outFile);
    stream.setAutoFormatting(true);

    stream.writeStartDocument();

    stream.writeStartElement("morphogengl");

    // Pipelines

    stream.writeStartElement("pipelines");

    for (auto pipeline: generator->pipelines)
    {
        stream.writeStartElement("pipeline");
        stream.writeAttribute("blendfactor", QString::number(pipeline->blendFactor));
        writeImageOperations(pipeline);
        stream.writeEndElement();
    }

    stream.writeStartElement("outputpipeline");
    writeImageOperations(generator->outputPipeline);
    stream.writeEndElement();

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

    stream.writeStartElement("mask");
    stream.writeCharacters(QString::number(generator->applyMask));
    stream.writeEndElement();

    stream.writeEndElement();

    stream.writeEndElement();

    stream.writeEndDocument();

    outFile.close();
}

void ConfigurationParser::writeImageOperations(Pipeline *pipeline)
{
    for (auto operation: pipeline->imageOperations)
    {
        stream.writeStartElement("operation");
        stream.writeAttribute("name", operation->getName());
        stream.writeAttribute("enabled", QString::number(operation->isEnabled()));

        for (auto parameter: operation->getBoolParameters())
        {
            stream.writeStartElement("parameter");
            stream.writeAttribute("name", parameter->name);
            stream.writeAttribute("type", "bool");
            stream.writeCharacters(QString::number(parameter->value));
            stream.writeEndElement();
        }

        for (auto parameter: operation->getIntParameters())
        {
            stream.writeStartElement("parameter");
            stream.writeAttribute("name", parameter->name);
            stream.writeAttribute("type", "int");
            stream.writeCharacters(QString::number(parameter->value));
            stream.writeEndElement();
        }

        for (auto parameter: operation->getFloatParameters())
        {
            stream.writeStartElement("parameter");
            stream.writeAttribute("name", parameter->name);
            stream.writeAttribute("type", "float");
            stream.writeCharacters(QString::number(parameter->value));
            stream.writeEndElement();
        }

        for (auto parameter: operation->getOptionsIntParameters())
        {
            stream.writeStartElement("parameter");
            stream.writeAttribute("name", parameter->name);
            stream.writeAttribute("type", "int");
            stream.writeCharacters(QString::number(parameter->value));
            stream.writeEndElement();
        }

        for (auto parameter: operation->getOptionsGLenumParameters())
        {
            stream.writeStartElement("parameter");
            stream.writeAttribute("name", parameter->name);
            stream.writeAttribute("type", "interpolationflag");
            stream.writeCharacters(QString::number(parameter->value));
            stream.writeEndElement();
        }

        if (operation->getKernelParameter())
        {
            stream.writeStartElement("parameter");
            stream.writeAttribute("name", operation->getKernelParameter()->name);
            stream.writeAttribute("type", "kernel");

            for (auto element: operation->getKernelParameter()->values)
            {
                stream.writeStartElement("element");
                stream.writeCharacters(QString::number(element));
                stream.writeEndElement();
            }

            stream.writeEndElement();
        }

        if (operation->getMatrixParameter())
        {
            stream.writeStartElement("parameter");
            stream.writeAttribute("name", operation->getMatrixParameter()->name);
            stream.writeAttribute("type", "matrix");

            for (auto element : operation->getMatrixParameter()->values)
            {
                stream.writeStartElement("element");
                stream.writeCharacters(QString::number(element));
                stream.writeEndElement();
            }

            stream.writeEndElement();
        }

        if (operation->getPolarKernelParameter())
        {
            stream.writeStartElement("parameter");
            stream.writeAttribute("name", operation->getPolarKernelParameter()->name);
            stream.writeAttribute("type", "polarkernel");

            stream.writeStartElement("centerelement");
            stream.writeCharacters(QString::number(operation->getPolarKernelParameter()->centerElement));
            stream.writeEndElement();

            for (auto polarKernel : operation->getPolarKernelParameter()->polarKernels)
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
    }
}

void ConfigurationParser::read()
{
    QFile inFile(filename);
    inFile.open(QIODevice::ReadOnly);

    xml.setDevice(&inFile);

    if (xml.readNextStartElement() && xml.name() == "morphogengl")
    {
        while (xml.readNextStartElement())
        {
            if (xml.name() == "pipelines")
            {
                generator->pipelines.clear();
                generator->outputPipeline->imageOperations.clear();

                while (xml.readNextStartElement())
                {
                    if (xml.name() == "pipeline")
                    {
                        float blendFactor = xml.attributes().value("blendfactor").toFloat();
                        generator->loadPipeline(blendFactor);
                        readImageOperations(generator->pipelines.back());
                    }
                    else if (xml.name() == "outputpipeline")
                    {
                        readImageOperations(generator->outputPipeline);
                    }
                    else
                    {
                        xml.skipCurrentElement();
                    }
                }
            }
            else if (xml.name() == "display")
            {

                if (xml.readNextStartElement() && xml.name() == "image")
                {
                    int imageWidth = generator->getWidth();
                    int imageHeight = generator->getHeight();

                    if (xml.readNextStartElement() && xml.name() == "width")
                        imageWidth = xml.readElementText().toInt();
                    if (xml.readNextStartElement() && xml.name() == "height")
                        imageHeight = xml.readElementText().toInt();

                    emit updateImageSize(imageWidth, imageHeight);

                    xml.skipCurrentElement();
                }

                if (xml.readNextStartElement() && xml.name() == "window")
                {
                    int windowWidth = heart->getMorphoWidgetWidth();
                    int windowHeight = heart->getMorphoWidgetHeight();

                    if (xml.readNextStartElement() && xml.name() == "width")
                        windowWidth = xml.readElementText().toInt();
                    if (xml.readNextStartElement() && xml.name() == "height")
                        windowHeight = xml.readElementText().toInt();

                   heart->resizeMorphoWidget(windowWidth, windowHeight);

                   xml.skipCurrentElement();
                }

                if (xml.readNextStartElement() && xml.name() == "mask")
                {
                    bool applyMask = xml.readElementText().toInt();
                    
                    emit updateMask(applyMask);

                    xml.skipCurrentElement();
                }
            }
            else
            {
                xml.skipCurrentElement();
            }   
        }
    }

    if (xml.tokenType() == QXmlStreamReader::Invalid)
        xml.readNext();

    if (xml.hasError())
        xml.raiseError();

    inFile.close();
}

void ConfigurationParser::readImageOperations(Pipeline *pipeline)
{
    while (xml.readNextStartElement())
    {
        if (xml.name() == "operation")
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

            pipeline->loadImageOperation(
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
        else
        {
            xml.skipCurrentElement();
        }
    }
}
