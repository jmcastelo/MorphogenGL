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

#include "imageoperations.h"
#include "parameter.h"



// Image operation base class

ImageOperation::ImageOperation(bool on, QOpenGLContext* mainContext) : enabled { on }
{
    context = mainContext;
    blender = new Blender(":/shaders/screen.vert", ":/shaders/blend.frag", mainContext);
}



ImageOperation::ImageOperation(const ImageOperation& operation) :
    enabled { operation.enabled },
    noParameters { operation.noParameters },
    context { operation.context }
{
    blender = new Blender(":/shaders/screen.vert", ":/shaders/blend.frag", context);
}



ImageOperation::~ImageOperation()
{
    delete blender;
    delete fbo;
}



void ImageOperation::applyOperation()
{
    if (blenderEnabled)
        blender->blend();

    if (enabled)
        fbo->draw();
    else
        fbo->identity();
}



void ImageOperation::blit()
{
    if (blitEnabled)
        fbo->blit();
}



void ImageOperation::clear()
{
    fbo->clear();
}



void ImageOperation::setInputData(QList<InputData *> data)
{
    if (data.size() == 1)
    {
        blenderEnabled = false;
        fbo->setInputTextureID(*data[0]->textureID);
    }
    else
    {
        blenderEnabled = true;
        fbo->setInputTextureID(*blender->getTextureID());
        blender->setInputData(data);
    }
}



void ImageOperation::adjustMinMax(float value, float minValue, float maxValue, float& min, float& max)
{
    if (value < minValue) min = value;
    else min = minValue;

    if (value > maxValue) max = value;
    else max = maxValue;
}



// Convolution

QString AverageBrightness::name = "Average brightness";

AverageBrightness::AverageBrightness(bool on, QOpenGLContext* mainContext, int theSize, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/average-brightness.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    size = new IntParameter("Size (px)", 0, this, theSize, 1, 20, 1, 1000, false);

    float min, max;
    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 1, this, theOpacity, min, max, 0.0f, 1.0f);
}



AverageBrightness::AverageBrightness(const AverageBrightness &operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/average-brightness.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    size = new IntParameter(*operation.size);
    size->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



AverageBrightness::~AverageBrightness()
{
    delete size;
    delete opacity;
}



void AverageBrightness::setParameters()
{
    setIntParameter(0, size->value());
    setFloatParameter(1, opacity->value());
}



void AverageBrightness::setIntParameter(int index, int value)
{
    if (index == 0)
    {
        //float x = (value - 0.5) / fbo->width;
        //float y = (value - 0.5) / fbo->height;

        /*QVector2D offset[] = {
            QVector2D(-x, y),
            QVector2D(0.0f, y),
            QVector2D(x, y),
            QVector2D(-x, 0.0f),
            QVector2D(0.0f, 0.0f),
            QVector2D(x, 0.0f),
            QVector2D(-x, -y),
            QVector2D(0.0f, -y),
            QVector2D(x, -y)
        };*/

        QVector2D offset[25];

        int element = 0;

        for (int y = -2; y <= 2; y++)
        {
            for (int x = -2; x <= 2; x++)
            {
                offset[element++] = QVector2D(static_cast<float>(x * (value - 0.5)) / fbo->width, static_cast<float>(y * (value - 0.5)) / fbo->height);
            }
        }

        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValueArray("offset", offset, 25);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



void AverageBrightness::setFloatParameter(int index, float value)
{
    if (index == 1)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Bilateral filter

QString BilateralFilter::name = "Bilateral filter";

BilateralFilter::BilateralFilter(bool on, QOpenGLContext* mainContext, int theNumSideElements, float theSize, float theSpatialSigma, float theRangeSigma, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/bilateral.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    numSideElements = new IntParameter("Side elements", 0, this, theNumSideElements, 2, 7, 2, 7, false);

    float min, max;
    adjustMinMax(theSize, 1.0f, 100.0f, min, max);
    size = new FloatParameter("Kernel size", 0, this, theSize, min, max, 0.0f, 1000.0f);

    adjustMinMax(theSpatialSigma, 1.0e-6f, 100.0f, min, max);
    spatialSigma = new FloatParameter("Spatial sigma", 1, this, theSpatialSigma, min, max, 1.0e-6f, 1.0e6f);

    adjustMinMax(theRangeSigma, 1.0e-6f, 1.0f, min, max);
    rangeSigma = new FloatParameter("Range sigma", 2, this, theRangeSigma, min, max, 1.0e-6f, 1.0e6f);

    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 3, this, theOpacity, min, max, 0.0f, 1.0f);
}



BilateralFilter::BilateralFilter(const BilateralFilter& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/bilateral.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    numSideElements = new IntParameter(*operation.numSideElements);
    numSideElements->setOperation(this);

    size = new FloatParameter(*operation.size);
    size->setOperation(this);

    spatialSigma = new FloatParameter(*operation.spatialSigma);
    spatialSigma->setOperation(this);

    rangeSigma = new FloatParameter(*operation.rangeSigma);
    rangeSigma->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



BilateralFilter::~BilateralFilter()
{
    delete numSideElements;
    delete size;
    delete spatialSigma;
    delete rangeSigma;
    delete opacity;
}



void BilateralFilter::setParameters()
{
    setIntParameter(0, numSideElements->value());
    setFloatParameter(2, rangeSigma->value());
    setFloatParameter(3, opacity->value());
}



void BilateralFilter::computeOffsets()
{
    QVector2D *offset = new QVector2D[numSideElements->value() * numSideElements->value()];

    for (int i = 0; i < numSideElements->value(); i++)
    {
        float x = size->value() * (-0.5f + i / (numSideElements->value() - 1.0f)) / fbo->width;

        for (int j = 0; j < numSideElements->value(); j++)
        {
            float y = size->value() * (-0.5f + j / (numSideElements->value() - 1.0f)) / fbo->height;

            offset[i * numSideElements->value() + j] = QVector2D(x, y);
        }
    }

    fbo->makeCurrent();
    fbo->program->bind();
    fbo->program->setUniformValueArray("offset", offset, numSideElements->value() * numSideElements->value());
    fbo->program->release();
    fbo->doneCurrent();

    delete [] offset;
}



void BilateralFilter::computeSpatialKernel()
{
    float* spatialKernel = new float[numSideElements->value() * numSideElements->value()];

    for (int i = 0; i < numSideElements->value(); i++)
    {
        float x = size->value() * (-0.5f + i / (numSideElements->value() - 1.0f)) / fbo->width;

        for (int j = 0; j < numSideElements->value(); j++)
        {
            float y = size->value() * (-0.5f + j / (numSideElements->value() - 1.0f)) / fbo->height;

            spatialKernel[i * numSideElements->value() + j] = exp(-0.5f * fbo->width * fbo->height * (x * x + y * y) / (spatialSigma->value() * spatialSigma->value()));
        }
    }

    fbo->makeCurrent();
    fbo->program->bind();
    fbo->program->setUniformValueArray("spatialKernel", spatialKernel, numSideElements->value() * numSideElements->value(), 1);
    fbo->program->release();
    fbo->doneCurrent();

    delete [] spatialKernel;
}



void BilateralFilter::setIntParameter(int index, int value)
{
    if (index == 0)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("numElements", value * value);
        fbo->program->release();
        fbo->doneCurrent();

        computeOffsets();
        computeSpatialKernel();
    }
}



void BilateralFilter::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        computeOffsets();
        computeSpatialKernel();
    }
    else if (index == 1)
    {
        computeSpatialKernel();
    }
    else if (index == 2)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("rangeSigma", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 3)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Brightness

QString Brightness::name = "Brightness";

Brightness::Brightness(bool on, QOpenGLContext* mainContext, float theBrightness, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/brightness.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(theBrightness, -1.0f, 1.0f, min, max);
    brightness = new FloatParameter("Brightness", 0, this, theBrightness, min, max, -10.0f, 10.0f);

    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 1, this, theOpacity, min, max, 0.0f, 1.0f);
}



Brightness::Brightness(const Brightness& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/brightness.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    brightness = new FloatParameter(*operation.brightness);
    brightness->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



Brightness::~Brightness()
{
    delete brightness;
    delete opacity;
}



void Brightness::setParameters()
{
    setFloatParameter(0, brightness->value());
    setFloatParameter(1, opacity->value());
}



void Brightness::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("brightness", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 1)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Color mix

QString ColorMix::name = "Color mix";

ColorMix::ColorMix(bool on, QOpenGLContext* mainContext, std::vector<float> theMatrix, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/colormix.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    rgbMatrix = new MatrixParameter("RGB Matrix", 0, this, theMatrix, -10.0f, 10.0f, -1000.0f, 1000.0f);

    float min, max;
    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 0, this, theOpacity, min, max, 0.0f, 1.0f);
}



ColorMix::ColorMix(const ColorMix& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/colormix.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    rgbMatrix = new MatrixParameter(*operation.rgbMatrix);
    rgbMatrix->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



ColorMix::~ColorMix()
{
    delete rgbMatrix;
    delete opacity;
}


void ColorMix::setParameters()
{
    setMatrixParameter(rgbMatrix->numbers);
    setFloatParameter(0, opacity->value());
}



void ColorMix::setMatrixParameter(std::vector<Number<float>*> numbers)
{
    QMatrix3x3 mixMatrix;

    int row = 0, col = 0;
    for (Number<float>* number : numbers)
    {
        mixMatrix(row, col++) = number->value;
        if (col >= 3) { row++; col = 0; }
    }

    fbo->makeCurrent();
    fbo->program->bind();
    fbo->program->setUniformValue("rgbMatrix", mixMatrix);
    fbo->program->release();
    fbo->doneCurrent();
}



void ColorMix::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Color quantization

QString ColorQuantization::name = "Color quantization";

ColorQuantization::ColorQuantization(bool on, QOpenGLContext* mainContext, int theRedLevels, int theGreenLevels, int theBlueLevels, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/color-quantization.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    redLevels = new IntParameter("Red levels", 0, this, theRedLevels, 2, 255, 2, 255, false);
    greenLevels = new IntParameter("Green levels", 1, this, theGreenLevels, 2, 255, 2, 255, false);
    blueLevels = new IntParameter("Blue levels", 2, this, theBlueLevels, 2, 255, 2, 255, false);

    float min, max;
    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 0, this, theOpacity, min, max, 0.0f, 1.0f);
}



ColorQuantization::ColorQuantization(const ColorQuantization& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/color-quantization.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    redLevels = new IntParameter(*operation.redLevels);
    redLevels->setOperation(this);

    greenLevels = new IntParameter(*operation.greenLevels);
    greenLevels->setOperation(this);

    blueLevels = new IntParameter(*operation.blueLevels);
    blueLevels->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



ColorQuantization::~ColorQuantization()
{
    delete redLevels;
    delete greenLevels;
    delete blueLevels;
    delete opacity;
}



void ColorQuantization::setParameters()
{
    setIntParameter(0, redLevels->value());
    setIntParameter(1, greenLevels->value());
    setIntParameter(2, blueLevels->value());

    setFloatParameter(0, opacity->value());
}



void ColorQuantization::setIntParameter(int index, int value)
{
    if (index == 0)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("redLevels", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 1)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("greenLevels", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 2)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("blueLevels", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



void ColorQuantization::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Contrast

QString Contrast::name = "Contrast";

Contrast::Contrast(bool on, QOpenGLContext* mainContext, float theContrast, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/contrast.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(theContrast, -1.0f, 2.0f, min, max);
    contrast = new FloatParameter("Contrast", 0, this, theContrast, min, max, -10.0f, 10.0f);

    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 1, this, theOpacity, min, max, 0.0f, 1.0f);
}



Contrast::Contrast(const Contrast& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/contrast.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    contrast = new FloatParameter(*operation.contrast);
    contrast->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



Contrast::~Contrast()
{
    delete contrast;
    delete opacity;
}



void Contrast::setParameters()
{
    setFloatParameter(0, contrast->value());
    setFloatParameter(1, opacity->value());
}



void Contrast::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("contrast", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 1)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Convolution

QString Convolution::name = "Convolution";

Convolution::Convolution(bool on, QOpenGLContext* mainContext, std::vector<float> theKernel, float theFactor, float theSize, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/convolution.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    kernel = new KernelParameter("Kernel", 0, this, theKernel, -10.0f, 10.0f, -1000.0f, 1000.0f, true);

    float min, max;
    adjustMinMax(theFactor, -10.0f, 10.0f, min, max);
    factor = new FloatParameter("Kernel factor", 0, this, theFactor, min, max, -1.0e3f, 1.0e3f);

    adjustMinMax(theSize, 1.0f, 20.0f, min, max);
    size = new FloatParameter("Size (px)", 1, this, theSize, min, max, 1.0f, 1000.0f);

    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 2, this, theOpacity, min, max, 0.0f, 1.0f);
}



Convolution::Convolution(const Convolution &operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/convolution.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    kernel = new KernelParameter(*operation.kernel);
    kernel->setOperation(this);

    factor = new FloatParameter(*operation.factor);
    factor->setOperation(this);

    size = new FloatParameter(*operation.size);
    size->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



Convolution::~Convolution()
{
    delete kernel;
    delete factor;
    delete size;
    delete opacity;
}



void Convolution::setParameters()
{
    setFloatParameter(0, factor->value());
    setFloatParameter(1, size->value());
    setFloatParameter(2, opacity->value());
}



void Convolution::setKernelParameter(std::vector<Number<float>*> numbers)
{
    std::vector<float> values;

    for (Number<float>* number : numbers)
        values.push_back(number->value * factor->value());

    fbo->makeCurrent();
    fbo->program->bind();
    fbo->program->setUniformValueArray("kernel", &values[0], 9, 1);
    fbo->program->release();
    fbo->doneCurrent();
}



void Convolution::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        setKernelParameter(kernel->numbers);
    }
    else if (index == 1)
    {
        float x = (value - 0.5) / fbo->width;
        float y = (value - 0.5) / fbo->height;

        QVector2D offset[] = {
            QVector2D(-x, y),
            QVector2D(0.0f, y),
            QVector2D(x, y),
            QVector2D(-x, 0.0f),
            QVector2D(0.0f, 0.0f),
            QVector2D(x, 0.0f),
            QVector2D(-x, -y),
            QVector2D(0.0f, -y),
            QVector2D(x, -y)
        };

        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValueArray("offset", offset, 9);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 2)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Dilation

QString Dilation::name = "Dilation";

Dilation::Dilation(bool on, QOpenGLContext* mainContext, float theSize, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/dilation.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(theSize, 1.0f, 100.0f, min, max);
    size = new FloatParameter("Size", 0, this, theSize, min, max, 1.0f, 1000.0f);

    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 1, this, theOpacity, min, max, 0.0f, 1.0f);
}



Dilation::Dilation(const Dilation& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/dilation.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    size = new FloatParameter(*operation.size);
    size->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



Dilation::~Dilation()
{
    delete size;
    delete opacity;
}



void Dilation::setParameters()
{
    setFloatParameter(0, size->value());
    setFloatParameter(1, opacity->value());
}



void Dilation::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        float x = value / fbo->width;
        float y = value / fbo->height;

        QVector2D offset[] = {
            QVector2D(-x, y),
            QVector2D(0.0f, y),
            QVector2D(x, y),
            QVector2D(-x, 0.0f),
            QVector2D(0.0f, 0.0f),
            QVector2D(x, 0.0f),
            QVector2D(-x, -y),
            QVector2D(0.0f, -y),
            QVector2D(x, -y)
        };

        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValueArray("offset", offset, 8);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 1)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Erosion

QString Erosion::name = "Erosion";

Erosion::Erosion(bool on, QOpenGLContext* mainContext, float theSize, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/erosion.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(theSize, 1.0f, 100.0f, min, max);
    size = new FloatParameter("Size", 0, this, theSize, min, max, 1.0f, 1000.0f);

    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 1, this, theOpacity, min, max, 0.0f, 1.0f);
}



Erosion::Erosion(const Erosion& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/erosion.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    size = new FloatParameter(*operation.size);
    size->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



Erosion::~Erosion()
{
    delete size;
    delete opacity;
}



void Erosion::setParameters()
{
    setFloatParameter(0, size->value());
    setFloatParameter(1, opacity->value());
}



void Erosion::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        float x = value / fbo->width;
        float y = value / fbo->height;

        QVector2D offset[] = {
            QVector2D(-x, y),
            QVector2D(0.0f, y),
            QVector2D(x, y),
            QVector2D(-x, 0.0f),
            QVector2D(0.0f, 0.0f),
            QVector2D(x, 0.0f),
            QVector2D(-x, -y),
            QVector2D(0.0f, -y),
            QVector2D(x, -y)
        };

        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValueArray("offset", offset, 8);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 1)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Gamma correction

QString GammaCorrection::name = "Gamma correction";

GammaCorrection::GammaCorrection(bool on, QOpenGLContext* mainContext, float theGammaRed, float theGammaGreen, float theGammaBlue, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/gamma.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(theGammaRed, 0.0f, 100.0f, min, max);
    gammaRed = new FloatParameter("Red", 0, this, theGammaRed, min, max, 0.0f, 1000.0f);
    
    adjustMinMax(theGammaGreen, 0.0f, 100.0f, min, max);
    gammaGreen = new FloatParameter("Green", 1, this, theGammaGreen, min, max, 0.0f, 1000.0f);
    
    adjustMinMax(theGammaBlue, 0.0f, 100.0f, min, max);
    gammaBlue = new FloatParameter("Blue", 2, this, theGammaBlue, min, max, 0.0f, 1000.0f);
    
    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 3, this, theOpacity, min, max, 0.0f, 1.0f);
}

GammaCorrection::GammaCorrection(const GammaCorrection& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/gamma.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    gammaRed = new FloatParameter(*operation.gammaRed);
    gammaRed->setOperation(this);

    gammaGreen = new FloatParameter(*operation.gammaGreen);
    gammaGreen->setOperation(this);

    gammaBlue = new FloatParameter(*operation.gammaBlue);
    gammaBlue->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



GammaCorrection::~GammaCorrection()
{
    delete gammaRed;
    delete gammaGreen;
    delete gammaBlue;
    delete opacity;
}



void GammaCorrection::setParameters()
{
    setFloatParameter(0, gammaRed->value());
    setFloatParameter(1, gammaGreen->value());
    setFloatParameter(2, gammaBlue->value());
    setFloatParameter(3, opacity->value());
}



void GammaCorrection::setFloatParameter(int index, float value)
{
    if (index == 0 || index == 1 || index == 2)
    {
        QVector3D gamma = { gammaRed->value(), gammaGreen->value(), gammaBlue->value() };

        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("gamma", gamma);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 3)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Hue shift

QString HueShift::name = "Hue shift";

HueShift::HueShift(bool on, QOpenGLContext* mainContext, float theShift, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/hueshift.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(theShift, -1.0f, 1.0f, min, max);
    shift = new FloatParameter("Shift", 0, this, theShift, min, max, -1.0f, 1.0f);

    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 1, this, theOpacity, min, max, 0.0f, 1.0f);
}



HueShift::HueShift(const HueShift& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/hueshift.frag",context);
    fbo->setInputTextureID(*blender->getTextureID());

    shift = new FloatParameter(*operation.shift);
    shift->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



HueShift::~HueShift()
{
    delete shift;
    delete opacity;
}



void HueShift::setParameters()
{
    setFloatParameter(0, shift->value());
    setFloatParameter(1, opacity->value());
}



void HueShift::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("shift", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 1)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Mask

QString Identity::name = "Identity";

Identity::Identity(bool on, QOpenGLContext* mainContext) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    noParameters = true;
}



Identity::Identity(const Identity& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    noParameters = true;
}



Identity::~Identity(){}



// Logistic

QString Logistic::name = "Logistic";

Logistic::Logistic(bool on, QOpenGLContext* mainContext, float R, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/logistic.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(R, 0.0f, 4.0f, min, max);
    r = new FloatParameter("Nonlinear parameter", 0, this, R, min, max, 0.0f, 4.0f);

    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 1, this, theOpacity, min, max, 0.0f, 1.0f);
}



Logistic::Logistic(const Logistic& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/logistic.frag",context);
    fbo->setInputTextureID(*blender->getTextureID());

    r = new FloatParameter(*operation.r);
    r->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



Logistic::~Logistic()
{
    delete r;
    delete opacity;
}



void Logistic::setParameters()
{
    setFloatParameter(0, r->value());
    setFloatParameter(1, opacity->value());
}



void Logistic::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("r", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 1)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Mask

QString Mask::name = "Mask";

Mask::Mask(bool on, QOpenGLContext* mainContext) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/mask.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    connect(fbo, &FBO::sizeChanged, this, &Mask::setScale);

    setScale();

    noParameters = true;
}



Mask::Mask(const Mask& operation) : QObject(nullptr), ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/mask.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    connect(fbo, &FBO::sizeChanged, this, &Mask::setScale);

    setScale();

    noParameters = true;
}



Mask::~Mask(){}



void Mask::setScale()
{
    float scaleX = fbo->height >= fbo->width ? 1.0 : static_cast<float>(fbo->width) / static_cast<float>(fbo->height);
    float scaleY = fbo->height >= fbo->width ? static_cast<float>(fbo->height) / static_cast<float>(fbo->width) : 1.0;

    fbo->makeCurrent();
    fbo->program->bind();
    fbo->program->setUniformValue("scaleX", scaleX);
    fbo->program->setUniformValue("scaleY", scaleY);
    fbo->program->release();
    fbo->doneCurrent();
}



// Median

QString Median::name = "Median";

Median::Median(bool on, QOpenGLContext* mainContext, float theSize, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/median.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(theSize, 1.0f, 100.0f, min, max);
    size = new FloatParameter("Size", 0, this, theSize, min, max, 1.0f, 1000.0f);

    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 1, this, theOpacity, min, max, 0.0f, 1.0f);
}



Median::Median(const Median& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/median.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    size = new FloatParameter(*operation.size);
    size->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



Median::~Median()
{
    delete size;
    delete opacity;
}



void Median::setParameters()
{
    setFloatParameter(0, size->value());
    setFloatParameter(1, opacity->value());
}



void Median::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        float x = value / fbo->width;
        float y = value / fbo->height;

        QVector2D offset[] = {
            QVector2D(-x, y),
            QVector2D(0.0f, y),
            QVector2D(x, y),
            QVector2D(-x, 0.0f),
            QVector2D(0.0f, 0.0f),
            QVector2D(x, 0.0f),
            QVector2D(-x, -y),
            QVector2D(0.0f, -y),
            QVector2D(x, -y)
        };

        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValueArray("offset", offset, 8);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 1)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Memory

QString Memory::name = "Memory";

Memory::Memory(bool on, QOpenGLContext* mainContext, int theFrames, float theBlendFactor, float theDecayFactor) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    frames = new IntParameter("Frames", 0, this, theFrames, 1, 10, 1, 10, false);

    float min, max;
    adjustMinMax(theBlendFactor, 0.0f, 1.0f, min, max);
    blendFactor = new FloatParameter("Blend factor", 0, this, theBlendFactor, min, max, 0.0f, 1.0f);

    adjustMinMax(theDecayFactor, 0.0f, 1.0f, min, max);
    decayFactor = new FloatParameter("Decay factor", 1, this, theDecayFactor, min, max, 0.0f, 1.0f);

    for (int i = 0; i < theFrames; i++)
        fbos.push_back(new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", mainContext));

    fbos[0]->setInputTextureID(*fbo->getTextureBlit());

    for (int i = 1; i < theFrames; i++)
        fbos[i]->setInputTextureID(*fbos[i - 1]->getTextureID());

    blenderOut = new Blender(":/shaders/screen.vert", ":/shaders/blend.frag", mainContext);

    setBlenderOutInputData();

    fboOut = new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", mainContext);

    enable(on);
}



Memory::Memory(const Memory& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    frames = new IntParameter(*operation.frames);
    frames->setOperation(this);

    blendFactor = new FloatParameter(*operation.blendFactor);
    blendFactor->setOperation(this);

    decayFactor = new FloatParameter(*operation.decayFactor);
    decayFactor->setOperation(this);

    for (int i = 0; i < operation.fbos.size(); i++)
        fbos.push_back(new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", context));

    fbos[0]->setInputTextureID(*fbo->getTextureBlit());

    for (int i = 1; i < operation.fbos.size(); i++)
        fbos[i]->setInputTextureID(*fbos[i - 1]->getTextureID());

    blenderOut = new Blender(":/shaders/screen.vert", ":/shaders/blend.frag", context);

    fboOut = new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", context);

    enable(operation.enabled);
}



Memory::~Memory()
{
    delete blendFactor;
    delete decayFactor;
    foreach (FBO* oneFBO, fbos)
        delete oneFBO;
    fbos.clear();
    delete blenderOut;
    delete fboOut;
}



void Memory::setBlenderOutInputData()
{
    QVector<InputData*> inputs;

    inputs.push_back(new InputData(InputType::Normal, fbo->getTextureID(), 1.0f));

    float factor = blendFactor->value();

    foreach (FBO* oneFBO, fbos)
    {
        inputs.push_back(new InputData(InputType::Normal, oneFBO->getTextureID(), factor));
        factor *= decayFactor->value();
    }
    blenderOut->setInputData(inputs);
}



void Memory::enable(bool on)
{
    if (on)
        fboOut->setInputTextureID(*blenderOut->getTextureID());
    else
        fboOut->setInputTextureID(*fbo->getTextureID());

    enabled = on;
}



void Memory::resize()
{
    blender->resize();
    fbo->resize();
    foreach (FBO* oneFBO, fbos)
        oneFBO->resize();
    blenderOut->resize();
    fboOut->resize();

    setBlenderOutInputData();
}



void Memory::applyOperation()
{
    if (blenderEnabled)
        blender->blend();

    if (enabled)
    {
        fbo->draw();

        for (int i = fbos.size() - 1; i >= 0; i--)
            fbos[i]->draw();

        fbo->blit();

        blenderOut->blend();

        fboOut->draw();
    }
    else
    {
        fbo->identity();
        fboOut->identity();
    }
}



void Memory::blit()
{
    fbo->blit();
    fboOut->blit();
}



void Memory::clear()
{
    fbo->clear();
    foreach (FBO* oneFBO, fbos)
        oneFBO->clear();
    fboOut->clear();
}



void Memory::setIntParameter(int index, int value)
{
    if (index == 0)
    {
        if (value < fbos.size())
        {
            int imax = fbos.size();
            for (int i = value; i < imax; i++)
                if (!fbos.empty())
                    fbos.removeLast();

            setBlenderOutInputData();
        }
        else if (value > fbos.size())
        {
            for (int i = fbos.size(); i < value; i++)
            {
                fbos.push_back(new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", context));
                fbos[i]->setInputTextureID(*fbos[i - 1]->getTextureID());
            }

            setBlenderOutInputData();
        }
    }
}



void Memory::setFloatParameter(int index, float)
{
    if (index == 0 || index == 1)
    {
        setBlenderOutInputData();
    }
}



// Morphological gradient

QString MorphologicalGradient::name = "Morphological gradient";

MorphologicalGradient::MorphologicalGradient(bool on, QOpenGLContext* mainContext, float theDilationSize, float theErosionSize, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/morphogradient.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(theDilationSize, 1.0f, 100.0f, min, max);
    dilationSize = new FloatParameter("Dilation size", 0, this, theDilationSize, min, max, 1.0f, 1000.0f);

    adjustMinMax(theDilationSize, 1.0f, 100.0f, min, max);
    erosionSize = new FloatParameter("Erosion size", 1, this, theErosionSize, min, max, 1.0f, 1000.0f);

    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 2, this, theOpacity, min, max, 0.0f, 1.0f);
}



MorphologicalGradient::MorphologicalGradient(const MorphologicalGradient& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/morphogradient.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    dilationSize = new FloatParameter(*operation.dilationSize);
    dilationSize->setOperation(this);

    erosionSize = new FloatParameter(*operation.erosionSize);
    erosionSize->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



MorphologicalGradient::~MorphologicalGradient()
{
    delete dilationSize;
    delete erosionSize;
    delete opacity;
}



void MorphologicalGradient::setParameters()
{
    setFloatParameter(0, dilationSize->value());
    setFloatParameter(1, erosionSize->value());
    setFloatParameter(2, opacity->value());
}



void MorphologicalGradient::setFloatParameter(int index, float value)
{
    float x = value / fbo->width;
    float y = value / fbo->height;

    QVector2D offset[] = {
        QVector2D(-x, y),
        QVector2D(0.0f, y),
        QVector2D(x, y),
        QVector2D(-x, 0.0f),
        QVector2D(0.0f, 0.0f),
        QVector2D(x, 0.0f),
        QVector2D(-x, -y),
        QVector2D(0.0f, -y),
        QVector2D(x, -y)
    };

    if (index == 0)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValueArray("dilationOffset", offset, 8);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 1)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValueArray("erosionOffset", offset, 8);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 2)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Pixelation

QString Pixelation::name = "Pixelation";

Pixelation::Pixelation(bool on, QOpenGLContext* mainContext, float theSize, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/pixelation.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(theSize, 0.0f, 20.0f, min, max);
    size = new FloatParameter("Size", 0, this, theSize, min, max, 0.0f, 20.0f);

    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 1, this, theOpacity, min, max, 0.0f, 1.0f);

    connect(fbo, &FBO::sizeChanged, this, &Pixelation::setWidthAndHeight);

    setWidthAndHeight();
}



Pixelation::Pixelation(const Pixelation& operation) : QObject(nullptr), ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/pixelation.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    size = new FloatParameter(*operation.size);
    size->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);

    connect(fbo, &FBO::sizeChanged, this, &Pixelation::setWidthAndHeight);

    setWidthAndHeight();
}



Pixelation::~Pixelation()
{
    delete size;
    delete opacity;
}



void Pixelation::setParameters()
{
    setFloatParameter(0, size->value());
    setFloatParameter(1, opacity->value());
}



void Pixelation::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("size", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 1)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



void Pixelation::setWidthAndHeight()
{
    fbo->makeCurrent();
    fbo->program->bind();
    fbo->program->setUniformValue("width", static_cast<float>(fbo->width));
    fbo->program->setUniformValue("height", static_cast<float>(fbo->height));
    fbo->program->release();
    fbo->doneCurrent();
}



// Polar convolution

QString PolarConvolution::name = "Polar convolution";

PolarConvolution::PolarConvolution(bool on, QOpenGLContext* mainContext, std::vector<PolarKernel*> thePolarKernels, float theCenterElement, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/polar-convolution.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    polarKernelParameter = new PolarKernelParameter("Polar kernels", 0, this, thePolarKernels, theCenterElement);

    float min, max;
    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 0, this, theOpacity, min, max, 0.0f, 1.0f);
}



PolarConvolution::PolarConvolution(const PolarConvolution& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/polar-convolution.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    polarKernelParameter = new PolarKernelParameter(*operation.polarKernelParameter);
    polarKernelParameter->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



PolarConvolution::~PolarConvolution()
{
    delete polarKernelParameter;
    delete opacity;
}


void PolarConvolution::setParameters()
{
    setPolarKernelParameter();
    setFloatParameter(0, opacity->value());
}



void PolarConvolution::setPolarKernelParameter()
{
    // Compute offsets and kernel elements

    int numElements = 0;

    for (auto& kernel : polarKernelParameter->polarKernels)
        numElements += kernel->numElements;

    QVector2D* offsets = new QVector2D[numElements];
    GLfloat* kernels = new GLfloat[numElements];

    int n = 0;

    for (auto& kernel : polarKernelParameter->polarKernels)
    {
        for (int i = 0; i < kernel->numElements; i++)
        {
            // Index

            float index = static_cast<float>(i) / kernel->numElements;

            // Angle: degrees to radians

            float angle = (kernel->initialAngle / 180.0f + 2.0f * index) * 3.14159265359f;

            // Offsets
                        
            offsets[n] = QVector2D(kernel->radius * cosf(angle), kernel->radius * sinf(angle));            
            
            // Kernel elements

            kernels[n++] = kernel->minimum + (kernel->maximum - kernel->minimum) * (1.0f + sinf((kernel->phase / 180.0 + kernel->frequency * 2.0f * index) * 3.14159265359f)) * 0.5f;
        }
    }

    fbo->makeCurrent();
    fbo->program->bind();
    fbo->program->setUniformValue("numElements", numElements);
    fbo->program->setUniformValueArray("offset", offsets, numElements);
    fbo->program->setUniformValueArray("kernel", kernels, numElements, 1);
    fbo->program->setUniformValue("centerElement", polarKernelParameter->centerElement);
    fbo->program->release();
    fbo->doneCurrent();

    delete [] offsets;
    delete [] kernels;
}



void PolarConvolution::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Power

QString Power::name = "Power";

Power::Power(bool on, QOpenGLContext* mainContext, float theExponent, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/power.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(theExponent, 0.00001f, 10.0f, min, max);
    exponent = new FloatParameter("Value", 0, this, theExponent, min, max, 0.00001f, 100.0f);

    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 1, this, theOpacity, min, max, 0.0f, 1.0f);
}



Power::Power(const Power& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/power.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    exponent = new FloatParameter(*operation.exponent);
    exponent->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



Power::~Power()
{
    delete exponent;
    delete opacity;
}



void Power::setParameters()
{
    setFloatParameter(0, exponent->value());
    setFloatParameter(1, opacity->value());
}



void Power::setFloatParameter(int index, float theValue)
{
    if (index == 0)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("exponent", theValue);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 1)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", theValue);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Rotation

QString Rotation::name = "Rotation";

Rotation::Rotation(bool on, QOpenGLContext* mainContext, float theAngle, GLenum theMinMagFilter) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(theAngle, -360.0f, 360.0f, min, max);
    angle = new FloatParameter("Angle", 0, this, theAngle, min, max, -1.0e6, 1.0e6);

    std::vector<QString> valueNames = { "Nearest neighbor", "Linear" };
    std::vector<GLenum> values = { GL_NEAREST, GL_LINEAR };
    minMagFilter = new OptionsParameter<GLenum>("Interpolation", 0, this, valueNames, values, theMinMagFilter);
}



Rotation::Rotation(const Rotation& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    angle = new FloatParameter(*operation.angle);
    angle->setOperation(this);

    minMagFilter = new OptionsParameter<GLenum>(*operation.minMagFilter);
    minMagFilter->setOperation(this);
}



Rotation::~Rotation()
{
    delete angle;
    delete minMagFilter;
}



void Rotation::setParameters()
{
    setFloatParameter(0, angle->value());
    setOptionsParameter(0, minMagFilter->value());
}



void Rotation::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        // Set rotation matrix

        fbo->transformationMatrix.setToIdentity();
        fbo->transformationMatrix.rotate(value, 0.0f, 0.0f, 1.0f);
        fbo->adjustTransform();

        // Set shader uniform

        /*fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("transform", transform);
        fbo->program->release();
        fbo->doneCurrent();*/
    }
}



void Rotation::setOptionsParameter(int index, GLenum value)
{
    if (index == 0)
    {
        // Set minifying and magnifying functions

        fbo->setMinMagFilter(value);
    }
}



// Saturation

QString Saturation::name = "Saturation";

Saturation::Saturation(bool on, QOpenGLContext* mainContext, float theSaturation, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/saturation.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(theSaturation, 0.0f, 1.0f, min, max);
    saturation = new FloatParameter("Saturation", 0, this, theSaturation, min, max, 0.0f, 1.0f);

    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 1, this, theOpacity, min, max, 0.0f, 1.0f);
}



Saturation::Saturation(const Saturation& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/saturation.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    saturation = new FloatParameter(*operation.saturation);
    saturation->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



Saturation::~Saturation()
{
    delete saturation;
    delete opacity;
}



void Saturation::setParameters()
{
    setFloatParameter(0, saturation->value());
    setFloatParameter(1, opacity->value());
}


void Saturation::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("saturation", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 1)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", value);
        fbo->program->release();
        fbo->doneCurrent();
    }
}



// Scale

QString Scale::name = "Scale";

Scale::Scale(bool on, QOpenGLContext* mainContext, float theScaleFactor, GLenum theMinMagFilter) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(theScaleFactor, 0.0f, 2.0f, min, max);
    scaleFactor = new FloatParameter("Scale factor", 0, this, theScaleFactor, min, max, -1.0e6, 1.0e6);

    std::vector<QString> valueNames = { "Nearest neighbor", "Linear" };
    std::vector<GLenum> values = { GL_NEAREST, GL_LINEAR };
    minMagFilter = new OptionsParameter<GLenum>("Interpolation", 0, this, valueNames, values, theMinMagFilter);
}



Scale::Scale(const Scale& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    scaleFactor = new FloatParameter(*operation.scaleFactor);
    scaleFactor->setOperation(this);

    minMagFilter = new OptionsParameter<GLenum>(*operation.minMagFilter);
    minMagFilter->setOperation(this);
}



Scale::~Scale()
{
    delete scaleFactor;
    delete minMagFilter;
}



void Scale::setParameters()
{
    setFloatParameter(0, scaleFactor->value());
    setOptionsParameter(0, minMagFilter->value());
}



void Scale::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        // Set scale matrix

        /*QMatrix4x4 transform;
        transform.setToIdentity();
        transform.scale(value, value);*/

        // Set shader uniform

        /*fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("transform", transform);
        fbo->program->release();
        fbo->doneCurrent();*/

        fbo->transformationMatrix.setToIdentity();
        fbo->transformationMatrix.scale(value, value);
        fbo->adjustTransform();
    }
}



void Scale::setOptionsParameter(int index, GLenum value)
{
    if (index == 0)
    {
        // Set minifying and magnifying functions

        fbo->setMinMagFilter(value);
    }
}



// Value

QString Value::name = "Value";

Value::Value(bool on, QOpenGLContext* mainContext, float theValue, float theOpacity) : ImageOperation(on, mainContext)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/value.frag", mainContext);
    fbo->setInputTextureID(*blender->getTextureID());

    float min, max;
    adjustMinMax(theValue, 0.0f, 1.0f, min, max);
    value = new FloatParameter("Value", 0, this, theValue, min, max, 0.0f, 1.0f);

    adjustMinMax(theOpacity, 0.0f, 1.0f, min, max);
    opacity = new FloatParameter("Opacity", 1, this, theOpacity, min, max, 0.0f, 1.0f);
}



Value::Value(const Value& operation) : ImageOperation(operation)
{
    fbo = new FBO(":/shaders/screen.vert", ":/shaders/value.frag", context);
    fbo->setInputTextureID(*blender->getTextureID());

    value = new FloatParameter(*operation.value);
    value->setOperation(this);

    opacity = new FloatParameter(*operation.opacity);
    opacity->setOperation(this);
}



Value::~Value()
{
    delete value;
    delete opacity;
}



void Value::setParameters()
{
    setFloatParameter(0, value->value());
    setFloatParameter(1, opacity->value());
}


void Value::setFloatParameter(int index, float theValue)
{
    if (index == 0)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("value", theValue);
        fbo->program->release();
        fbo->doneCurrent();
    }
    else if (index == 1)
    {
        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("opacity", theValue);
        fbo->program->release();
        fbo->doneCurrent();
    }
}
