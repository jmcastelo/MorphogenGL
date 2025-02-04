#ifndef NUMBERWIDGET_H
#define NUMBERWIDGET_H



#include "parameterwidget.h"
#include "focuswidgets.h"



template <typename T>
class NumberParameterWidget : public ParameterWidget
{
    Q_OBJECT

public:
    NumberParameterWidget(NumberParameter<T>* theParameter, QWidget* parent = nullptr) :
        ParameterWidget(parent),
        mParameter(theParameter)
    {
        lineEdit = new FocusLineEdit;
        lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        QValidator* validator;
        if (std::is_same<T, float>::value)
            validator = new QDoubleValidator(mParameter->inf(), mParameter->sup(), 10, lineEdit);
        else if (std::is_same<T, int>::value)
            validator = new QIntValidator(mParameter->inf(), mParameter->sup(), lineEdit);

        validator->setLocale(QLocale::English);

        lineEdit->setValidator(validator);
        lineEdit->setText(QString::number(mParameter->value()));

        connect(lineEdit, &FocusLineEdit::editingFinished, this, [&](){
            if (std::is_same<T, float>::value)
                mParameter->setValue(lineEdit->text().toFloat());
            else if (std::is_same<T, int>::value)
                mParameter->setValue(lineEdit->text().toInt());
            mParameter->setIndex();
        });

        connect(lineEdit, &FocusLineEdit::focusIn, this, [&](){ emit focusIn(mParameter->number()); });
        connect(lineEdit, &FocusLineEdit::focusIn, this, QOverload<>::of(&ParameterWidget::focusIn));
        connect(lineEdit, &FocusLineEdit::focusOut, this, &ParameterWidget::focusOut);

        connect(mParameter, &Parameter::valueChanged, this, [=, this](QVariant newValue){
            if (std::is_same<T, float>::value)
                lineEdit->setText(QString::number(newValue.toFloat()));
            else if (std::is_same<T, int>::value)
                lineEdit->setText(QString::number(newValue.toInt()));
            emit focusIn();
        });

        focusedWidget = lineEdit;
    }

    QString getName() { return mParameter->name(); }

private:
    NumberParameter<T>* mParameter;
    FocusLineEdit* lineEdit;
};



#endif // NUMBERWIDGET_H
