#ifndef PARAMETERWIDGET_H
#define PARAMETERWIDGET_H



#include "../parameters/number.h"
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
    }

    Number<T>* selectedNumber() { return mSelectedNumber; }
    FocusLineEdit* lastFocusedWidget() { return mLastFocusedWidget; }
    QGroupBox* widget() { return mGroupBox; }

    virtual void setRow(int i) = 0;
    virtual void setCol(int i) = 0;

    virtual void setValueFromIndex(int index) = 0;

    virtual QString name() = 0;
    virtual void setName(QString theName) = 0;

    virtual void setMin(T theMin) = 0;
    virtual void setMax(T theMax) = 0;
    virtual void setInf(T theInf) = 0;
    virtual void setSup(T theSup) = 0;

protected:
    Number<T>* mSelectedNumber;
    FocusLineEdit* mLastFocusedWidget;
    QGroupBox* mGroupBox;
};



#endif // PARAMETERWIDGET_H
