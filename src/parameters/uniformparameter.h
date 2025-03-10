#ifndef UNIFORMPARAMETER_H
#define UNIFORMPARAMETER_H



#include "baseuniformparameter.h"



template <typename T>
class UniformParameter : public BaseUniformParameter<T>
{
public:
    UniformParameter(QString theName, QString theUniformName, int theUniformType, int numItems, bool isEditable, QList<T> theValues, T theMin, T theMax, T theInf, T theSup, ImageOperation* theOperation);

    UniformParameter(QString theName, QString theUniformName, int theUniformType, int numItems, bool isEditable, QList<QPair<QUuid, T>> theIdValuePairs, T theMin, T theMax, T theInf, T theSup, ImageOperation* theOperation);

    UniformParameter(const UniformParameter<T>& parameter);

    void setUniform() override;

    int size();

    int numItems();

    QPair<int, int> colsRowsPerItem();

private:
    int nItems;
};



#endif // UNIFORMPARAMETER_H
