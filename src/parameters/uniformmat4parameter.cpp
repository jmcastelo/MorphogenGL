


#include "uniformmat4parameter.h"
#include "../imageoperation.h"



UniformMat4Parameter::UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, QList<float> theValues, float theMin, float theMax, float theInf, float theSup, ImageOperation* theOperation) :
    Parameter(theName, theUniformName, GL_FLOAT_MAT4, isEditable, theOperation),
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

    for (int i = 0; i < mNumbers.size(); i++)
        connect(mNumbers[i], &NumberSignals::valueChanged, this, [=, this](QVariant value){ emit valueChanged(i, value); });
}



UniformMat4Parameter::UniformMat4Parameter(QString theName, QString theUniformName, bool isEditable, UniformMat4Type theMat4Type, QList<QPair<QUuid, float>> theIdValuePairs, float theMin, float theMax, float theInf, float theSup, ImageOperation* theOperation) :
    Parameter(theName, theUniformName, GL_FLOAT_MAT4, isEditable, theOperation),
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

    for (int i = 0; i < mNumbers.size(); i++)
        connect(mNumbers[i], &NumberSignals::valueChanged, this, [=, this](QVariant value){ emit valueChanged(i, value); });
}



UniformMat4Parameter::UniformMat4Parameter(const UniformMat4Parameter& parameter) :
    Parameter(parameter)
{
    mNumberNames = parameter.mNumberNames;

    for (const Number<float>* number : parameter.mNumbers)
    {
        Number<float>* newNumber = new Number<float>(*number);
        mNumbers.push_back(newNumber);
    }

    for (int i = 0; i < mNumbers.size(); i++)
        connect(mNumbers[i], &NumberSignals::valueChanged, this, [=, this](QVariant value){ emit valueChanged(i, value); });
}



UniformMat4Parameter::~UniformMat4Parameter()
{
    qDeleteAll(mNumbers);
}



float UniformMat4Parameter::value(int i)
{
    if (i < mNumbers.size())
        return mNumbers.at(i)->value();
    else
        return 0;
}



void UniformMat4Parameter::setValue(int i, float theValue)
{
    if (i < mNumbers.size())
    {
        mNumbers[i]->setValue(theValue);

        //emit valueChanged(i, QVariant(theValue));
        emit valueChanged(QVariant(theValue));

        if (mUpdate)
            setUniform();
    }
}



void UniformMat4Parameter::setValueFromIndex(int i, int index)
{
    if (i < mNumbers.size())
    {
        mNumbers[i]->setValueFromIndex(index);

        emit valueChanged(i, QVariant(mNumbers[i]->value()));

        if (mUpdate)
            setUniform();
    }
}



void UniformMat4Parameter::setUniform()
{
    mOperation->setMat4Uniform(mName, mType, values());
}



void UniformMat4Parameter::setMin(float theMin)
{
    foreach (Number<float>* number, mNumbers)
    number->setMin(theMin);
}



void UniformMat4Parameter::setMax(float theMax)
{
    foreach (Number<float>* number, mNumbers)
    number->setMax(theMax);
}



void UniformMat4Parameter::setInf(float theInf)
{
    foreach (Number<float>* number, mNumbers)
    number->setInf(theInf);
}



void UniformMat4Parameter::setSup(float theSup)
{
    foreach (Number<float>* number, mNumbers)
    number->setSup(theSup);
}



QList<float> UniformMat4Parameter::values()
{
    QList<float> theValues;
    foreach (Number<float>* number, mNumbers)
        theValues.append(number->value());
    return theValues;
}



QList<Number<float>*> UniformMat4Parameter::numbers()
{
    return mNumbers;
}



Number<float>* UniformMat4Parameter::number(QUuid theId)
{
    foreach (Number<float>* number, mNumbers)
    if (theId == number->id())
        return number;
    return nullptr;
}



Number<float>* UniformMat4Parameter::number(int i)
{
    if (i < mNumbers.size())
        return mNumbers[i];
    else
        return new Number<float>(0, 0, 0, 0, 0);
}



QList<QString> UniformMat4Parameter::numberNames()
{
    return mNumberNames;
}
