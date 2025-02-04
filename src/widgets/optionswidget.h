#ifndef OPTIONSWIDGET_H
#define OPTIONSWIDGET_H



#include "parameterwidget.h"
#include "focuswidgets.h"



template <class T>
class OptionsParameterWidget : public ParameterWidget
{
public:
    OptionsParameterWidget(OptionsParameter<T>* theOptionsParameter, QWidget* parent = nullptr) :
        ParameterWidget(parent),
        optionsParameter { theOptionsParameter }
    {
        comboBox = new FocusComboBox;
        comboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        for (QString valueName : optionsParameter->valueNames())
            comboBox->addItem(valueName);
        comboBox->setCurrentIndex(optionsParameter->indexOf());

        connect(comboBox, QOverload<int>::of(&QComboBox::activated), this, [=, this](int index)
        {
            optionsParameter->setValue(index);
        });

        connect(comboBox, &FocusComboBox::focusIn, this, QOverload<>::of(&ParameterWidget::focusIn));
        connect(comboBox, &FocusComboBox::focusOut, this, &ParameterWidget::focusOut);

        focusedWidget = comboBox;
    }

    QString getName() { return optionsParameter->name; }

private:
    OptionsParameter<T>* optionsParameter;
    FocusComboBox* comboBox;
};



#endif // OPTIONSWIDGET_H
