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

#pragma once

#include "fbo.h"
#include <vector>
#include <cmath>
#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QString>
#include <QOpenGLContext>
#include <QDebug>

struct BoolParameter;
struct IntParameter;
class FloatParameter;
template <class T>
class OptionsParameter;
class KernelParameter;
class MatrixParameter;
struct PolarKernel;
class PolarKernelParameter;

// Base image operation class

class ImageOperation
{
public:
    ImageOperation(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext);
    virtual ~ImageOperation();

    bool isEnabled() { return enabled; }
    void enable(bool on) { enabled = on; };

    virtual QString getName() = 0;

    GLuint getTextureID() { return fbo->getTextureID(); }
    void resize() { fbo->resize(); };

    virtual std::vector<BoolParameter*> getBoolParameters() { std::vector<BoolParameter*> parameters; return parameters; };
    virtual std::vector<IntParameter*> getIntParameters() { std::vector<IntParameter*> parameters; return parameters; };
    virtual std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters; return parameters; };
    virtual std::vector<OptionsParameter<int>*> getOptionsIntParameters() { std::vector<OptionsParameter<int>*> parameters; return parameters; }
    virtual std::vector<OptionsParameter<GLenum>*> getOptionsGLenumParameters() { std::vector<OptionsParameter<GLenum>*> parameters; return parameters; }
    virtual MatrixParameter* getMatrixParameter() { return nullptr; }
    virtual KernelParameter* getKernelParameter() { return nullptr; }
    virtual PolarKernelParameter* getPolarKernelParameter() { return nullptr; }
    
    virtual void setFloatParameter(int, float) {}
    virtual void setOptionsParameter(int, GLenum) {}
    virtual void setOptionsParameter(int, int) {}
    virtual void setMatrixParameter(GLfloat*) {}
    virtual void setKernelParameter(GLfloat*) {}
    virtual void setPolarKernelParameter() {}

    void adjustMinMax(float value, float minValue, float maxValue, float& min, float& max);

    void applyOperation(GLuint inTextureID);

protected:
    bool enabled;
    FBO* fbo;
};

// Brightness

class Brightness : public ImageOperation
{
public:
    Brightness(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theBrightness);
    ~Brightness();

    static QString name;
    QString getName() { return name; };

    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { brightness }; return parameters; }

private:
    FloatParameter* brightness;
};

// Color mix

class ColorMix : public ImageOperation
{
public:
    ColorMix(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, std::vector<float> theMatrix);
    ~ColorMix();

    static QString name;
    QString getName() { return name; };

    void setMatrixParameter(GLfloat* values);

    MatrixParameter* getMatrixParameter() { return rgbMatrix; }

private:
    MatrixParameter* rgbMatrix;
};

// Contrast

class Contrast : public ImageOperation
{
public:
    Contrast(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theContrast);
    ~Contrast();

    static QString name;
    QString getName() { return name; };

    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { contrast }; return parameters; };

private:
    FloatParameter* contrast;
};

// Convolution

class Convolution : public ImageOperation
{
public:
    Convolution(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, std::vector<float> theKernel, float theSize);
    ~Convolution();

    static QString name;
    QString getName() { return name; };

    void setKernelParameter(GLfloat* values);
    void setFloatParameter(int index, float value);

    KernelParameter* getKernelParameter() { return kernel; };
    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { size }; return parameters; };

private:
    KernelParameter* kernel;
    FloatParameter* size;
};

// Dilation

class Dilation : public ImageOperation
{
public:
    Dilation(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theSize);
    ~Dilation();

    static QString name;
    QString getName() { return name; };

    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { size }; return parameters; };

private:
    FloatParameter* size;
};

// Erosion

class Erosion : public ImageOperation
{
public:
    Erosion(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theSize);
    ~Erosion();

    static QString name;
    QString getName() { return name; };

    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { size }; return parameters; };

private:
    FloatParameter* size;
};

// Gamma correction

class GammaCorrection : public ImageOperation
{
public:
    GammaCorrection(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theGammaRed, float theGammaGreen, float theGammaBlue);
    ~GammaCorrection();

    static QString name;
    QString getName() { return name; };

    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { gammaRed, gammaGreen, gammaBlue }; return parameters; };

private:
    FloatParameter* gammaRed;
    FloatParameter* gammaGreen;
    FloatParameter* gammaBlue;
};

// Hue shift

class HueShift : public ImageOperation
{
public:
    HueShift(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theShift);
    ~HueShift();

    static QString name;
    QString getName() { return name; };

    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { shift }; return parameters; };

private:
    FloatParameter* shift;
};

// Morphological gradient

class MorphologicalGradient : public ImageOperation
{
public:
    MorphologicalGradient(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theDilationSize, float theErosionSize);
    ~MorphologicalGradient();

    static QString name;
    QString getName() { return name; };

    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { dilationSize, erosionSize }; return parameters; };

private:
    FloatParameter* dilationSize;
    FloatParameter* erosionSize;
};

// Polar convolution

class PolarConvolution : public ImageOperation
{
public:
    PolarConvolution(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, std::vector<PolarKernel*> thePolarKernels, float theCenterElement);
    ~PolarConvolution();

    static QString name;
    QString getName() { return name; }

    void setPolarKernelParameter();

    PolarKernelParameter* getPolarKernelParameter() { return polarKernelParameter; }

private:
    PolarKernelParameter* polarKernelParameter;
};

// Rotation

class Rotation: public ImageOperation
{
public:
    Rotation(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theAngle, GLenum theMinMagFilter);
    ~Rotation();

    static QString name;
    QString getName() { return name; };

    void setFloatParameter(int index, float value);
    void setOptionsParameter(int index, GLenum value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { angle }; return parameters; };
    std::vector<OptionsParameter<GLenum>*> getOptionsGLenumParameters() { std::vector<OptionsParameter<GLenum>*> parameters = { minMagFilter }; return parameters; }

private:
    FloatParameter* angle;
    OptionsParameter<GLenum>* minMagFilter;
};

// Saturation

class Saturation: public ImageOperation
{
public:
    Saturation(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theSaturation);
    ~Saturation();

    static QString name;
    QString getName() { return name; };

    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { saturation }; return parameters; };

private:
    FloatParameter* saturation;
};

// Scale

class Scale : public ImageOperation
{
public:
    Scale(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theScaleFactor, GLenum theMinMagFilter);
    ~Scale();

    static QString name;
    QString getName() { return name; };

    void setFloatParameter(int index, float value);
    void setOptionsParameter(int index, GLenum value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { scaleFactor }; return parameters; };
    std::vector<OptionsParameter<GLenum>*> getOptionsGLenumParameters() { std::vector<OptionsParameter<GLenum>*> parameters = { minMagFilter }; return parameters; }

private:
    FloatParameter* scaleFactor;
    OptionsParameter<GLenum>* minMagFilter;
};

// Value

class Value: public ImageOperation
{
public:
    Value(bool on, QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext, float theValue);
    ~Value();

    static QString name;
    QString getName() { return name; };

    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { value }; return parameters; };

private:
    FloatParameter* value;
};
