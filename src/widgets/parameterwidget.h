#ifndef PARAMETERWIDGET_H
#define PARAMETERWIDGET_H



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



#endif // PARAMETERWIDGET_H
