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

class NumberSignals : public QObject
{
    Q_OBJECT

public:
       explicit NumberSignals(QObject *parent = nullptr) : QObject(parent) {}

signals:
    void currentValueChanged(float currentValue);
    void currentValueChanged(int currentValue);
    void currentIndexChanged(int currentIndex);
};

template <class T>
class Number : public NumberSignals
{
public:
    T value, min, max, inf, sup;
    int indexMax = 100000;

    Number(T theValue, T theMin, T theMax, T theInf, T theSup) :
        value { theValue },
        min { theMin },
        max { theMax },
        inf { theInf },
        sup { theSup }
    {}

    Number(const Number& number)
    {
        value = number.value;
        min = number.min;
        max = number.max;
        inf = number.inf;
        sup = number.sup;
        indexMax = number.indexMax;
    }

    void setMin(T theMin) { min = theMin; }
    T getMin() { return min; }

    void setMax(T theMax) { max = theMax; }
    T getMax() { return max; }

    T getInf() { return inf; }
    T getSup() { return sup; }

    void setValue(T newValue)
    {
        value = newValue;
        emit currentValueChanged(value);
    }

    void setValueFromIndex(int newIndex)
    {
        value = static_cast<T>(min + (max - min) * static_cast<float>(newIndex) / static_cast<float>(indexMax));
        emit currentValueChanged(value);
    }

    void setIndex()
    {
        int index = static_cast<int>(indexMax * static_cast<float>(value - min) / static_cast<float>(max - min));
        emit currentIndexChanged(index);
    }

    int getIndex()
    {
        return static_cast<int>(indexMax * static_cast<float>(value - min) / static_cast<float>(max - min));
    }
};

class Parameter
{
public:
    QString name;

    Parameter(QString theName, int theIndex, ImageOperation* theOperation) :
        name { theName },
        index { theIndex },
        operation { theOperation }
    {}

    void setOperation(ImageOperation* op) { operation = op; }

    int index;
    ImageOperation* operation;
};

template <class T>
class OptionsParameter : public Parameter
{
public:
    std::vector<QString> valueNames;
    std::vector<T> values;
    T value;

    OptionsParameter(QString theName, int theIndex, ImageOperation* theOperation, std::vector<QString> theValueNames, std::vector<T> theValues, T theValue) :
        Parameter(theName, theIndex, theOperation),
        valueNames { theValueNames },
        values { theValues },
        value { theValue }
    {}

    void setValue(int valueIndex)
    {
        value = values[valueIndex];
        operation->setOptionsParameter(index, value);
    }
};

class IntParameter : public Parameter
{
public:
    Number<int>* number;
    bool isOdd;

    IntParameter(QString theName, int theIndex, ImageOperation* theOperation, int theValue, int theMin, int theMax, int theInf, int theSup, bool odd) :
        Parameter(theName, theIndex, theOperation),
        isOdd { odd }
    {
        number = new Number<int>(theValue, theMin, theMax, theInf, theSup);
    }

    IntParameter(const IntParameter& parameter) :
        Parameter(parameter)
    {
        number = new Number<int>(*parameter.number);
    }

    ~IntParameter()
    {
        delete number;
    }

    void setValue(int theValue)
    {
        operation->setIntParameter(index, theValue);
    }
};

class FloatParameter : public Parameter
{
public:
    Number<float>* number;

    FloatParameter(QString theName, int theIndex, ImageOperation* theOperation, float theValue, float theMin, float theMax, float theInf, float theSup) :
        Parameter(theName, theIndex, theOperation)
    {
        number = new Number<float>(theValue, theMin, theMax, theInf, theSup);
    }

    FloatParameter(const FloatParameter& parameter) :
        Parameter(parameter)
    {
        number = new Number<float>(*parameter.number);
    }

    ~FloatParameter()
    {
        delete number;
    }

    void setValue(float theValue)
    {
        operation->setFloatParameter(index, theValue);
    }
};

class ArrayParameter : public Parameter
{
public:
    std::vector<Number<float>*> numbers;

    ArrayParameter(QString theName, int theIndex, ImageOperation* theOperation, std::vector<float> theValues, float theMin, float theMax, float theInf, float theSup) :
        Parameter(theName, theIndex, theOperation)
    {
        for (float value : theValues)
        {
            Number<float>* number = new Number<float>(value, theMin, theMax, theInf, theSup);
            numbers.push_back(number);
        }
    }

    ArrayParameter(const ArrayParameter& parameter) :
        Parameter(parameter)
    {
        for (auto n : parameter.numbers)
        {
            Number<float>* number = new Number<float>(*n);
            numbers.push_back(number);
        }
    }

    virtual ~ArrayParameter()
    {
        for (Number<float>* number : numbers)
            delete number;
        numbers.clear();
    }

    virtual void setValues() = 0;
};

class KernelParameter : public ArrayParameter
{
public:
    bool normalize;

    KernelParameter(QString theName, int theIndex, ImageOperation* theOperation, std::vector<float> theValues, float theMin, float theMax, float theInf, float theSup, bool norm) :
        ArrayParameter(theName, theIndex, theOperation, theValues, theMin, theMax, theInf, theSup),
        normalize { norm }
    {}

    KernelParameter(const KernelParameter& parameter) :
        ArrayParameter(parameter)
    {
        normalize = parameter.normalize;
    }

    void setValues()
    {
        operation->setKernelParameter(numbers);
    }
};

class MatrixParameter : public ArrayParameter
{
public:
    MatrixParameter(QString theName, int theIndex, ImageOperation* theOperation, std::vector<float> theValues, float theMin, float theMax, float theInf, float theSup) :
        ArrayParameter(theName, theIndex, theOperation, theValues, theMin, theMax, theInf, theSup)
    {}

    MatrixParameter(const MatrixParameter& parameter) :
        ArrayParameter(parameter)
    {}

    void setValues()
    {
        operation->setMatrixParameter(numbers);
    }
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

class PolarKernelParameter : public Parameter
{
public:
    std::vector<PolarKernel*> polarKernels;
    float centerElement;

    PolarKernelParameter(QString theName, int theIndex, ImageOperation* theOperation, std::vector<PolarKernel*> thePolarKernels, float theCenterElement) :
        Parameter(theName, theIndex, theOperation),
        polarKernels { thePolarKernels },
        centerElement { theCenterElement }
    {}
    PolarKernelParameter(const PolarKernelParameter& parameter) :
        Parameter(parameter.name, parameter.index, parameter.operation),
        centerElement { parameter.centerElement }
    {
        for (PolarKernel* kernel : parameter.polarKernels)
            polarKernels.push_back(new PolarKernel(*kernel));
    }

   ~PolarKernelParameter()
    {
        for (auto& kernel : polarKernels)
            delete kernel;

        polarKernels.clear();
    }

    void setValues()
    {
        operation->setPolarKernelParameter();
    }
};
