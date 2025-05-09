#ifndef UNIFORMPARAMETER_H
#define UNIFORMPARAMETER_H



#include "baseuniformparameter.h"



class UniformMat4Parameter;



template <typename T>
class UniformParameter : public BaseUniformParameter<T>
{
public:
    UniformParameter(QString theName, QString theUniformName, int theUniformType, int numItems, bool isEditable, ImageOperation* theOperation);

    UniformParameter(QString theName, QString theUniformName, int theUniformType, int numItems, bool isEditable, QList<T> theValues, QList<T> theMin, QList<T> theMax, QList<T> theInf, QList<T> theSup, ImageOperation* theOperation);

    UniformParameter(QString theName, QString theUniformName, int theUniformType, int numItems, bool isEditable, QList<QUuid> theIds, QList<T> theValues, QList<T> theMin, QList<T> theMax, QList<T> theInf, QList<T> theSup, ImageOperation* theOperation);

    UniformParameter(const UniformParameter<T>& parameter);

    UniformParameter(const UniformMat4Parameter& parameter);

    void setUniform() override;

    int size();
    int numItems();
    QPair<int, int> colsRowsPerItem();

    bool isMat4Equivalent();

private:
    int nItems;
};



#endif // UNIFORMPARAMETER_H
