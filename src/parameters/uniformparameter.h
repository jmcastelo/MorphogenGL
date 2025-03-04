#ifndef UNIFORMPARAMETER_H
#define UNIFORMPARAMETER_H



#include "parameter.h"
#include "number.h"



template <typename T>
class UniformParameter : public Parameter
{
public:
    UniformParameter(QString theName, QString theUniformName, int theUniformType, int numItems, bool isEditable, QList<T> theValues, T theMin, T theMax, T theInf, T theSup, ImageOperation* theOperation);

    UniformParameter(QString theName, QString theUniformName, int theUniformType, int numItems, bool isEditable, QList<QPair<QUuid, T>> theIdValuePairs, T theMin, T theMax, T theInf, T theSup, ImageOperation* theOperation);

    UniformParameter(const UniformParameter<T>& parameter);

    ~UniformParameter();

    T value(int i);
    void setValue(int i, T theValue);

    void setValueFromIndex(int i, int index);

    void setUniform();

    void setMin(T theMin);
    void setMax(T theMax);

    void setInf(T theInf);
    void setSup(T theSup);

    QList<T> values();

    QList<Number<T>*> numbers();

    Number<T>* number(QUuid theId);
    Number<T>* number(int i);

    int size();

    int numItems();

    QPair<int, int> colsRowsPerItem();

private:
    QList<Number<T>*> mNumbers;
    int nItems;
};



#endif // UNIFORMPARAMETER_H
