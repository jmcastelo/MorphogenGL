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

    Number(const Number<T>& number)
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



class Parameter : public ParameterSignals
{
public:
    Parameter(QString theName, QString theUniformName, QString theUniformType, bool isEditable) :
        mName { theName },
        mUniformName { theUniformName },
        mUniformType { theUniformType },
        mEditable { isEditable }
    {}

    Parameter(const Parameter& parameter)
    {
        mName = parameter.mName;
        mUniformName = parameter.mUniformName;
        mUniformType = parameter.mUniformType;
        mEditable = parameter.mEditable;
    }

    QString name() { return mName; }
    QString uniformName() { return mUniformName; }
    QString uniformType() { return mUniformType; }
    bool editable() { return mEditable; }

    void setOperation(ImageOperation* op) { mOperation = op; }

protected:
    QString mName;
    QString mUniformName;
    QString mUniformType;
    bool mEditable;
    ImageOperation* mOperation;
};



template <typename T>
class OptionsParameter : public Parameter
{
public:
    OptionsParameter(QString theName, QString theUniformName, QString theUniformType, bool isEditable, QList<QString> theValueNames, QList<T> theValues, T theValue) :
        Parameter(theName, theUniformName, theUniformType, isEditable),
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



/*template <typename T>
class NumberParameter : public Parameter
{
public:
    NumberParameter(QString theName, QString theUniformName, QString theUniformType, bool isEditable, T theValue, T theMin, T theMax, T theInf, T theSup) :
        Parameter(theName, theUniformName, theUniformType, isEditable)
    {
        mNumber = new Number<T>(theValue, theMin, theMax, theInf, theSup);
    }

    NumberParameter(QString theName, QString theUniformName, QString theUniformType, QUuid theId, T theValue, T theMin, T theMax, T theInf, T theSup) :
        Parameter(theName, theUniformName, theUniformType, isEditable)
    {
        mNumber = new Number<T>(theId, theValue, theMin, theMax, theInf, theSup);
    }

    NumberParameter(const NumberParameter<T>& parameter) :
        Parameter(parameter)
    {
        mNumber = new Number<T>(*parameter->number);
    }

    ~NumberParameter()
    {
        delete mNumber;
    }

    void setValue(T theValue)
    {
        mNumber->setValue(theValue);
        mNumber->setIndex();
        mOperation->setNumberParameter<T>(this);
        emit valueChanged(theValue);
    }

    T value() { return mNumber->value; }

    T min(){ return mNumber->min(); }
    T max(){ return mNumber->max(); }

    T inf(){ return mNumber->inf(); }
    T sup(){ return mNumber->sup(); }

    Number<T>* number(QUuid theId)
    {
        if (theId == mNumber->id())
            return mNumber;
        return nullptr;
    }

private:
    Number<T>* mNumber;
};*/



template <typename T>
class UniformParameter : public Parameter
{
public:
    UniformParameter(QString theName, QString theUniformName, QString theUniformType, int numItems, bool isArray, bool isEditable, QList<T> theValues, T theMin, T theMax, T theInf, T theSup) :
        Parameter(theName, theUniformName, theUniformType, isEditable),
        nItems { numItems },
        mArray { isArray }
    {
        for (T value : theValues)
        {
            Number<T>* number = new Number<T>(value, theMin, theMax, theInf, theSup);
            mNumbers.append(number);
        }
    }

    UniformParameter(QString theName, QString theUniformName, QString theUniformType, bool isEditable, QList<QPair<QUuid, T>> theIdValuePairs, T theMin, T theMax, T theInf, T theSup) :
        Parameter(theName, theUniformName, theUniformType, isEditable)
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
            mNumbers.push_back(number);
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

            emit valueChanged(i, theValue);

            setUniform();
        }
    }

    void setUniform()
    {
        if (!mArray)
        {
            if (mUniformType == "float" || mUniformType == "int")
                mOperation->setUniform1<T>(mName, toValue());
            else if (mUniformType == "vec2" || mUniformType == "ivec2")
                mOperation->setUniform2<T>(mName, toVector2D());

        }
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

    T toValue()
    {
        if (mNumbers.size() > 0)
            return mNumbers.at(0)->value();
        else
            return 0;
    }

    QVector2D toVector2D()
    {
        if (mNumbers.size() > 1)
            return QVector2D(mNumbers.at(0)->value(), mNumbers.at(1)->value());
        else if (mNumbers.size() > 0)
            return QVector2D(mNumbers.at(0)->value(), 0.0);
        else
            return QVector2D(0.0, 0.0);
    }

    QVector3D toVector3D()
    {
        if (mNumbers.size() > 2)
            return QVector3D(mNumbers.at(0)->value(), mNumbers.at(1)->value(), mNumbers.at(2)->value());
        else if (mNumbers.size() > 1)
            return QVector3D(mNumbers.at(0)->value(), mNumbers.at(1)->value(), 0.0);
        else if (mNumbers.size() > 0)
            return QVector3D(mNumbers.at(0)->value(), 0.0, 0.0);
        else
            return QVector3D(0.0, 0.0, 0.0);
    }

    QVector4D toVector4D()
    {
        if (mNumbers.size() > 3)
            return QVector4D(mNumbers.at(0)->value(), mNumbers.at(1)->value(), mNumbers.at(2)->value()), mNumbers.at(3)->value();
        else if (mNumbers.size() > 2)
            return QVector4D(mNumbers.at(0)->value(), mNumbers.at(1)->value(), mNumbers.at(2)->value(), 0.0);
        else if (mNumbers.size() > 1)
            return QVector4D(mNumbers.at(0)->value(), mNumbers.at(1)->value(), 0.0, 0.0);
        else if (mNumbers.size() > 0)
            return QVector4D(mNumbers.at(0)->value(), 0.0, 0.0, 0.0);
        else
            return QVector4D(0.0, 0.0, 0.0, 0.0);
    }

    QMatrix2x2 toMatrix2x2()
    {
        QMatrix2x2 matrix;

        int row = 0, col = 0;
        for (auto number : mNumbers)
        {
            matrix(row, col++) = number->value();
            if (col >= 2) { row++; col = 0; }
        }

        return matrix;
    }

    QMatrix3x3 toMatrix3x3()
    {
        QMatrix3x3 matrix;

        int row = 0, col = 0;
        for (auto number : mNumbers)
        {
            matrix(row, col++) = number->value();
            if (col >= 3) { row++; col = 0; }
        }

        return matrix;
    }

    QMatrix4x4 toMatrix4x4()
    {
        QMatrix4x4 matrix;

        int row = 0, col = 0;
        for (auto number : mNumbers)
        {
            matrix(row, col++) = number->value();
            if (col >= 4) { row++; col = 0; }
        }

        return matrix;
    }

private:
    QList<Number<T>*> mNumbers;
    bool mArray;
    int nCols, nRows, nItems;
};



#endif // PARAMETER_H
