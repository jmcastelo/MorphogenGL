#ifndef PARAMETERWIDGET_H
#define PARAMETERWIDGET_H



#include "../parameter.h"
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
    ParameterWidget(QObject* parent = nullptr) : ParameterWidgetSignals(parent) {}

    Number<T>* selectedNumber() { return mSelectedNumber; }
    FocusLineEdit* lastFocusedWidget() { return mLastFocusedWidget; }
    QGroupBox* widget() { return mGroupBox; }

    virtual void setRow(int i) = 0;
    virtual void setCol(int i) = 0;

    virtual void setValueFromIndex(int index) = 0;
    virtual QString name() = 0;

protected:
    Number<T>* mSelectedNumber;
    FocusLineEdit* mLastFocusedWidget;
    QGroupBox* mGroupBox;
};



#endif // PARAMETERWIDGET_H
