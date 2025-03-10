#ifndef UNIFORMMAT4PARAMETER_H
#define UNIFORMMAT4PARAMETER_H



#include "baseuniformparameter.h"

#include "QOpenGLFunctions"



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
    UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, QList<float> theValues, float theMin, float theMax, float theInf, float theSup, ImageOperation* theOperation);

    UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, QList<QPair<QUuid, float>> theIdValuePairs, float theMin, float theMax, float theInf, float theSup, ImageOperation* theOperation);

    UniformMat4Parameter(const UniformMat4Parameter& parameter);

    void setUniform() override;

    QList<QString> numberNames();

private:
    UniformMat4Type mType;
    QList<QString> mNumberNames;
};



#endif // UNIFORMMAT4PARAMETER_H
