#ifndef UNIFORMMAT4PARAMETER_H
#define UNIFORMMAT4PARAMETER_H



#include "parameter.h"
#include "number.h"

#include "QOpenGLFunctions"



enum class UniformMat4Type
{
    TRANSLATION = 0,
    ROTATION = 1,
    SCALING = 2,
    ORTHOGRAPHIC = 3
};


class UniformMat4Parameter : public Parameter
{
public:
    UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, QList<float> theValues, float theMin, float theMax, float theInf, float theSup, ImageOperation* theOperation);

    UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, QList<QPair<QUuid, float>> theIdValuePairs, float theMin, float theMax, float theInf, float theSup, ImageOperation* theOperation);

    UniformMat4Parameter(const UniformMat4Parameter& parameter);

    ~UniformMat4Parameter();

    float value(int i);
    void setValue(int i, float theValue);
    void setValueFromIndex(int i, int index);

    void setUniform();

    void setMin(float theMin);
    void setMax(float theMax);

    void setInf(float theInf);

    void setSup(float theSup);

    QList<float> values();

    QList<Number<float>*> numbers();

    Number<float>* number(QUuid theId);

    Number<float>* number(int i);

    QList<QString> numberNames();

private:
    UniformMat4Type mType;
    QList<Number<float>*> mNumbers;
    QList<QString> mNumberNames;
};



#endif // UNIFORMMAT4PARAMETER_H
