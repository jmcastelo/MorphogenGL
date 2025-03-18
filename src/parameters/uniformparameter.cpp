


#include "uniformparameter.h"
#include "uniformmat4parameter.h"
#include "../imageoperation.h"

#include <QPair>
#include <QOpenGLFunctions>



template <typename T>
UniformParameter<T>::UniformParameter(QString theName, QString theUniformName, int theUniformType, int numItems, bool isEditable, ImageOperation* theOperation) :
    BaseUniformParameter<T>(theName, theUniformName, theUniformType, isEditable, theOperation),
    nItems { numItems }
{
    // Determine number of values per item

    int numValuesPerItem = 0;

    if (BaseUniformParameter<T>::mUniformType == GL_FLOAT || BaseUniformParameter<T>::mUniformType == GL_INT || BaseUniformParameter<T>::mUniformType == GL_UNSIGNED_INT)
        numValuesPerItem = 1;
    else if (BaseUniformParameter<T>::mUniformType == GL_FLOAT_VEC2 || BaseUniformParameter<T>::mUniformType == GL_INT_VEC2 || BaseUniformParameter<T>::mUniformType == GL_UNSIGNED_INT_VEC2)
        numValuesPerItem = 2;
    else if (BaseUniformParameter<T>::mUniformType == GL_FLOAT_VEC3 || BaseUniformParameter<T>::mUniformType == GL_INT_VEC3 || BaseUniformParameter<T>::mUniformType == GL_UNSIGNED_INT_VEC3)
        numValuesPerItem = 3;
    else if (BaseUniformParameter<T>::mUniformType == GL_FLOAT_VEC4 || BaseUniformParameter<T>::mUniformType == GL_INT_VEC4 || BaseUniformParameter<T>::mUniformType == GL_UNSIGNED_INT_VEC4 || BaseUniformParameter<T>::mUniformType == GL_FLOAT_MAT2)
        numValuesPerItem = 4;
    else if (BaseUniformParameter<T>::mUniformType == GL_FLOAT_MAT3)
        numValuesPerItem = 9;
    else if (BaseUniformParameter<T>::mUniformType == GL_FLOAT_MAT4)
        numValuesPerItem = 16;

    // Set values and numbers with default values

    QList<T> theValues = QList<T>(nItems * numValuesPerItem, 0);

    for (T value : theValues)
    {
        Number<T>* number = new Number<T>(value, 0, 1, 0, 1);
        BaseUniformParameter<T>::mNumbers.append(number);
    }

    BaseUniformParameter<T>::mEmpty = theValues.empty();

    // Connections

    for (int i = 0; i < BaseUniformParameter<T>::mNumbers.size(); i++)
        BaseUniformParameter<T>::connect(BaseUniformParameter<T>::mNumbers[i], &NumberSignals::valueChanged, this, [=, this](QVariant value){ emit BaseUniformParameter<T>::valueChanged(i, value); });
}



template <typename T>
UniformParameter<T>::UniformParameter(const UniformMat4Parameter& parameter) :
    UniformParameter<T>(parameter.name(), parameter.uniformName(), parameter.uniformType(), 1, parameter.editable(), parameter.operation())
{
    BaseUniformParameter<T>::mRow = parameter.row();
    BaseUniformParameter<T>::mCol = parameter.col();
}



template <typename T>
UniformParameter<T>::UniformParameter(QString theName, QString theUniformName, int theUniformType, int numItems, bool isEditable, QList<T> theValues, T theMin, T theMax, T theInf, T theSup, ImageOperation* theOperation) :
    BaseUniformParameter<T>(theName, theUniformName, theUniformType, isEditable, theValues, theMin, theMax, theInf, theSup, theOperation),
    nItems { numItems }
{}



template <typename T>
UniformParameter<T>::UniformParameter(QString theName, QString theUniformName, int theUniformType, int numItems, bool isEditable, QList<QPair<QUuid, T>> theIdValuePairs, T theMin, T theMax, T theInf, T theSup, ImageOperation* theOperation) :
    BaseUniformParameter<T>(theName, theUniformName, theUniformType, isEditable, theIdValuePairs, theMin, theMax, theInf, theSup, theOperation),
    nItems { numItems }
{}



template <typename T>
UniformParameter<T>::UniformParameter(const UniformParameter<T>& parameter) :
    BaseUniformParameter<T>(parameter),
    nItems { parameter.nItems }
{}



template <typename T>
void UniformParameter<T>::setUniform()
{
    BaseUniformParameter<T>::mOperation->setUniform(BaseUniformParameter<T>::mName, BaseUniformParameter<T>::mUniformType, nItems, BaseUniformParameter<T>::values().constData());
}



template <typename T>
int UniformParameter<T>::size()
{
    return BaseUniformParameter<T>::mNumbers.size();
}



template <typename T>
int UniformParameter<T>::numItems()
{
    return nItems;
}



template <typename T>
QPair<int, int> UniformParameter<T>::colsRowsPerItem()
{
    if (BaseUniformParameter<T>::mUniformType == GL_FLOAT || BaseUniformParameter<T>::mUniformType == GL_INT || BaseUniformParameter<T>::mUniformType == GL_UNSIGNED_INT)
        return QPair<int, int>(1, 1);
    else if (BaseUniformParameter<T>::mUniformType == GL_FLOAT_VEC2 || BaseUniformParameter<T>::mUniformType == GL_INT_VEC2 || BaseUniformParameter<T>::mUniformType == GL_UNSIGNED_INT_VEC2)
        return QPair<int, int>(2, 1);
    else if (BaseUniformParameter<T>::mUniformType == GL_FLOAT_VEC3 || BaseUniformParameter<T>::mUniformType == GL_INT_VEC3 || BaseUniformParameter<T>::mUniformType == GL_UNSIGNED_INT_VEC3)
        return QPair<int, int>(3, 1);
    else if (BaseUniformParameter<T>::mUniformType == GL_FLOAT_VEC4 || BaseUniformParameter<T>::mUniformType == GL_INT_VEC4 || BaseUniformParameter<T>::mUniformType == GL_UNSIGNED_INT_VEC4)
        return QPair<int, int>(4, 1);
    else if (BaseUniformParameter<T>::mUniformType == GL_FLOAT_MAT2)
        return QPair<int, int>(2, 2);
    else if (BaseUniformParameter<T>::mUniformType == GL_FLOAT_MAT3)
        return QPair<int, int>(3, 3);
    else if (BaseUniformParameter<T>::mUniformType == GL_FLOAT_MAT4)
        return QPair<int, int>(4, 4);

    return QPair<int, int>(0, 0);
}



template <typename T>
bool UniformParameter<T>::isMat4Equivalent()
{
    return (nItems == 1) && (BaseUniformParameter<T>::mUniformType == GL_FLOAT_MAT4);
}



template class UniformParameter<float>;
template class UniformParameter<int>;
template class UniformParameter<unsigned int>;
