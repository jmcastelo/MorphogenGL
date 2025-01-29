#ifndef INTWIDGET_H
#define INTWIDGET_H



#include "parameterwidget.h"
#include "focuswidgets.h"



// Integer parameter widget

class IntParameterWidget : public ParameterWidget
{
    Q_OBJECT

public:
    FocusLineEdit* lineEdit;

    IntParameterWidget(IntParameter* theIntParameter, QWidget* parent = nullptr) : ParameterWidget(parent), intParameter(theIntParameter)
    {
        lineEdit = new FocusLineEdit;
        lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

        QIntValidator* validator = new QIntValidator(intParameter->number->getMin(), intParameter->number->getMax(), lineEdit);
        lineEdit->setValidator(validator);
        lineEdit->setText(QString::number(intParameter->number->value));

        connect(lineEdit, &FocusLineEdit::editingFinished, this, [=, this]()
        {
            intParameter->number->setValue(lineEdit->text().toInt());
            intParameter->number->setIndex();
        });
        connect(lineEdit, &FocusLineEdit::focusIn, this, [=, this](){ emit focusIn(intParameter->number); });
        connect(lineEdit, &FocusLineEdit::focusIn, this, QOverload<>::of(&ParameterWidget::focusIn));
        connect(lineEdit, &FocusLineEdit::focusOut, this, &ParameterWidget::focusOut);

        focusedWidget = lineEdit;

        connect(intParameter->number, QOverload<int>::of(&Number<int>::currentValueChanged), this, [=, this](int newValue)
        {
            intParameter->setValue(newValue);
            lineEdit->setText(QString::number(newValue));
            emit focusIn();
        });
    }

    QString getName() { return intParameter->name; }

private:
    IntParameter* intParameter;
};



#endif // INTWIDGET_H
