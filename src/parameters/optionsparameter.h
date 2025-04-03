#ifndef OPTIONSPARAMETER_H
#define OPTIONSPARAMETER_H



#include "parameter.h"



template <typename T>
class OptionsParameter : public Parameter
{
public:
    OptionsParameter(QString theName, bool isEditable, QList<QString> theValueNames, QList<T> theValues, T theValue, ImageOperation* theOperation);

    OptionsParameter(const OptionsParameter<T>& parameter);

    void setValue(int valueIndex);
    void setValue();

    T value();

    int indexOf();

    QList<QString> valueNames();
    QList<T> values();

private:
    QList<QString> mValueNames;
    QList<T> mValues;
    T mCurrentValue;
};



#endif // OPTIONSPARAMETER_H
