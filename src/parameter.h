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

#include "imageoperations.h"
#include <vector>
#include <QString>

class ImageOperation;

struct BoolParameter
{
    QString name;
    bool value;

    BoolParameter(QString theName, bool theValue) :
        name { theName },
        value { theValue }
    {}
};

template <class T>
class OptionsParameter
{
public:
    QString name;
    std::vector<QString> valueNames;
    std::vector<T> values;
    T value;

    OptionsParameter(QString theName, int theIndex, ImageOperation* theOperation, std::vector<QString> theValueNames, std::vector<T> theValues, T theValue) :
        name { theName },
        valueNames { theValueNames },
        values { theValues },
        value { theValue },
        index { theIndex },
        operation { theOperation }
    {}

    void setValue(int valueIndex)
    {
        value = values[valueIndex];
        operation->setOptionsParameter(index, value);
    }

    void setOperation(ImageOperation* op) { operation = op; }

private:
    int index;
    ImageOperation* operation;
};

class IntParameter
{
public:
    QString name;
    int value, min, max;
    bool isOdd;

    IntParameter(QString theName, int theIndex, ImageOperation* theOperation, int theValue, int theMin, int theMax, bool odd) :
        name { theName },
        value { theValue },
        min { theMin },
        max { theMax },
        isOdd { odd },
        index { theIndex },
        operation { theOperation }
    {}

    void setValue(int theValue);
    void setOperation(ImageOperation* theOperation) { operation = theOperation; }

private:
    int index;
    ImageOperation* operation;
};

class FloatParameter
{
public:
    QString name;
    float value, min, max, inf, sup;
    
    FloatParameter(QString theName, int theIndex, ImageOperation* theOperation, float theValue, float theMin, float theMax, float theInf, float theSup) :
        name { theName },
        value { theValue },
        min { theMin },
        max { theMax },
        inf { theInf },
        sup { theSup },
        index { theIndex },
        operation { theOperation }
    {}

    void setValue(float theValue);
    void setOperation(ImageOperation* op) { operation = op; }

private:
    int index;
    ImageOperation* operation;
};

class ArrayParameter
{
public:
    QString name;
    std::vector<float> values;
    float min, max;
    ArrayParameter(QString theName, ImageOperation* theOperation, std::vector<float> theValues, float theMin, float theMax) :
        name { theName },
        values { theValues },
        min { theMin },
        max { theMax },
        operation { theOperation }
    {}
    virtual ~ArrayParameter(){}

    virtual void setValues() = 0;

protected:
    ImageOperation* operation;
};

class KernelParameter : public ArrayParameter
{
public:
    bool normalize;
    KernelParameter(QString theName, ImageOperation* theOperation, std::vector<float> theValues, float theMin, float theMax, bool norm) :
        ArrayParameter(theName, theOperation, theValues, theMin, theMax),
        normalize { norm }
    {}

    void setValues();
    void setOperation(ImageOperation* op) { operation = op; }
};

class MatrixParameter : public ArrayParameter
{
public:
    MatrixParameter(QString theName, ImageOperation* theOperation, std::vector<float> theValues, float theMin, float theMax) :
        ArrayParameter(theName, theOperation, theValues, theMin, theMax)
    {}

    void setValues();
    void setOperation(ImageOperation* op) { operation = op; }
};

struct PolarKernel
{
    int numElements;
    float radius;
    float initialAngle;
    float frequency;
    float phase;
    float minimum;
    float maximum;

    PolarKernel(int theNumElements, float theRadius, float theInitialAngle, float theFrequency, float thePhase, float theMinimum, float theMaximum) :
        numElements { theNumElements },
        radius { theRadius },
        initialAngle { theInitialAngle },
        frequency { theFrequency },
        phase { thePhase },
        minimum { theMinimum},
        maximum { theMaximum }
    {}
};

class PolarKernelParameter
{
public:
    QString name;
    std::vector<PolarKernel*> polarKernels;
    float centerElement;

    PolarKernelParameter(QString theName, ImageOperation* theOperation, std::vector<PolarKernel*> thePolarKernels, float theCenterElement) :
        name { theName },
        polarKernels { thePolarKernels },
        centerElement { theCenterElement },
        operation { theOperation }
    {}
    PolarKernelParameter(const PolarKernelParameter& parameter);
    ~PolarKernelParameter();

    void setValues();
    void setOperation(ImageOperation* op) { operation = op; }

private:
    ImageOperation* operation;
};
