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



#ifndef PARAMETER_H
#define PARAMETER_H



#include "imageoperations.h"

#include <QObject>
#include <QVariant>
#include <QString>
#include <QUuid>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix2x2>
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QOpenGLExtraFunctions>



class ImageOperation;



class NumberSignals : public QObject
{
    Q_OBJECT

public:
    explicit NumberSignals(QObject *parent = nullptr) : QObject(parent) {}

signals:
    void valueChanged(QVariant value);
    void indexChanged(int index);
    void deleting();
    void linked(bool set);
};



template <class T>
class Number : public NumberSignals
{
public:
    Number(T theValue, T theMin, T theMax, T theInf, T theSup) :
        mValue { theValue },
        mMin { theMin },
        mMax { theMax },
        mInf { theInf },
        mSup { theSup }
    {
        mId = QUuid::createUuid();
    }

    Number(QUuid theId, T theValue, T theMin, T theMax, T theInf, T theSup) :
        mId { theId },
        mValue { theValue },
        mMin { theMin },
        mMax { theMax },
        mInf { theInf },
        mSup { theSup }
    {}

    Number(const Number<T>& number) :
        NumberSignals()
    {
        mId = number.mId;
        mValue = number.mValue;
        mMin = number.mMin;
        mMax = number.mMax;
        mInf = number.mInf;
        mSup = number.mSup;
        mIndexMax = number.mIndexMax;
    }

    ~Number()
    {
        emit deleting();
    }

    QUuid id(){ return mId; }

    void setMin(T theMin) { mMin = theMin; }
    T min(){ return mMin; }

    void setMax(T theMax) { mMax = theMax; }
    T max(){ return mMax; }

    T inf(){ return mInf; }
    T sup(){ return mSup; }

    void setValue(T theValue)
    {
        if (theValue < mMin)
            theValue = mMin;
        else if (theValue > mMax)
            theValue = mMax;

        mValue = theValue;

        emit valueChanged(mValue);
    }

    void setValueFromIndex(int theIndex)
    {
        mValue = static_cast<T>(mMin + (mMax - mMin) * static_cast<float>(theIndex) / static_cast<float>(mIndexMax));
        emit valueChanged(mValue);
    }

    T value() { return mValue; }

    void setIndex()
    {
        int index = static_cast<int>(mIndexMax * static_cast<float>(mValue - mMin) / static_cast<float>(mMax - mMin));
        emit indexChanged(index);
    }

    int index()
    {
        return static_cast<int>(mIndexMax * static_cast<float>(mValue - mMin) / static_cast<float>(mMax - mMin));
    }

    int indexMax() { return mIndexMax; }
    void setIndexMax(int theIndexMax){ mIndexMax = theIndexMax; }

    bool midiLinked(){ return mMidiLinked; }

    void setMidiLinked(bool set)
    {
        mMidiLinked = set;
        emit linked(mMidiLinked);
    }

private:
    QUuid mId;
    T mValue, mMin, mMax, mInf, mSup;
    int mIndexMax = 100'000;
    bool mMidiLinked = false;
};



class ParameterSignals : public QObject
{
    Q_OBJECT

public:
    explicit ParameterSignals(QObject *parent = nullptr) : QObject(parent) {}

signals:
    void valueChanged(QVariant value);
    void valueChanged(int i, QVariant value);
    void indexChanged(int index);
};



enum class UniformType
{
    FLOAT = GL_FLOAT,
    FLOAT_VEC2 = GL_FLOAT_VEC2,
    FLOAT_VEC3 = GL_FLOAT_VEC3,
    FLOAT_VEC4 = GL_FLOAT_VEC4,
    INT = GL_INT,
    INT_VEC2 = GL_INT_VEC2,
    INT_VEC3 = GL_INT_VEC3,
    INT_VEC4 = GL_INT_VEC4,
    UNSIGNED_INT = GL_UNSIGNED_INT,
    UNSIGNED_INT_VEC2 = GL_UNSIGNED_INT_VEC2,
    UNSIGNED_INT_VEC3 = GL_UNSIGNED_INT_VEC3,
    UNSIGNED_INT_VEC4 = GL_UNSIGNED_INT_VEC4,
    FLOAT_MAT_2 = GL_FLOAT_MAT2,
    FLOAT_MAT_3 = GL_FLOAT_MAT3,
    FLOAT_MAT_4 = GL_FLOAT_MAT4
};



class Parameter : public ParameterSignals
{
public:
    Parameter(QString theName, QString theUniformName, UniformType theUniformType, bool isEditable, ImageOperation* theOperation) :
        ParameterSignals(),
        mName { theName },
        mUniformName { theUniformName },
        mUniformType { theUniformType },
        mEditable { isEditable },
        mOperation { theOperation }
    {}

    Parameter(const Parameter& parameter) :
        ParameterSignals()
    {
        mName = parameter.mName;
        mUniformName = parameter.mUniformName;
        mUniformType = parameter.mUniformType;
        mEditable = parameter.mEditable;
    }

    QString name() { return mName; }
    QString uniformName() { return mUniformName; }
    UniformType uniformType() { return mUniformType; }
    bool editable() { return mEditable; }

    void setOperation(ImageOperation* operation) { mOperation = operation; }

protected:
    QString mName;
    QString mUniformName;
    UniformType mUniformType;
    bool mEditable;
    ImageOperation* mOperation;
};



template <typename T>
class OptionsParameter : public Parameter
{
public:
    OptionsParameter(QString theName, QString theUniformName, UniformType theUniformType, bool isEditable, QList<QString> theValueNames, QList<T> theValues, T theValue, ImageOperation* theOperation) :
        Parameter(theName, theUniformName, theUniformType, isEditable, theOperation),
        mValueNames { theValueNames },
        mValues { theValues },
        mCurrentValue { theValue }
    {}

    OptionsParameter(const OptionsParameter<T>& parameter) :
        Parameter(parameter)
    {
        mValueNames = parameter.mValueNames;
        mValues = parameter.mValues;
        mCurrentValue = parameter.mCurrentValue;
    }

    void setValue(int valueIndex)
    {
        mCurrentValue = mValues[valueIndex];
        mOperation->setOptionsParameter<T>(this);
    }

    T value(){ return mCurrentValue; }

    int indexOf()
    {
        return mValues.indexOf(mCurrentValue);
    }

    QList<QString> valueNames(){ return mValueNames; }

private:
    QList<QString> mValueNames;
    QList<T> mValues;
    T mCurrentValue;
};



template <typename T>
class UniformParameter : public Parameter
{
public:
    UniformParameter(QString theName, QString theUniformName, UniformType theUniformType, int numItems, bool isEditable, QList<T> theValues, T theMin, T theMax, T theInf, T theSup, ImageOperation* theOperation) :
        Parameter(theName, theUniformName, theUniformType, isEditable, theOperation),
        nItems { numItems }
    {
        for (T value : theValues)
        {
            Number<T>* number = new Number<T>(value, theMin, theMax, theInf, theSup);
            mNumbers.append(number);
        }
    }

    UniformParameter(QString theName, QString theUniformName, UniformType theUniformType, bool isEditable, QList<QPair<QUuid, T>> theIdValuePairs, T theMin, T theMax, T theInf, T theSup, ImageOperation* theOperation) :
        Parameter(theName, theUniformName, theUniformType, isEditable, theOperation)
    {
        for (QPair<QUuid, T> pair : theIdValuePairs)
        {
            Number<T>* number = new Number<T>(pair.first. pair.second, theMin, theMax, theInf, theSup);
            mNumbers.append(number);
        }
    }

    UniformParameter(const UniformParameter<T>& parameter) :
        Parameter(parameter)
    {
        for (Number<T>* number : parameter.mNumbers)
        {
            Number<T>* newNumber = new Number<T>(*number);
            mNumbers.push_back(newNumber);
        }
    }

    ~UniformParameter()
    {
        qDeleteAll(mNumbers);
    }

    T value(int i)
    {
        if (i < mNumbers.size())
            return mNumbers.at(i)->value();
        else
            return 0;
    }

    void setValue(int i, T theValue)
    {
        if (i < mNumbers.size())
        {
            mNumbers[i]->setValue(theValue);
            mNumbers[i]->setIndex();

            emit valueChanged(i, QVariant(theValue));
            emit valueChanged(QVariant(theValue));

            setUniform();
        }
    }

    void setUniform()
    {
        mOperation->setUniform<T>(mName, mUniformType, nItems, values().constData());
    }

    QList<T> values()
    {
        QList<T> theValues;
        for (Number<T>* number : mNumbers)
            theValues.append(number->value());
        return theValues;
    }

    QList<Number<T>*> numbers()
    {
        return mNumbers;
    }

    Number<T>* number(QUuid theId)
    {
        for (Number<T>* number : mNumbers)
            if (theId == number->id())
                return number;
        return nullptr;
    }

    Number<T>* number(int i)
    {
        if (i < mNumbers.size())
            return mNumbers[i];
        else
            return new Number<T>(0, 0, 0, 0, 0);
    }

    int size()
    {
        return mNumbers.size();
    }

    int numItems()
    {
        return nItems;
    }

    QPair<int, int> colsRowsPerItem()
    {
        if (mUniformType == UniformType::FLOAT || mUniformType == UniformType::INT || mUniformType == UniformType::UNSIGNED_INT)
            return QPair<int, int>(1, 1);
        else if (mUniformType == UniformType::FLOAT_VEC2 || mUniformType == UniformType::INT_VEC2 || mUniformType == UniformType::UNSIGNED_INT_VEC2)
            return QPair<int, int>(2, 1);
        else if (mUniformType == UniformType::FLOAT_VEC3 || mUniformType == UniformType::INT_VEC3 || mUniformType == UniformType::UNSIGNED_INT_VEC3)
            return QPair<int, int>(3, 1);
        else if (mUniformType == UniformType::FLOAT_VEC4 || mUniformType == UniformType::INT_VEC4 || mUniformType == UniformType::UNSIGNED_INT_VEC4)
            return QPair<int, int>(4, 1);
        else if (mUniformType == UniformType::FLOAT_MAT_2)
            return QPair<int, int>(2, 2);
        else if (mUniformType == UniformType::FLOAT_MAT_3)
            return QPair<int, int>(3, 3);
        else if (mUniformType == UniformType::FLOAT_MAT_4)
            return QPair<int, int>(4, 4);

        return QPair<int, int>(9, 0);
    }

private:
    QList<Number<T>*> mNumbers;
    int nItems;
};



enum class UniformMat4Type
{
    TRANSLATION = 0,
    ROTATION = 1,
    SCALING = 2,
    ORTHOGRAPHIC = 3
};


class UniformMat4Parameter : public Parameter
{
public:
    UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, QList<float> theValues, float theMin, float theMax, float theInf, float theSup, ImageOperation* theOperation) :
        Parameter(theName, theUniformName, UniformType::FLOAT_MAT_4, isEditable, theOperation),
        mType { theMat4Type }
    {
        if (mType == UniformMat4Type::TRANSLATION)
        {
            mNumberNames.append("X");
            mNumbers.append(new Number<float>(theValues.at(0), theMin, theMax, theInf, theSup));

            mNumberNames.append("Y");
            mNumbers.append(new Number<float>(theValues.at(1), theMin, theMax, theInf, theSup));
        }
        else if (mType == UniformMat4Type::ROTATION)
        {
            mNumberNames.append("Angle");
            mNumbers.append(new Number<float>(theValues.at(0), theMin, theMax, theInf, theSup));
        }
        else if (mType == UniformMat4Type::SCALING)
        {
            mNumberNames.append("X");
            mNumbers.append(new Number<float>(theValues.at(0), theMin, theMax, theInf, theSup));

            mNumberNames.append("Y");
            mNumbers.append(new Number<float>(theValues.at(1), theMin, theMax, theInf, theSup));
        }
        else if (mType == UniformMat4Type::ORTHOGRAPHIC)
        {
            mEditable = false;
            mOperation->setOrthographicProjection(mUniformName);
        }
    }

    UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, QList<QPair<QUuid, float>> theIdValuePairs, float theMin, float theMax, float theInf, float theSup, ImageOperation* theOperation) :
        Parameter(theName, theUniformName, UniformType::FLOAT_MAT_4, isEditable, theOperation),
        mType { theMat4Type }
    {
        if (mType == UniformMat4Type::TRANSLATION)
        {
            mNumberNames.append("X");
            mNumbers.append(new Number<float>(theIdValuePairs.at(0).first, theIdValuePairs.at(0).second, theMin, theMax, theInf, theSup));

            mNumberNames.append("Y");
            mNumbers.append(new Number<float>(theIdValuePairs.at(1).first, theIdValuePairs.at(1).second, theMin, theMax, theInf, theSup));
        }
        else if (mType == UniformMat4Type::ROTATION)
        {
            mNumberNames.append("Angle");
            mNumbers.append(new Number<float>(theIdValuePairs.at(0).first, theIdValuePairs.at(0).second, theMin, theMax, theInf, theSup));
        }
        else if (mType == UniformMat4Type::SCALING)
        {
            mNumberNames.append("X");
            mNumbers.append(new Number<float>(theIdValuePairs.at(0).first, theIdValuePairs.at(0).second, theMin, theMax, theInf, theSup));

            mNumberNames.append("Y");
            mNumbers.append(new Number<float>(theIdValuePairs.at(1).first, theIdValuePairs.at(1).second, theMin, theMax, theInf, theSup));
        }
        else if (mType == UniformMat4Type::ORTHOGRAPHIC)
        {
            mEditable = false;
            mOperation->setOrthographicProjection(mUniformName);
        }
    }

    UniformMat4Parameter(const UniformMat4Parameter& parameter) :
        Parameter(parameter)
    {
        mNumberNames = parameter.mNumberNames;

        for (const Number<float>* number : parameter.mNumbers)
        {
            Number<float>* newNumber = new Number<float>(*number);
            mNumbers.push_back(newNumber);
        }
    }

    ~UniformMat4Parameter()
    {
        qDeleteAll(mNumbers);
    }

    float value(int i)
    {
        if (i < mNumbers.size())
            return mNumbers.at(i)->value();
        else
            return 0;
    }

    void setValue(int i, float theValue)
    {
        if (i < mNumbers.size())
        {
            mNumbers[i]->setValue(theValue);
            mNumbers[i]->setIndex();

            emit valueChanged(i, QVariant(theValue));
            emit valueChanged(QVariant(theValue));

            setUniform();
        }
    }

    void setUniform()
    {
        mOperation->setMat4Uniform(mName, mType, values());
    }

    QList<float> values()
    {
        QList<float> theValues;
        foreach (Number<float>* number, mNumbers)
            theValues.append(number->value());
        return theValues;
    }

    QList<Number<float>*> numbers()
    {
        return mNumbers;
    }

    Number<float>* number(QUuid theId)
    {
        foreach (Number<float>* number, mNumbers)
            if (theId == number->id())
                return number;
        return nullptr;
    }

    Number<float>* number(int i)
    {
        if (i < mNumbers.size())
            return mNumbers[i];
        else
            return new Number<float>(0, 0, 0, 0, 0);
    }

    QList<QString> numberNames() { return mNumberNames; }

private:
    UniformMat4Type mType;
    QList<Number<float>*> mNumbers;
    QList<QString> mNumberNames;
};



#endif // PARAMETER_H
