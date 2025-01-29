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
#include <QObject>



template <class T>
class Number;
class IntParameter;
class FloatParameter;
template <class T>
class OptionsParameter;
class KernelParameter;
class MatrixParameter;



// Base image operation class

class ImageOperation
{
public:
    ImageOperation(bool on, QOpenGLContext* mainContext);
    ImageOperation(const ImageOperation& operation);
    virtual ~ImageOperation();

    virtual ImageOperation* clone() = 0;

    bool isEnabled() { return enabled; }
    virtual void enable(bool on) { enabled = on; }

    void enableBlit(bool on) { blitEnabled = on; }

    bool hasParameters() { return !noParameters; }

    virtual QString getName() = 0;

    GLuint getFBO() { return fbo->getFBO(); }

    virtual GLuint** getTextureBlit() { return fbo->getTextureBlit(); }
    virtual GLuint** getTextureID() { return fbo->getTextureID(); }

    void setInputData(QList<InputData*> data);

    virtual void resize() { blender->resize(); fbo->resize(); }

    virtual std::vector<IntParameter*> getIntParameters() { std::vector<IntParameter*> parameters; return parameters; };
    virtual std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters; return parameters; };
    virtual std::vector<OptionsParameter<int>*> getOptionsIntParameters() { std::vector<OptionsParameter<int>*> parameters; return parameters; }
    virtual std::vector<OptionsParameter<GLenum>*> getOptionsGLenumParameters() { std::vector<OptionsParameter<GLenum>*> parameters; return parameters; }
    virtual MatrixParameter* getMatrixParameter() { return nullptr; }
    virtual KernelParameter* getKernelParameter() { return nullptr; }

    virtual void setParameters() = 0;
    virtual void setIntParameter(int, int) {};
    virtual void setFloatParameter(int, float) {};
    virtual void setOptionsParameter(int, GLenum) {};
    virtual void setOptionsParameter(int, int) {};
    virtual void setMatrixParameter(std::vector<Number<float>*>) {};
    virtual void setKernelParameter(std::vector<Number<float>*>) {};
    virtual void setPolarKernelParameter() {};

    void adjustMinMax(float value, float minValue, float maxValue, float& min, float& max);

    virtual void applyOperation();
    virtual void blit();
    virtual void clear();

    QImage outputImage(){ return fbo->outputImage(); }
    void setTextureFormat(){ fbo->setTextureFormat(); }

protected:
    bool enabled;
    bool blenderEnabled = false;
    bool blitEnabled = false;
    bool noParameters = false;
    QOpenGLContext* context;
    FBO* fbo;
    Blender* blender;
};



// Bilateral filter

class BilateralFilter : public ImageOperation
{
public:
    BilateralFilter(bool on, QOpenGLContext* mainContext, int theNumSideElements, float theSize, float theSpatialSigma, float theRangeSigma, float theOpacity);
    BilateralFilter(const BilateralFilter& operation);
    ~BilateralFilter();

    ImageOperation* clone() { return new BilateralFilter(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setIntParameter(int index, int value);
    void setFloatParameter(int index, float value);

    std::vector<IntParameter*> getIntParameters() { std::vector<IntParameter*> parameters = { numSideElements }; return parameters; }
    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { size, spatialSigma, rangeSigma, opacity }; return parameters; }

private:
    IntParameter* numSideElements;
    FloatParameter* size;
    FloatParameter* spatialSigma;
    FloatParameter* rangeSigma;
    FloatParameter* opacity;

    void computeOffsets();
    void computeSpatialKernel();

    void setParametersOperation(BilateralFilter* operation);
};



// Brightness

class Brightness : public ImageOperation
{
public:
    Brightness(bool on, QOpenGLContext* mainContext, float theBrightness, float theOpacity);
    Brightness(const Brightness& operation);
    ~Brightness();

    ImageOperation* clone() { return new Brightness(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { brightness, opacity }; return parameters; }

private:
    FloatParameter* brightness;
    FloatParameter* opacity;
};



// Color mix

class ColorMix : public ImageOperation
{
public:
    ColorMix(bool on, QOpenGLContext* mainContext, std::vector<float> theMatrix, float theOpacity);
    ColorMix(const ColorMix& operation);
    ~ColorMix();

    ImageOperation* clone() { return new ColorMix(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setMatrixParameter(std::vector<Number<float>*> numbers);
    void setFloatParameter(int index, float value);

    MatrixParameter* getMatrixParameter() { return rgbMatrix; }
    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { opacity }; return parameters; }

private:
    MatrixParameter* rgbMatrix;
    FloatParameter* opacity;
};



// Color quantization

class ColorQuantization : public ImageOperation
{
public:
    ColorQuantization(bool on, QOpenGLContext* mainContext, int theRedLevels, int theGreenLevels, int theBlueLevels, float theOpacity);
    ColorQuantization(const ColorQuantization& operation);
    ~ColorQuantization();

    ImageOperation* clone() { return new ColorQuantization(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setIntParameter(int index, int value);
    void setFloatParameter(int index, float value);

    std::vector<IntParameter*> getIntParameters() { std::vector<IntParameter*> parameters = { redLevels, greenLevels, blueLevels }; return parameters; };
    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { opacity }; return parameters; }

private:
    IntParameter* redLevels;
    IntParameter* greenLevels;
    IntParameter* blueLevels;
    FloatParameter* opacity;
};



// Contrast

class Contrast : public ImageOperation
{
public:
    Contrast(bool on, QOpenGLContext* mainContext, float theContrast, float theOpacity);
    Contrast(const Contrast& operation);
    ~Contrast();

    ImageOperation* clone() { return new Contrast(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { contrast, opacity }; return parameters; };

private:
    FloatParameter* contrast;
    FloatParameter* opacity;
};



// Convolution

class Convolution : public ImageOperation
{
public:
    Convolution(bool on, QOpenGLContext* mainContext, std::vector<float> theKernel, float theFactor, float theSize, float theOpacity);
    Convolution(const Convolution& operation);
    ~Convolution();

    ImageOperation* clone() { return new Convolution(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setKernelParameter(std::vector<Number<float>*> numbers);
    void setFloatParameter(int index, float value);

    KernelParameter* getKernelParameter() { return kernel; };
    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { factor, size, opacity }; return parameters; };

private:
    KernelParameter* kernel;
    FloatParameter* factor;
    FloatParameter* size;
    FloatParameter* opacity;
};



// Dilation

class Dilation : public ImageOperation
{
public:
    Dilation(bool on, QOpenGLContext* mainContext, float theSize, float theOpacity);
    Dilation(const Dilation& operation);
    ~Dilation();

    ImageOperation* clone() { return new Dilation(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { size, opacity }; return parameters; };

private:
    FloatParameter* size;
    FloatParameter* opacity;
};



// Equalize histogram

class EqualizeHistogram : public ImageOperation
{
public:
    EqualizeHistogram(bool on, QOpenGLContext* mainContext, int theSize, int levels, float theOpacity);
    EqualizeHistogram(const EqualizeHistogram& operation);
    ~EqualizeHistogram();

    ImageOperation* clone() { return new EqualizeHistogram(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setIntParameter(int index, int value);
    void setFloatParameter(int index, float value);

    std::vector<IntParameter*> getIntParameters() { std::vector<IntParameter*> parameters = { size, levels }; return parameters; };
    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { opacity }; return parameters; }

private:
    IntParameter* size;
    IntParameter* levels;
    FloatParameter* opacity;
};



// Erosion

class Erosion : public ImageOperation
{
public:
    Erosion(bool on, QOpenGLContext* mainContext, float theSize, float theOpacity);
    Erosion(const Erosion& operation);
    ~Erosion();

    ImageOperation* clone() { return new Erosion(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { size, opacity }; return parameters; };

private:
    FloatParameter* size;
    FloatParameter* opacity;
};



// Gamma correction

class GammaCorrection : public ImageOperation
{
public:
    GammaCorrection(bool on, QOpenGLContext* mainContext, float theGammaRed, float theGammaGreen, float theGammaBlue, float theOpacity);
    GammaCorrection(const GammaCorrection& operation);
    ~GammaCorrection();

    ImageOperation* clone() { return new GammaCorrection(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { gammaRed, gammaGreen, gammaBlue, opacity }; return parameters; };

private:
    FloatParameter* gammaRed;
    FloatParameter* gammaGreen;
    FloatParameter* gammaBlue;
    FloatParameter* opacity;
};



// Geometry

class Geometry: public ImageOperation
{
public:
    Geometry(bool on, QOpenGLContext* mainContext, float theScaleX, float theScaleY, float theAngle, float theX, float theY, GLenum theMinMagFilter);
    Geometry(const Geometry& operation);
    ~Geometry();

    ImageOperation* clone() { return new Geometry(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setFloatParameter(int index, float value);
    void setOptionsParameter(int index, GLenum value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { scaleX, scaleY, angle, X, Y }; return parameters; };
    std::vector<OptionsParameter<GLenum>*> getOptionsGLenumParameters() { std::vector<OptionsParameter<GLenum>*> parameters = { minMagFilter }; return parameters; }

private:
    FloatParameter *scaleX, *scaleY;
    FloatParameter* angle;
    FloatParameter *X, *Y;
    OptionsParameter<GLenum>* minMagFilter;
};



// Hue shift

class HueShift : public ImageOperation
{
public:
    HueShift(bool on, QOpenGLContext* mainContext, float theShift, float theOpacity);
    HueShift(const HueShift& operation);
    ~HueShift();

    ImageOperation* clone() { return new HueShift(*this); }

    static QString name;
    QString getName() { return name; }

    void setParameters();
    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { shift, opacity }; return parameters; };

private:
    FloatParameter* shift;
    FloatParameter* opacity;
};



// Identity

class Identity : public ImageOperation
{
public:
    Identity(bool on, QOpenGLContext* mainContext);
    Identity(const Identity& operation);
    ~Identity();

    void setParameters() {}
    ImageOperation* clone() { return new Identity(*this); }

    static QString name;
    QString getName() { return name; };
};



// Logistic

class Logistic : public ImageOperation
{
public:
    Logistic(bool on, QOpenGLContext* mainContext, float R, float theOpacity);
    Logistic(const Logistic& operation);
    ~Logistic();

    ImageOperation* clone() { return new Logistic(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { r, opacity }; return parameters; };

private:
    FloatParameter* r;
    FloatParameter* opacity;
};



// Mask

class Mask : public QObject, public ImageOperation
{
    Q_OBJECT

public:
    Mask(bool on, QOpenGLContext* mainContext, float innerRadius, float outerRadius);
    Mask(const Mask& operation);
    ~Mask();

    ImageOperation* clone() { return new Mask(*this); }

    static QString name;
    QString getName() { return name; }

    void setParameters();
    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { innerRadius, outerRadius }; return parameters; };

private:
    FloatParameter* innerRadius;
    FloatParameter* outerRadius;

private slots:
    void setScale();
};



// Median

class Median : public ImageOperation
{
public:
    Median(bool on, QOpenGLContext* mainContext, float theSize, float theOpacity);
    Median(const Median& operation);
    ~Median();

    ImageOperation* clone() { return new Median(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { size, opacity }; return parameters; };

private:
    FloatParameter* size;
    FloatParameter* opacity;
};



// Memory

class Memory : public ImageOperation
{
public:
    Memory(bool on, QOpenGLContext* mainContext, int theFrames, float theBlendFactor, float theDecayFactor);
    Memory(const Memory& operation);
    ~Memory();

    ImageOperation* clone() { return new Memory(*this); }

    static QString name;
    QString getName() { return name; };

    GLuint** getTextureBlit() { return fboOut->getTextureBlit(); }
    GLuint** getTextureID() { return fboOut->getTextureID(); }

    void enable(bool on);

    void resize();

    void applyOperation();
    void blit();
    void clear();

    void setParameters();
    void setIntParameter(int index, int value);
    void setFloatParameter(int index, float value);

    std::vector<IntParameter*> getIntParameters() { std::vector<IntParameter*> parameters = { frames }; return parameters; };
    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { blendFactor, decayFactor }; return parameters; };

private:
    QList<FBO*> fbos;
    Blender* blenderOut;
    FBO* fboOut;

    IntParameter* frames;
    FloatParameter* blendFactor;
    FloatParameter* decayFactor;

    void setBlenderOutInputData();
};



// Morphological gradient

class MorphologicalGradient : public ImageOperation
{
public:
    MorphologicalGradient(bool on, QOpenGLContext* mainContext, float theDilationSize, float theErosionSize, float theOpacity);
    MorphologicalGradient(const MorphologicalGradient& operation);
    ~MorphologicalGradient();

    ImageOperation* clone() { return new MorphologicalGradient(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { dilationSize, erosionSize, opacity }; return parameters; };

private:
    FloatParameter* dilationSize;
    FloatParameter* erosionSize;
    FloatParameter* opacity;
};

// Morphological gradient

class Pixelation : public QObject, public ImageOperation
{
    Q_OBJECT

public:
    Pixelation(bool on, QOpenGLContext* mainContext, float theSize, float theOpacity);
    Pixelation(const Pixelation& operation);
    ~Pixelation();

    ImageOperation* clone() { return new Pixelation(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { size, opacity }; return parameters; };

private:
    FloatParameter* size;
    FloatParameter* opacity;

private slots:
    void setWidthAndHeight();
};



// Power

class Power: public ImageOperation
{
public:
    Power(bool on, QOpenGLContext* mainContext, float theExponent, float theOpacity);
    Power(const Power& operation);
    ~Power();

    ImageOperation* clone() { return new Power(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { exponent, opacity }; return parameters; };

private:
    FloatParameter* exponent;
    FloatParameter* opacity;
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

    void setParameters();
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
    Saturation(bool on, QOpenGLContext* mainContext, float theSaturation, float theOpacity);
    Saturation(const Saturation& operation);
    ~Saturation();

    ImageOperation* clone() { return new Saturation(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { saturation, opacity }; return parameters; };

private:
    FloatParameter* saturation;
    FloatParameter* opacity;
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

    void setParameters();
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
    Value(bool on, QOpenGLContext* mainContext, float theValue, float theOpacity);
    Value(const Value& operation);
    ~Value();

    ImageOperation* clone() { return new Value(*this); }

    static QString name;
    QString getName() { return name; };

    void setParameters();
    void setFloatParameter(int index, float value);

    std::vector<FloatParameter*> getFloatParameters() { std::vector<FloatParameter*> parameters = { value, opacity }; return parameters; };

private:
    FloatParameter* value;
    FloatParameter* opacity;
};
