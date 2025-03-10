


#include "baseuniformparameter.h"
#include "../imageoperation.h"



template <typename T>
BaseUniformParameter<T>::BaseUniformParameter(QString theName, QString theUniformName, int theUniformType, bool isEditable, QList<T> theValues, T theMin, T theMax, T theInf, T theSup, ImageOperation* theOperation) :
    Parameter(theName, isEditable, theOperation),
    mUniformName { theUniformName },
    mUniformType { theUniformType }
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
BaseUniformParameter<T>::BaseUniformParameter(QString theName, QString theUniformName, int theUniformType, bool isEditable, QList<QPair<QUuid, T>> theIdValuePairs, T theMin, T theMax, T theInf, T theSup, ImageOperation* theOperation) :
    Parameter(theName, isEditable, theOperation),
    mUniformName { theUniformName },
    mUniformType { theUniformType }
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
BaseUniformParameter<T>::BaseUniformParameter(const BaseUniformParameter<T>& parameter) :
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
}



template <typename T>
BaseUniformParameter<T>::~BaseUniformParameter()
{
    qDeleteAll(mNumbers);
}


template <typename T>
QString BaseUniformParameter<T>::uniformName() { return mUniformName; }

template <typename T>
int BaseUniformParameter<T>::uniformType() { return mUniformType; }



template <typename T>
T BaseUniformParameter<T>::value(int i)
{
    if (i < mNumbers.size())
        return mNumbers.at(i)->value();
    else
        return 0;
}



template <typename T>
void BaseUniformParameter<T>::setValue(int i, T theValue)
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
void BaseUniformParameter<T>::setValueFromIndex(int i, int index)
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
void BaseUniformParameter<T>::setMin(T theMin)
{
    foreach (Number<T>* number, mNumbers)
        number->setMin(theMin);
}



template <typename T>
void BaseUniformParameter<T>::setMax(T theMax)
{
    foreach (Number<T>* number, mNumbers)
        number->setMax(theMax);
}



template <typename T>
void BaseUniformParameter<T>::setInf(T theInf)
{
    foreach (Number<T>* number, mNumbers)
        number->setInf(theInf);
}



template <typename T>
void BaseUniformParameter<T>::setSup(T theSup)
{
    foreach (Number<T>* number, mNumbers)
        number->setSup(theSup);
}



template <typename T>
QList<T> BaseUniformParameter<T>::values()
{
    QList<T> theValues;
    for (Number<T>* number : mNumbers)
        theValues.append(number->value());
    return theValues;
}



template <typename T>
QList<Number<T>*> BaseUniformParameter<T>::numbers()
{
    return mNumbers;
}



template <typename T>
Number<T>* BaseUniformParameter<T>::number(QUuid theId)
{
    foreach (Number<T>* number, mNumbers)
        if (theId == number->id())
            return number;
    return nullptr;
}



template <typename T>
Number<T>* BaseUniformParameter<T>::number(int i)
{
    if (i < mNumbers.size())
        return mNumbers[i];
    else
        return new Number<T>(0, 0, 0, 0, 0);
}



template class BaseUniformParameter<float>;
template class BaseUniformParameter<int>;
template class BaseUniformParameter<unsigned int>;
