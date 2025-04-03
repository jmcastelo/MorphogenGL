


#include "optionsparameter.h"
#include "../imageoperation.h"



template <typename T>
OptionsParameter<T>::OptionsParameter(QString theName, bool isEditable, QList<QString> theValueNames, QList<T> theValues, T theValue, ImageOperation* theOperation) :
    Parameter(theName, isEditable, theOperation),
    mValueNames { theValueNames },
    mValues { theValues },
    mCurrentValue { theValue }
{}



template <typename T>
OptionsParameter<T>::OptionsParameter(const OptionsParameter<T>& parameter) :
    Parameter(parameter)
{
    mValueNames = parameter.mValueNames;
    mValues = parameter.mValues;
    mCurrentValue = parameter.mCurrentValue;
}



template <typename T>
void OptionsParameter<T>::setValue(int valueIndex)
{
    mCurrentValue = mValues[valueIndex];
    mOperation->setOptionsParameter<T>(this);
}



template <typename T>
void OptionsParameter<T>::setValue()
{
    mOperation->setOptionsParameter<T>(this);
}



template <typename T>
T OptionsParameter<T>::value(){ return mCurrentValue; }



template <typename T>
int OptionsParameter<T>::indexOf()
{
    return mValues.indexOf(mCurrentValue);
}



template <typename T>
QList<QString> OptionsParameter<T>::valueNames()
{
    return mValueNames;
}



template <typename T>
QList<T> OptionsParameter<T>::values()
{
    return mValues;
}



template class OptionsParameter<GLenum>;
