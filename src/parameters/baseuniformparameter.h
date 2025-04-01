#ifndef BASEUNIFORMPARAMETER_H
#define BASEUNIFORMPARAMETER_H



#include "parameter.h"
#include "number.h"



template <typename T>
class BaseUniformParameter : public Parameter
{
public:
    BaseUniformParameter(QString theName, QString theUniformName, int theUniformType, bool isEditable, ImageOperation* theOperation);

    BaseUniformParameter(QString theName, QString theUniformName, int theUniformType, bool isEditable, QList<T> theValues, T theMin, T theMax, T theInf, T theSup, ImageOperation* theOperation);

    BaseUniformParameter(QString theName, QString theUniformName, int theUniformType, bool isEditable, QList<QPair<QUuid, T>> theIdValuePairs, T theMin, T theMax, T theInf, T theSup, ImageOperation* theOperation);

    BaseUniformParameter(const BaseUniformParameter<T>& parameter);

    ~BaseUniformParameter();

    QString uniformName() const;
    int uniformType() const;

    T value(int i);
    void setValue(int i, T theValue);
    void setValueFromIndex(int i, int index);

    virtual void setUniform() = 0;

    void setMin(T theMin);
    void setMax(T theMax);

    void setInf(T theInf);
    void setSup(T theSup);

    QList<T> values();

    QList<Number<T>*> numbers();

    Number<T>* number(QUuid theId);
    Number<T>* number(int i);

    QList<QString> presetNames();

    QMap<QString, QList<T>> presets();

    void addPreset(QString name);
    void removePreset(QString name);
    void setPreset(QString name);

protected:
    QString mUniformName;
    int mUniformType;
    QList<Number<T>*> mNumbers;
    QMap<QString, QList<T>> mPresets;
};

#endif // BASEUNIFORMPARAMETER_H
