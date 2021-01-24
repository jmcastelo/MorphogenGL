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

#include "fbo.h"
#include "blender.h"
#include <vector>
#include <cmath>
#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QString>
#include <QMap>
#include <QUuid>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QDebug>

struct BoolParameter;
class IntParameter;
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
    ImageOperation(bool on, QOpenGLContext* mainContext);
    ImageOperation(const ImageOperation& operation);
    virtual ~ImageOperation();

    virtual ImageOperation* clone() = 0;

    bool isEnabled() { return enabled; }
    void enable(bool on) { enabled = on; }

    bool hasParameters() { return !noParameters; }

    virtual QString getName() = 0;

    GLuint** getTextureBlit() { return fbo->getTextureBlit(); }
    GLuint** getTextureID() { return fbo->getTextureID(); }

    void setInputData(QVector<InputData*> data);

    void resize() { blender->resize(); fbo->resize(); }

    virtual std::vector<BoolParameter*> getBoolParameters() { std::vector<BoolParameter*> parameters; return parameters; };
    virtual std::vector<IntParameter*> getIntParameters() { std::vector<IntParameter*> parameters; return parameters; };
    virtual std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters; return parameters; };
    virtual std::vector<OptionsParameter<int>*> getOptionsIntParameters() { std::vector<OptionsParameter<int>*> parameters; return parameters; }
    virtual std::vector<OptionsParameter<GLenum>*> getOptionsGLenumParameters() { std::vector<OptionsParameter<GLenum>*> parameters; return parameters; }
    virtual MatrixParameter* getMatrixParameter() { return nullptr; }
    virtual KernelParameter* getKernelParameter() { return nullptr; }
    virtual PolarKernelParameter* getPolarKernelParameter() { return nullptr; }
    
    virtual void setIntParameter(int, int) {}
    virtual void setFloatParameter(int, float) {}
    virtual void setOptionsParameter(int, GLenum) {}
    virtual void setOptionsParameter(int, int) {}
    virtual void setMatrixParameter(GLfloat*) {}
    virtual void setKernelParameter(std::vector<float>) {}
    virtual void setPolarKernelParameter() {}

    void adjustMinMax(float value, float minValue, float maxValue, float& min, float& max);

    void applyOperation();
    void blit();
    void clear();

protected:
    bool enabled;
    bool noParameters = false;
    QOpenGLContext* context;
    FBO* fbo;
    Blender* blender;
};

// Bilateral filter

class BilateralFilter : public ImageOperation
{
public:
    BilateralFilter(bool on, QOpenGLContext* mainContext, int theNumSideElements, float theSize, float theSpatialSigma, float theRangeSigma);
    BilateralFilter(const BilateralFilter& operation);
    ~BilateralFilter();

    ImageOperation* clone() { return new BilateralFilter(*this); }

    static QString name;
    QString getName() { return name; };

    void setIntParameter(int index, int value);
    void setFloatParameter(int index, float value);

    std::vector<IntParameter*> getIntParameters() { std::vector<IntParameter*> parameters = { numSideElements }; return parameters; };
    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { size, spatialSigma, rangeSigma }; return parameters; }

private:
    IntParameter* numSideElements;
    FloatParameter* size;
    FloatParameter* spatialSigma;
    FloatParameter* rangeSigma;

    void computeOffsets();
    void computeSpatialKernel();

    void setParametersOperation(BilateralFilter* operation);
};

// Brightness

class Brightness : public ImageOperation
{
public:
    Brightness(bool on, QOpenGLContext* mainContext, float theBrightness);
    Brightness(const Brightness& operation);
    ~Brightness();

    ImageOperation* clone() { return new Brightness(*this); }

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
    ColorMix(bool on, QOpenGLContext* mainContext, std::vector<float> theMatrix);
    ColorMix(const ColorMix& operation);
    ~ColorMix();

    ImageOperation* clone() { return new ColorMix(*this); }

    static QString name;
    QString getName() { return name; };

    void setMatrixParameter(GLfloat* values);

    MatrixParameter* getMatrixParameter() { return rgbMatrix; }

private:
    MatrixParameter* rgbMatrix;
};

// Color quantization

class ColorQuantization : public ImageOperation
{
public:
    ColorQuantization(bool on, QOpenGLContext* mainContext, int theRedLevels, int theGreenLevels, int theBlueLevels);
    ColorQuantization(const ColorQuantization& operation);
    ~ColorQuantization();

    ImageOperation* clone() { return new ColorQuantization(*this); }

    static QString name;
    QString getName() { return name; };

    void setIntParameter(int index, int value);

    std::vector<IntParameter*> getIntParameters() { std::vector<IntParameter*> parameters = { redLevels, greenLevels, blueLevels }; return parameters; };

private:
    IntParameter* redLevels;
    IntParameter* greenLevels;
    IntParameter* blueLevels;
};

// Contrast

class Contrast : public ImageOperation
{
public:
    Contrast(bool on, QOpenGLContext* mainContext, float theContrast);
    Contrast(const Contrast& operation);
    ~Contrast();

    ImageOperation* clone() { return new Contrast(*this); }

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
    Convolution(bool on, QOpenGLContext* mainContext, std::vector<float> theKernel, float theFactor, float theSize);
    Convolution(const Convolution& operation);
    ~Convolution();

    ImageOperation* clone() { return new Convolution(*this); }

    static QString name;
    QString getName() { return name; };

    void setKernelParameter(std::vector<float> values);
    void setFloatParameter(int index, float value);

    KernelParameter* getKernelParameter() { return kernel; };
    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { factor, size }; return parameters; };

private:
    KernelParameter* kernel;
    FloatParameter* factor;
    FloatParameter* size;
};

// Dilation

class Dilation : public ImageOperation
{
public:
    Dilation(bool on, QOpenGLContext* mainContext, float theSize);
    Dilation(const Dilation& operation);
    ~Dilation();

    ImageOperation* clone() { return new Dilation(*this); }

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
    Erosion(bool on, QOpenGLContext* mainContext, float theSize);
    Erosion(const Erosion& operation);
    ~Erosion();

    ImageOperation* clone() { return new Erosion(*this); }

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
    GammaCorrection(bool on, QOpenGLContext* mainContext, float theGammaRed, float theGammaGreen, float theGammaBlue);
    GammaCorrection(const GammaCorrection& operation);
    ~GammaCorrection();

    ImageOperation* clone() { return new GammaCorrection(*this); }

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
    HueShift(bool on, QOpenGLContext* mainContext, float theShift);
    HueShift(const HueShift& operation);
    ~HueShift();

    ImageOperation* clone() { return new HueShift(*this); }

    static QString name;
    QString getName() { return name; };

    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { shift }; return parameters; };

private:
    FloatParameter* shift;
};

// Identity

class Identity : public ImageOperation
{
public:
    Identity(bool on, QOpenGLContext* mainContext);
    Identity(const Identity& operation);
    ~Identity();

    ImageOperation* clone() { return new Identity(*this); }

    static QString name;
    QString getName() { return name; };
};

// Hue shift

class Logistic : public ImageOperation
{
public:
    Logistic(bool on, QOpenGLContext* mainContext, float R);
    Logistic(const Logistic& operation);
    ~Logistic();

    ImageOperation* clone() { return new Logistic(*this); }

    static QString name;
    QString getName() { return name; };

    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { r }; return parameters; };

private:
    FloatParameter* r;
};

// Mask

class Mask : public ImageOperation
{
public:
    Mask(bool on, QOpenGLContext* mainContext);
    Mask(const Mask& operation);
    ~Mask();

    ImageOperation* clone() { return new Mask(*this); }

    static QString name;
    QString getName() { return name; };
};


// Morphological gradient

class MorphologicalGradient : public ImageOperation
{
public:
    MorphologicalGradient(bool on, QOpenGLContext* mainContext, float theDilationSize, float theErosionSize);
    MorphologicalGradient(const MorphologicalGradient& operation);
    ~MorphologicalGradient();

    ImageOperation* clone() { return new MorphologicalGradient(*this); }

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
    PolarConvolution(bool on, QOpenGLContext* mainContext, std::vector<PolarKernel*> thePolarKernels, float theCenterElement);
    PolarConvolution(const PolarConvolution& operation);
    ~PolarConvolution();

    ImageOperation* clone() { return new PolarConvolution(*this); }

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
    Rotation(bool on, QOpenGLContext* mainContext, float theAngle, GLenum theMinMagFilter);
    Rotation(const Rotation& operation);
    ~Rotation();

    ImageOperation* clone() { return new Rotation(*this); }

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
    Saturation(bool on, QOpenGLContext* mainContext, float theSaturation);
    Saturation(const Saturation& operation);
    ~Saturation();

    ImageOperation* clone() { return new Saturation(*this); }

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
    Scale(bool on, QOpenGLContext* mainContext, float theScaleFactor, GLenum theMinMagFilter);
    Scale(const Scale& operation);
    ~Scale();

    ImageOperation* clone() { return new Scale(*this); }

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
    Value(bool on, QOpenGLContext* mainContext, float theValue);
    Value(const Value& operation);
    ~Value();

    ImageOperation* clone() { return new Value(*this); }

    static QString name;
    QString getName() { return name; };

    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { value }; return parameters; };

private:
    FloatParameter* value;
};
