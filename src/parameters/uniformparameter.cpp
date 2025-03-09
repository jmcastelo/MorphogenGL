


#include "uniformparameter.h"
#include "../imageoperation.h"

#include <QPair>
#include <QOpenGLFunctions>



template <typename T>
UniformParameter<T>::UniformParameter(QString theName, QString theUniformName, int theUniformType, int numItems, bool isEditable, QList<T> theValues, T theMin, T theMax, T theInf, T theSup, ImageOperation* theOperation) :
    Parameter(theName, isEditable, theOperation),
    mUniformName { theUniformName },
    mUniformType { theUniformType },
    nItems { numItems }
{
    for (T value : theValues)
    {
        Number<T>* number = new Number<T>(value, theMin, theMax, theInf, theSup);
        mNumbers.append(number);
    }

    for (int i = 0; i < mNumbers.size(); i++)
        connect(mNumbers[i], &NumberSignals::valueChanged, this, [=, this](QVariant value){ emit valueChanged(i, value); });
}



template <typename T>
UniformParameter<T>::UniformParameter(QString theName, QString theUniformName, int theUniformType, int numItems, bool isEditable, QList<QPair<QUuid, T>> theIdValuePairs, T theMin, T theMax, T theInf, T theSup, ImageOperation* theOperation) :
    Parameter(theName, isEditable, theOperation),
    mUniformName { theUniformName },
    mUniformType { theUniformType },
    nItems { numItems }
{
    foreach (auto pair, theIdValuePairs)
    {
        Number<T>* number = new Number<T>(pair.first, pair.second, theMin, theMax, theInf, theSup);
        mNumbers.append(number);
    }

    for (int i = 0; i < mNumbers.size(); i++)
        connect(mNumbers[i], &NumberSignals::valueChanged, this, [=, this](QVariant value){ emit valueChanged(i, value); });
}



template <typename T>
UniformParameter<T>::UniformParameter(const UniformParameter<T>& parameter) :
    Parameter(parameter)
{
    foreach (Number<T>* number, parameter.mNumbers)
    {
        Number<T>* newNumber = new Number<T>(*number);
        mNumbers.append(newNumber);
    }

    for (int i = 0; i < mNumbers.size(); i++)
        connect(mNumbers[i], &NumberSignals::valueChanged, this, [=, this](QVariant value){ emit valueChanged(i, value); });

    mUniformName = parameter.mUniformName;
    mUniformType = parameter.mUniformType;
    nItems = parameter.nItems;
}



template <typename T>
UniformParameter<T>::~UniformParameter()
{
    qDeleteAll(mNumbers);
}


template <typename T>
QString UniformParameter<T>::uniformName() { return mUniformName; }

template <typename T>
int UniformParameter<T>::uniformType() { return mUniformType; }



template <typename T>
T UniformParameter<T>::value(int i)
{
    if (i < mNumbers.size())
        return mNumbers.at(i)->value();
    else
        return 0;
}



template <typename T>
void UniformParameter<T>::setValue(int i, T theValue)
{
    if (i < mNumbers.size())
    {
        mNumbers[i]->setValue(theValue);

        //emit valueChanged(i, QVariant(theValue));
        emit valueChanged(QVariant(theValue));

        if (mUpdate)
            setUniform();
    }
}



template <typename T>
void UniformParameter<T>::setValueFromIndex(int i, int index)
{
    if (i < mNumbers.size())
    {
        mNumbers[i]->setValueFromIndex(index);

        emit valueChanged(i, QVariant(mNumbers[i]->value()));

        if (mUpdate)
            setUniform();
    }
}



template <typename T>
void UniformParameter<T>::setUniform()
{
    mOperation->setUniform<T>(mName, mUniformType, nItems, values().constData());
}



template <typename T>
void UniformParameter<T>::setMin(T theMin)
{
    foreach (Number<T>* number, mNumbers)
    number->setMin(theMin);
}



template <typename T>
void UniformParameter<T>::setMax(T theMax)
{
    foreach (Number<T>* number, mNumbers)
    number->setMax(theMax);
}



template <typename T>
void UniformParameter<T>::setInf(T theInf)
{
    foreach (Number<T>* number, mNumbers)
    number->setInf(theInf);
}



template <typename T>
void UniformParameter<T>::setSup(T theSup)
{
    foreach (Number<T>* number, mNumbers)
    number->setSup(theSup);
}



template <typename T>
QList<T> UniformParameter<T>::values()
{
    QList<T> theValues;
    for (Number<T>* number : mNumbers)
        theValues.append(number->value());
    return theValues;
}



template <typename T>
QList<Number<T>*> UniformParameter<T>::numbers()
{
    return mNumbers;
}



template <typename T>
Number<T>* UniformParameter<T>::number(QUuid theId)
{
    foreach (Number<T>* number, mNumbers)
    if (theId == number->id())
        return number;
    return nullptr;
}



template <typename T>
Number<T>* UniformParameter<T>::number(int i)
{
    if (i < mNumbers.size())
        return mNumbers[i];
    else
        return new Number<T>(0, 0, 0, 0, 0);
}



template <typename T>
int UniformParameter<T>::size()
{
    return mNumbers.size();
}



template <typename T>
int UniformParameter<T>::numItems()
{
    return nItems;
}



template <typename T>
QPair<int, int> UniformParameter<T>::colsRowsPerItem()
{
    if (mUniformType == GL_FLOAT || mUniformType == GL_INT || mUniformType == GL_UNSIGNED_INT)
        return QPair<int, int>(1, 1);
    else if (mUniformType == GL_FLOAT_VEC2 || mUniformType == GL_INT_VEC2 || mUniformType == GL_UNSIGNED_INT_VEC2)
        return QPair<int, int>(2, 1);
    else if (mUniformType == GL_FLOAT_VEC3 || mUniformType == GL_INT_VEC3 || mUniformType == GL_UNSIGNED_INT_VEC3)
        return QPair<int, int>(3, 1);
    else if (mUniformType == GL_FLOAT_VEC4 || mUniformType == GL_INT_VEC4 || mUniformType == GL_UNSIGNED_INT_VEC4)
        return QPair<int, int>(4, 1);
    else if (mUniformType == GL_FLOAT_MAT2)
        return QPair<int, int>(2, 2);
    else if (mUniformType == GL_FLOAT_MAT3)
        return QPair<int, int>(3, 3);
    else if (mUniformType == GL_FLOAT_MAT4)
        return QPair<int, int>(4, 4);

    return QPair<int, int>(0, 0);
}



template class UniformParameter<float>;
template class UniformParameter<int>;
template class UniformParameter<unsigned int>;
