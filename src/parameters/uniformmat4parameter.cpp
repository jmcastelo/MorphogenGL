


#include "uniformmat4parameter.h"
#include "../imageoperation.h"



UniformMat4Parameter::UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, ImageOperation* theOperation) :
    BaseUniformParameter<float>(theName, theUniformName, GL_FLOAT_MAT4, isEditable, theOperation),
    mType { theMat4Type }
{
    setType(mType);
}



UniformMat4Parameter::UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, QList<float> theValues, QList<float> theMin, QList<float> theMax, QList<float> theInf, QList<float> theSup, ImageOperation* theOperation) :
    BaseUniformParameter<float>(theName, theUniformName, GL_FLOAT_MAT4, isEditable, theOperation),
    mType { theMat4Type }
{
    if (mType == UniformMat4Type::TRANSLATION)
    {
        mNumberNames.append("X");
        mNumbers.append(new Number<float>(theValues.at(0), theMin.at(0), theMax.at(0), theInf.at(0), theSup.at(0)));

        mNumberNames.append("Y");
        mNumbers.append(new Number<float>(theValues.at(1), theMin.at(1), theMax.at(1), theInf.at(1), theSup.at(1)));
    }
    else if (mType == UniformMat4Type::ROTATION)
    {
        mNumberNames.append("Angle");
        mNumbers.append(new Number<float>(theValues.at(0), theMin.at(0), theMax.at(0), theInf.at(0), theSup.at(0)));
    }
    else if (mType == UniformMat4Type::SCALING)
    {
        mNumberNames.append("X");
        mNumbers.append(new Number<float>(theValues.at(0), theMin.at(0), theMax.at(0), theInf.at(0), theSup.at(0)));

        mNumberNames.append("Y");
        mNumbers.append(new Number<float>(theValues.at(1), theMin.at(1), theMax.at(1), theInf.at(1), theSup.at(1)));
    }
}



UniformMat4Parameter::UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, QList<QUuid> theIds, QList<float> theValues, QList<float> theMin, QList<float> theMax, QList<float> theInf, QList<float> theSup, ImageOperation* theOperation) :
    BaseUniformParameter<float>(theName, theUniformName, GL_FLOAT_MAT4, isEditable, theOperation),
    mType { theMat4Type }
{
    if (mType == UniformMat4Type::TRANSLATION)
    {
        mNumberNames.append("X");
        mNumbers.append(new Number<float>(theIds.at(0), theValues.at(0), theMin.at(0), theMax.at(0), theInf.at(0), theSup.at(0)));

        mNumberNames.append("Y");
        mNumbers.append(new Number<float>(theIds.at(1), theValues.at(1), theMin.at(1), theMax.at(1), theInf.at(1), theSup.at(1)));
    }
    else if (mType == UniformMat4Type::ROTATION)
    {
        mNumberNames.append("Angle");
        mNumbers.append(new Number<float>(theIds.at(0), theValues.at(0), theMin.at(0), theMax.at(0), theInf.at(0), theSup.at(0)));
    }
    else if (mType == UniformMat4Type::SCALING)
    {
        mNumberNames.append("X");
        mNumbers.append(new Number<float>(theIds.at(0), theValues.at(0), theMin.at(0), theMax.at(0), theInf.at(0), theSup.at(0)));

        mNumberNames.append("Y");
        mNumbers.append(new Number<float>(theIds.at(1), theValues.at(1), theMin.at(1), theMax.at(1), theInf.at(1), theSup.at(1)));
    }
}



UniformMat4Parameter::UniformMat4Parameter(const UniformMat4Parameter& parameter) :
    BaseUniformParameter<float>(parameter)
{
    mType = parameter.mType;
    mNumberNames = parameter.mNumberNames;
}



UniformMat4Parameter::UniformMat4Parameter(const UniformParameter<float>& parameter, UniformMat4Type theMat4Type) :
    UniformMat4Parameter(parameter.name(), parameter.uniformName(), parameter.editable(), theMat4Type, parameter.operation())
{
    mRow = parameter.row();
    mCol = parameter.col();
}



void UniformMat4Parameter::setType(UniformMat4Type type)
{
    mType = type;

    mNumberNames.clear();

    qDeleteAll(mNumbers);
    mNumbers.clear();

    if (mType == UniformMat4Type::TRANSLATION)
    {
        mNumberNames.append("X");
        mNumbers.append(new Number<float>(0.0, -1.0, 1.0, -1.0, 1.0));

        mNumberNames.append("Y");
        mNumbers.append(new Number<float>(0.0, -1.0, 1.0, -1.0, 1.0));
    }
    else if (mType == UniformMat4Type::ROTATION)
    {
        mNumberNames.append("Angle");
        mNumbers.append(new Number<float>(0.0, -180.0, 180.0, -360.0, 360.0));
    }
    else if (mType == UniformMat4Type::SCALING)
    {
        mNumberNames.append("X");
        mNumbers.append(new Number<float>(1.0, 0.0, 2.0, 0.0, 1000.0));

        mNumberNames.append("Y");
        mNumbers.append(new Number<float>(1.0, 0.0, 2.0, 0.0, 1000.0));
    }

    mPresets.clear();

    mEmpty = mNumbers.empty();

    for (int i = 0; i < mNumbers.size(); i++)
        connect(mNumbers[i], &NumberSignals::valueChanged, this, [=, this](QVariant value){ emit valueChanged(i, value); });
}


void UniformMat4Parameter::setUniform()
{
    mOperation->setMat4Uniform(mUniformName, mType, values());
}



QList<QString> UniformMat4Parameter::numberNames()
{
    return mNumberNames;
}



int UniformMat4Parameter::typeIndex() const
{
    return static_cast<int>(mType);
}
