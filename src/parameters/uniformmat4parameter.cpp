


#include "uniformmat4parameter.h"
#include "../imageoperation.h"



UniformMat4Parameter::UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, QList<float> theValues, float theMin, float theMax, float theInf, float theSup, ImageOperation* theOperation) :
    BaseUniformParameter<float>(theName, theUniformName, GL_FLOAT_MAT4, isEditable, theValues, theMin, theMax, theInf, theSup, theOperation),
    mType { theMat4Type }
{
    if (mType == UniformMat4Type::TRANSLATION)
    {
        mNumberNames.append("X");
        mNumbers.append(new Number<float>(theValues.at(0), theMin, theMax, theInf, theSup));

        mNumberNames.append("Y");
        mNumbers.append(new Number<float>(theValues.at(1), theMin, theMax, theInf, theSup));
    }
    else if (mType == UniformMat4Type::ROTATION)
    {
        mNumberNames.append("Angle");
        mNumbers.append(new Number<float>(theValues.at(0), theMin, theMax, theInf, theSup));
    }
    else if (mType == UniformMat4Type::SCALING)
    {
        mNumberNames.append("X");
        mNumbers.append(new Number<float>(theValues.at(0), theMin, theMax, theInf, theSup));

        mNumberNames.append("Y");
        mNumbers.append(new Number<float>(theValues.at(1), theMin, theMax, theInf, theSup));
    }
    else if (mType == UniformMat4Type::ORTHOGRAPHIC)
    {
        mEditable = false;
        mOperation->setOrthographicProjection(mUniformName);
    }
}



UniformMat4Parameter::UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, QList<QPair<QUuid, float>> theIdValuePairs, float theMin, float theMax, float theInf, float theSup, ImageOperation* theOperation) :
    BaseUniformParameter<float>(theName, theUniformName, GL_FLOAT_MAT4, isEditable, theIdValuePairs, theMin, theMax, theInf, theSup, theOperation),
    mType { theMat4Type }
{
    if (mType == UniformMat4Type::TRANSLATION)
    {
        mNumberNames.append("X");
        mNumbers.append(new Number<float>(theIdValuePairs.at(0).first, theIdValuePairs.at(0).second, theMin, theMax, theInf, theSup));

        mNumberNames.append("Y");
        mNumbers.append(new Number<float>(theIdValuePairs.at(1).first, theIdValuePairs.at(1).second, theMin, theMax, theInf, theSup));
    }
    else if (mType == UniformMat4Type::ROTATION)
    {
        mNumberNames.append("Angle");
        mNumbers.append(new Number<float>(theIdValuePairs.at(0).first, theIdValuePairs.at(0).second, theMin, theMax, theInf, theSup));
    }
    else if (mType == UniformMat4Type::SCALING)
    {
        mNumberNames.append("X");
        mNumbers.append(new Number<float>(theIdValuePairs.at(0).first, theIdValuePairs.at(0).second, theMin, theMax, theInf, theSup));

        mNumberNames.append("Y");
        mNumbers.append(new Number<float>(theIdValuePairs.at(1).first, theIdValuePairs.at(1).second, theMin, theMax, theInf, theSup));
    }
    else if (mType == UniformMat4Type::ORTHOGRAPHIC)
    {
        mEditable = false;
        mOperation->setOrthographicProjection(mUniformName);
    }
}



UniformMat4Parameter::UniformMat4Parameter(const UniformMat4Parameter& parameter) :
    BaseUniformParameter<float>(parameter)
{
    mType = parameter.mType;
    mNumberNames = parameter.mNumberNames;
}



void UniformMat4Parameter::setUniform()
{
    mOperation->setMat4Uniform(mName, mType, values());
}



QList<QString> UniformMat4Parameter::numberNames()
{
    return mNumberNames;
}
