#ifndef INTODDWIDGET_H
#define INTODDWIDGET_H



#include "parameterwidget.h"


// Odd integer parameter widget

class IntOddParameterWidget : public ParameterWidget
{
public:
    FocusLineEdit* lineEdit;

    IntOddParameterWidget(IntParameter* theIntParameter, QWidget* parent = nullptr) : ParameterWidget(parent), intParameter(theIntParameter)
    {
        lineEdit = new FocusLineEdit;
        lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

        QIntValidator* validator = new QIntValidator(intParameter->number->getInf(), intParameter->number->getSup(), lineEdit);
        lineEdit->setValidator(validator);
        lineEdit->setText(QString::number(intParameter->number->value));

        connect(lineEdit, &FocusLineEdit::editingFinished, this, [=, this]()
                {
                    int value = lineEdit->text().toInt();
                    if (value > 0 && value % 2 == 0)
                    {
                        value--;
                        lineEdit->setText(QString::number(value));
                    }
                    intParameter->number->setValue(value);
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



#endif // INTODDWIDGET_H
