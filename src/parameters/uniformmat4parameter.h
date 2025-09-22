#ifndef UNIFORMMAT4PARAMETER_H
#define UNIFORMMAT4PARAMETER_H



#include "baseuniformparameter.h"



template <typename T>
class UniformParameter;



enum class UniformMat4Type
{
    TRANSLATION = 0,
    ROTATION = 1,
    SCALING = 2,
    ORTHOGRAPHIC = 3
};


class UniformMat4Parameter : public BaseUniformParameter<float>
{
public:
    UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, ImageOperation* theOperation);

    UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, QList<float> theValues, QList<float> theMin, QList<float> theMax, QList<float> theInf, QList<float> theSup, ImageOperation* theOperation);

    UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, QList<QUuid> theIds, QList<float> theValues, QList<float> theMin, QList<float> theMax, QList<float> theInf, QList<float> theSup, ImageOperation* theOperation);

    UniformMat4Parameter(const UniformMat4Parameter& parameter);
    UniformMat4Parameter(const UniformParameter<float>& parameter, UniformMat4Type theMat4Type);

    void setType(UniformMat4Type type);
    UniformMat4Type type() const;
    int typeIndex() const;

    void setUniform() override;

    QList<QString> numberNames();

private:
    UniformMat4Type mType;
    QList<QString> mNumberNames;
};



#endif // UNIFORMMAT4PARAMETER_H
