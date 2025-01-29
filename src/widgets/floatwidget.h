#ifndef FLOATWIDGET_H
#define FLOATWIDGET_H



#include "parameterwidget.h"
#include "focuswidgets.h"



// Float parameter widget

class FloatParameterWidget : public ParameterWidget
{
    Q_OBJECT

public:
    FocusLineEdit* lineEdit;

    FloatParameterWidget(FloatParameter* theFloatParameter, QWidget* parent = nullptr) :
        ParameterWidget(parent),
        floatParameter(theFloatParameter)
    {
        // Focus line edit setup

        lineEdit = new FocusLineEdit;
        lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

        QDoubleValidator* validator = new QDoubleValidator(floatParameter->number->getInf(), floatParameter->number->getSup(), 10, lineEdit);
        validator->setLocale(QLocale::English);
        lineEdit->setValidator(validator);
        lineEdit->setText(QString::number(floatParameter->value()));

        connect(lineEdit, &FocusLineEdit::editingFinished, this, [&]()
        {
            floatParameter->number->setValue(lineEdit->text().toFloat());
            floatParameter->number->setIndex();
        });
        connect(lineEdit, &FocusLineEdit::focusIn, this, [&](){ emit focusIn(floatParameter->number); });
        connect(lineEdit, &FocusLineEdit::focusIn, this, QOverload<>::of(&ParameterWidget::focusIn));
        connect(lineEdit, &FocusLineEdit::focusOut, this, &ParameterWidget::focusOut);

        focusedWidget = lineEdit;

        connect(floatParameter->number, QOverload<float>::of(&Number<float>::currentValueChanged), this, [=, this](float newValue)
        {
            floatParameter->setValue(newValue);
            lineEdit->setText(QString::number(newValue));
            emit focusIn();
        });
    }

    QString getName() { return floatParameter->name; }

private:
    FloatParameter* floatParameter;
};



#endif // FLOATWIDGET_H
