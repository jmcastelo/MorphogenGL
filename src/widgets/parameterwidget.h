#ifndef PARAMETERWIDGET_H
#define PARAMETERWIDGET_H



#include "../parameter.h"
#include "focuswidgets.h"

#include <QWidget>



// Parameter widget base class

class ParameterWidgetSignals : public QObject
{
    Q_OBJECT

public:
    explicit ParameterWidgetSignals(QObject* parent = nullptr) : QObject(parent) {}

signals:
    void focusIn();
    void focusOut();
};



template <typename T>
class ParameterWidget : public ParameterWidgetSignals
{
public:
    ParameterWidget(QObject* parent = nullptr) : ParameterWidgetSignals(parent) {}

    Number<T>* selectedNumber() { return mSelectedNumber; }
    FocusLineEdit* lastFocusedWidget() { return mLastFocusedWidget; }
    virtual QString name() = 0;

protected:
    Number<T>* mSelectedNumber;
    FocusLineEdit* mLastFocusedWidget;
};



#endif // PARAMETERWIDGET_H
