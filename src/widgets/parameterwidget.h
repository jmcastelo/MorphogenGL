#ifndef PARAMETERWIDGET_H
#define PARAMETERWIDGET_H



#include "../parameter.h"

#include <QWidget>



// Parameter widget base class

class ParameterWidget : public QWidget
{
    Q_OBJECT

public:
    ParameterWidget(QWidget* parent = nullptr) : QWidget(parent){}

    QWidget* lastFocusedWidget() { return focusedWidget; }
    virtual QString getName() = 0;

signals:
    void focusIn(Number<QVariant>* number);
    void focusIn();
    void focusOut();

protected:
    QWidget* focusedWidget;
};



#endif // PARAMETERWIDGET_H
