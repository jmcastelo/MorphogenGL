/*
*  Copyright 2020 Jose Maria Castelo Ares
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

ImageOperation::ImageOperation(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext) : enabled { on }
{
    fbo = new FBO(vertexShader, fragmentShader, mainContext);
}

ImageOperation::~ImageOperation()
{
    delete fbo;
}

void ImageOperation::applyOperation(GLuint inTextureID)
{
    fbo->setInputTextureID(inTextureID);
    fbo->draw();
}

void ImageOperation::adjustMinMax(float value, float minValue, float maxValue, float& min, float& max)
{
    if (value < minValue) min = value;
    else min = minValue;

    if (value > maxValue) max = value;
    else max = maxValue;
}

// Brightness

QString Brightness::name = "Brightness";

Brightness::Brightness(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theBrightness) : ImageOperation(on, vertexShader, fragmentShader, mainContext)
{
    float minBrightness, maxBrightness;
    adjustMinMax(theBrightness, -1.0f, 1.0f, minBrightness, maxBrightness);
    brightness = new FloatParameter("Brightness", 0, this, theBrightness, minBrightness, maxBrightness, -10.0f, 10.0f);
    setFloatParameter(0, theBrightness);
}

Brightness::~Brightness()
{
    delete brightness;
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
}

// Color mix

QString ColorMix::name = "Color mix";

ColorMix::ColorMix(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, std::vector<float> theMatrix) : ImageOperation(on, vertexShader, fragmentShader, mainContext)
{
    rgbMatrix = new MatrixParameter("RGB Matrix", this, theMatrix, -1000.0f, 1000.0f);
    setMatrixParameter(&theMatrix[0]);
}

ColorMix::~ColorMix()
{
    delete rgbMatrix;
}

void ColorMix::setMatrixParameter(GLfloat* values)
{
    QMatrix3x3 mixMatrix(values);

    fbo->makeCurrent();
    fbo->program->bind();
    fbo->program->setUniformValue("rgbMatrix", mixMatrix);
    fbo->program->release();
    fbo->doneCurrent();
}

// Contrast

QString Contrast::name = "Contrast";

Contrast::Contrast(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theContrast) : ImageOperation(on, vertexShader, fragmentShader, mainContext)
{
    float min, max;
    adjustMinMax(theContrast, -1.0f, 2.0f, min, max);
    contrast = new FloatParameter("Contrast", 0, this, theContrast, min, max, -10.0f, 10.0f);
    setFloatParameter(0, theContrast);
}

Contrast::~Contrast()
{
    delete contrast;
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
}

// Convolution

QString Convolution::name = "Convolution";

Convolution::Convolution(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, std::vector<float> theKernel, float theSize) : ImageOperation(on, vertexShader, fragmentShader, mainContext)
{
    kernel = new KernelParameter("Kernel", this, theKernel, -1000.0f, 1000.0f, true);
    setKernelParameter(&theKernel[0]);

    float min, max;
    adjustMinMax(theSize, 0.0f, 1.0f, min, max);
    size = new FloatParameter("Size", 0, this, theSize, min, max, 0.0f, 1.0f);
    setFloatParameter(0, theSize);
}

Convolution::~Convolution()
{
    delete kernel;
    delete size;
}

void Convolution::setKernelParameter(GLfloat* values)
{
    fbo->makeCurrent();
    fbo->program->bind();
    fbo->program->setUniformValueArray("kernel", values, 9, 1);
    fbo->program->release();
    fbo->doneCurrent();
}

void Convolution::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        QVector2D offset[] = {
            QVector2D(-value, value),
            QVector2D(0.0f, value),
            QVector2D(value, value),
            QVector2D(-value, 0.0f),
            QVector2D(0.0f, 0.0f),
            QVector2D(value, 0.0f),
            QVector2D(-value, -value),
            QVector2D(0.0f, -value),
            QVector2D(value, -value)
        };

        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValueArray("offset", offset, 9);
        fbo->program->release();
        fbo->doneCurrent();
    }
}

// Dilation

QString Dilation::name = "Dilation";

Dilation::Dilation(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theSize) : ImageOperation(on, vertexShader, fragmentShader, mainContext)
{
    float min, max;
    adjustMinMax(theSize, 0.0f, 1.0f, min, max);
    size = new FloatParameter("Size", 0, this, theSize, min, max, 0.0f, 1.0f);
    setFloatParameter(0, theSize);
}

Dilation::~Dilation()
{
    delete size;
}

void Dilation::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        QVector2D offset[] = {
            QVector2D(-value, value),
            QVector2D(0.0f, value),
            QVector2D(value, value),
            QVector2D(-value, 0.0f),
            QVector2D(value, 0.0f),
            QVector2D(-value, -value),
            QVector2D(0.0f, -value),
            QVector2D(value, -value)
        };

        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValueArray("offset", offset, 8);
        fbo->program->release();
        fbo->doneCurrent();
    }
}

// Erosion

QString Erosion::name = "Erosion";

Erosion::Erosion(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theSize) : ImageOperation(on, vertexShader, fragmentShader, mainContext)
{
    float min, max;
    adjustMinMax(theSize, 0.0f, 1.0f, min, max);
    size = new FloatParameter("Size", 0, this, theSize, min, max, 0.0f, 1.0f);
    setFloatParameter(0, theSize);
}

Erosion::~Erosion()
{
    delete size;
}

void Erosion::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        QVector2D offset[] = {
            QVector2D(-value, value),
            QVector2D(0.0f, value),
            QVector2D(value, value),
            QVector2D(-value, 0.0f),
            QVector2D(value, 0.0f),
            QVector2D(-value, -value),
            QVector2D(0.0f, -value),
            QVector2D(value, -value)
        };

        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValueArray("offset", offset, 8);
        fbo->program->release();
        fbo->doneCurrent();
    }
}


// Gamma correction

QString GammaCorrection::name = "Gamma correction";

GammaCorrection::GammaCorrection(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theGammaRed, float theGammaGreen, float theGammaBlue) : ImageOperation(on, vertexShader, fragmentShader, mainContext)
{
    float min, max;
    adjustMinMax(theGammaRed, 0.0f, 100.0f, min, max);
    gammaRed = new FloatParameter("Red", 0, this, theGammaRed, min, max, 0.0f, 1000.0f);
    
    adjustMinMax(theGammaGreen, 0.0f, 100.0f, min, max);
    gammaGreen = new FloatParameter("Green", 1, this, theGammaGreen, min, max, 0.0f, 1000.0f);
    
    adjustMinMax(theGammaBlue, 0.0f, 100.0f, min, max);
    gammaBlue = new FloatParameter("Blue", 2, this, theGammaBlue, min, max, 0.0f, 1000.0f);
    
    setFloatParameter(0, theGammaRed);
    setFloatParameter(1, theGammaGreen);
    setFloatParameter(2, theGammaBlue);
}

GammaCorrection::~GammaCorrection()
{
    delete gammaRed;
    delete gammaGreen;
    delete gammaBlue;
}

void GammaCorrection::setFloatParameter(int index, float)
{
    if (index == 0 || index == 1 || index == 2)
    {
        QVector3D gamma = { gammaRed->value, gammaGreen->value, gammaBlue->value };

        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("gamma", gamma);
        fbo->program->release();
        fbo->doneCurrent();
    }
}

// Morphological gradient

QString MorphologicalGradient::name = "Morphological gradient";

MorphologicalGradient::MorphologicalGradient(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theDilationSize, float theErosionSize) : ImageOperation(on, vertexShader, fragmentShader, mainContext)
{
    float min, max;
    adjustMinMax(theDilationSize, 0.0f, 1.0f, min, max);
    dilationSize = new FloatParameter("Dilation size", 0, this, theDilationSize, min, max, 0.0f, 1.0f);
    setFloatParameter(0, theDilationSize);

    adjustMinMax(theDilationSize, 0.0f, 1.0f, min, max);
    erosionSize = new FloatParameter("Erosion size", 1, this, theErosionSize, min, max, 0.0f, 1.0f);
    setFloatParameter(1, theErosionSize);
}

MorphologicalGradient::~MorphologicalGradient()
{
    delete dilationSize;
    delete erosionSize;
}

void MorphologicalGradient::setFloatParameter(int index, float value)
{
    QVector2D offset[] = {
            QVector2D(-value, value),
            QVector2D(0.0f, value),
            QVector2D(value, value),
            QVector2D(-value, 0.0f),
            QVector2D(value, 0.0f),
            QVector2D(-value, -value),
            QVector2D(0.0f, -value),
            QVector2D(value, -value)
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
}

// Polar convolution

QString PolarConvolution::name = "Polar convolution";

PolarConvolution::PolarConvolution(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, std::vector<PolarKernel*> thePolarKernels, float theCenterElement) : ImageOperation(on, vertexShader, fragmentShader, mainContext)
{
    polarKernelParameter = new PolarKernelParameter("Polar kernels", this, thePolarKernels, theCenterElement);
    setPolarKernelParameter();
}

PolarConvolution::~PolarConvolution()
{
    delete polarKernelParameter;
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

// Rotation

QString Rotation::name = "Rotation";

Rotation::Rotation(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theAngle, GLenum theMinMagFilter) : ImageOperation(on, vertexShader, fragmentShader, mainContext)
{
    float minAngle, maxAngle;
    adjustMinMax(theAngle, -360.0f, 360.0f, minAngle, maxAngle);
    angle = new FloatParameter("Angle", 0, this, theAngle, minAngle, maxAngle, -1.0e6, 1.0e6);
    setFloatParameter(0, theAngle);

    std::vector<QString> valueNames = { "Nearest neighbor", "Linear" };
    std::vector<GLenum> values = { GL_NEAREST, GL_LINEAR };
    minMagFilter = new OptionsParameter<GLenum>("Interpolation", 0, this, valueNames, values, theMinMagFilter);
    setOptionsParameter(0, theMinMagFilter);
}

Rotation::~Rotation()
{
    delete angle;
    delete minMagFilter;
}

void Rotation::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        // Set rotation matrix

        QMatrix4x4 transform;
        transform.setToIdentity();
        transform.rotate(value, 0.0f, 0.0f, 1.0f);

        // Set shader uniform

        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("transform", transform);
        fbo->program->release();
        fbo->doneCurrent();
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

// Scale

QString Scale::name = "Scale";

Scale::Scale(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theScaleFactor, GLenum theMinMagFilter) : ImageOperation(on, vertexShader, fragmentShader, mainContext)
{
    float minScaleFactor, maxScaleFactor;
    adjustMinMax(theScaleFactor, 0.0f, 2.0f, minScaleFactor, maxScaleFactor);
    scaleFactor = new FloatParameter("Scale factor", 0, this, theScaleFactor, minScaleFactor, maxScaleFactor, -1.0e6, 1.0e6);
    setFloatParameter(0, theScaleFactor);

    std::vector<QString> valueNames = { "Nearest neighbor", "Linear" };
    std::vector<GLenum> values = { GL_NEAREST, GL_LINEAR };
    minMagFilter = new OptionsParameter<GLenum>("Interpolation", 0, this, valueNames, values, theMinMagFilter);
    setOptionsParameter(0, theMinMagFilter);
}

Scale::~Scale()
{
    delete scaleFactor;
    delete minMagFilter;
}

void Scale::setFloatParameter(int index, float value)
{
    if (index == 0)
    {
        // Set scale matrix

        QMatrix4x4 transform;
        transform.setToIdentity();
        transform.scale(value, value);

        // Set shader uniform

        fbo->makeCurrent();
        fbo->program->bind();
        fbo->program->setUniformValue("transform", transform);
        fbo->program->release();
        fbo->doneCurrent();
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
