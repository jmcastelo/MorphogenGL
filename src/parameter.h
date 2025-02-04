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
    Parameter(QString theName, QString theUniformName) :
        mName { theName },
        mUniformName { theUniformName }
    {}

    QString name() { return mName; }
    QString uniformName() { return mUniformName; }

    void setOperation(ImageOperation* op) { mOperation = op; }

protected:
    QString mName;
    QString mUniformName;
    ImageOperation* mOperation;
};



template <typename T>
class OptionsParameter : public Parameter
{
public:
    OptionsParameter(QString theName, QString theUniformName, QList<QString> theValueNames, QList<T> theValues, T theValue) :
        Parameter(theName, theUniformName),
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
class NumberParameter : public Parameter
{
public:
    NumberParameter(QString theName, QString theUniformName, T theValue, T theMin, T theMax, T theInf, T theSup) :
        Parameter(theName, theUniformName)
    {
        mNumber = new Number<T>(theValue, theMin, theMax, theInf, theSup);
    }

    NumberParameter(QString theName, QString theUniformName, QUuid theId, T theValue, T theMin, T theMax, T theInf, T theSup) :
        Parameter(theName, theUniformName)
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
};



template <typename T>
class ArrayParameter : public Parameter
{
public:
    ArrayParameter(QString theName, QString theUniformName, QList<T> theValues, T theMin, T theMax, T theInf, T theSup) :
        Parameter(theName, theUniformName)
    {
        for (T value : theValues)
        {
            Number<T>* number = new Number<T>(value, theMin, theMax, theInf, theSup);
            mNumbers.append(number);
        }
    }

    ArrayParameter(QString theName, QString theUniformName, QList<QPair<QUuid, T>> theIdValuePairs, T theMin, T theMax, T theInf, T theSup) :
        Parameter(theName, theUniformName)
    {
        for (QPair<QUuid, T> pair : theIdValuePairs)
        {
            Number<T>* number = new Number<T>(pair.first. pair.second, theMin, theMax, theInf, theSup);
            mNumbers.append(number);
        }
    }

    ArrayParameter(const ArrayParameter<T>& parameter) :
        Parameter(parameter)
    {
        for (Number<T>* number : parameter.mNumbers)
        {
            Number<T>* newNumber = new Number<T>(*number);
            mNumbers.push_back(number);
        }
    }

    ~ArrayParameter()
    {
        qDeleteAll(mNumbers);
    }

    T value(int i)
    {
        return mNumbers.at(i)->value();
    }

    void setValue(int i, T theValue)
    {
        mNumbers[i]->setValue(theValue);
        mNumbers[i]->setIndex();
        mOperation->setArrayParameter<T>(this);
        emit valueChanged(i, theValue);
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
        return mNumbers[i];
    }

    int size()
    {
        return mNumbers.size();
    }

private:
    QList<Number<T>*> mNumbers;
};



#endif // PARAMETER_H
