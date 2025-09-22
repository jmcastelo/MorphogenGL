


#include "baseuniformparameter.h"
#include "../imageoperation.h"



template <typename T>
BaseUniformParameter<T>::BaseUniformParameter(QString theName, QString theUniformName, int theUniformType, bool isEditable, ImageOperation* theOperation) :
    Parameter(theName, isEditable, theOperation),
    mUniformName { theUniformName },
    mUniformType { theUniformType }
{}



template <typename T>
BaseUniformParameter<T>::BaseUniformParameter(QString theName, QString theUniformName, int theUniformType, bool isEditable, QList<T> theValues, QList<T> theMin, QList<T> theMax, QList<T> theInf, QList<T> theSup, ImageOperation* theOperation) :
    Parameter(theName, isEditable, theOperation),
    mUniformName { theUniformName },
    mUniformType { theUniformType }
{
    for (int i = 0; i < theValues.size(); i++)
    {
        Number<T>* number = new Number<T>(theValues.at(i), theMin.at(i), theMax.at(i), theInf.at(i), theSup.at(i));
        mNumbers.append(number);
    }

    for (int i = 0; i < mNumbers.size(); i++)
        connect(mNumbers[i], &NumberSignals::valueChanged, this, [=, this](QVariant value){ emit valueChanged(i, value); });
}



template <typename T>
BaseUniformParameter<T>::BaseUniformParameter(QString theName, QString theUniformName, int theUniformType, bool isEditable, QList<QUuid> theIds, QList<T> theValues, QList<T> theMin, QList<T> theMax, QList<T> theInf, QList<T> theSup, ImageOperation* theOperation) :
    Parameter(theName, isEditable, theOperation),
    mUniformName { theUniformName },
    mUniformType { theUniformType }
{
    for (int i = 0; i < theValues.size(); i++)
    {
        Number<T>* number = new Number<T>(theIds.at(i), theValues.at(i), theMin.at(i), theMax.at(i), theInf.at(i), theSup.at(i));
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
QString BaseUniformParameter<T>::uniformName() const
{
    return mUniformName;
}



template <typename T>
int BaseUniformParameter<T>::uniformType() const
{
    return mUniformType;
}



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

        setUniform();
    }
}



template <typename T>
void BaseUniformParameter<T>::setValues(QList<T> values)
{
    if (values.size() == mNumbers.size())
    {
        for (int i = 0; i < mNumbers.size(); i++)
            mNumbers[i]->setValue(values[i]);

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
    foreach (Number<T>* number, mNumbers)
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



template <typename T>
QList<QString> BaseUniformParameter<T>::presetNames()
{
    return mPresets.keys();
}



template <typename T>
QMap<QString, QList<T>> BaseUniformParameter<T>::presets()
{
    return mPresets;
}



template <typename T>
void BaseUniformParameter<T>::setPresets(QMap<QString, QList<T>> thePresets)
{
    mPresets = thePresets;
}



template <typename T>
void BaseUniformParameter<T>::addPreset(QString name)
{
    if (mPresets.contains(name))
        mPresets.remove(name);

    mPresets.insert(name, values());
}



template <typename T>
void BaseUniformParameter<T>::removePreset(QString name)
{
    if (mPresets.contains(name))
        mPresets.remove(name);
}



template <typename T>
void BaseUniformParameter<T>::setPreset(QString name)
{
    if (mPresets.contains(name))
    {
        for (int i = 0; i < mNumbers.size(); i++)
        {
            mNumbers[i]->setValue(mPresets[name][i]);
            emit valueChanged(i, QVariant(mNumbers[i]->value()));
        }

        setUniform();
    }
}



template class BaseUniformParameter<float>;
template class BaseUniformParameter<int>;
template class BaseUniformParameter<unsigned int>;
