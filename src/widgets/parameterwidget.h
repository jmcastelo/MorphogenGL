#ifndef PARAMETERWIDGET_H
#define PARAMETERWIDGET_H



#include "../parameters/number.h"
#include "../parameters/parameter.h"
#include "focuswidgets.h"

#include <QObject>
#include <QGroupBox>



// Parameter widget signals class

class ParameterWidgetSignals : public QObject
{
    Q_OBJECT

public:
    explicit ParameterWidgetSignals(QObject* parent = nullptr) : QObject(parent) {}

signals:
    void focusIn();
    void focusOut();
};



// Parameter widget base class

template <typename T>
class ParameterWidget : public ParameterWidgetSignals
{
public:
    ParameterWidget(QObject* parent = nullptr) : ParameterWidgetSignals(parent)
    {
        mGroupBox = new QGroupBox;
        mGroupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        mGroupBox->setStyleSheet("QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; font-size: 18pt; margin: 7px; }");
        mGroupBox->setCheckable(false);
    }

    Number<T>* selectedNumber() { return mSelectedNumber; }

    FocusLineEdit* lastFocusedWidget() { return mLastFocusedWidget; }

    QGroupBox* widget() { return mGroupBox; }

    void setCheckable(bool checkable)
    {
        mGroupBox->setCheckable(checkable);

        if (checkable)
        {
            mGroupBox->setChecked(mParameter->editable());
            connect(mGroupBox, &QGroupBox::toggled, mParameter, &Parameter::setEditable);
        }
        else
        {
            disconnect(mGroupBox, &QGroupBox::toggled, mParameter, &Parameter::setEditable);
        }
    }
    void toggleVisibility(bool visible) { mGroupBox->setVisible(visible); }
    void setDefaultVisibility() { mGroupBox->setVisible(mParameter->editable()); }

    void setRow(int i) { mParameter->setRow(i); }
    void setCol(int i) { mParameter->setCol(i); }

    QString name(){ return mParameter->name(); }

    void setName(QString theName)
    {
        mParameter->setName(theName);
        mGroupBox->setTitle(theName);
    }

    virtual void setValueFromIndex(int index) = 0;

    virtual void setMin(T theMin) = 0;
    virtual void setMax(T theMax) = 0;
    virtual void setInf(T theInf) = 0;
    virtual void setSup(T theSup) = 0;

    bool isEditable(){ return mParameter->editable(); }

    Parameter* parameter() const { return mParameter; }

protected:
    Parameter* mParameter;
    Number<T>* mSelectedNumber;
    FocusLineEdit* mLastFocusedWidget;
    QGroupBox* mGroupBox;
};



#endif // PARAMETERWIDGET_H
