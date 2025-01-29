#ifndef OPTIONSWIDGET_H
#define OPTIONSWIDGET_H



#include "parameterwidget.h"
#include "focuswidgets.h"



// Options parameter widget: QComboBox

template <class T>
class OptionsParameterWidget : public ParameterWidget
{
public:
    FocusComboBox* comboBox;

    OptionsParameterWidget(OptionsParameter<T>* theOptionsParameter, QWidget* parent = nullptr) : ParameterWidget(parent), optionsParameter(theOptionsParameter)
    {
        // Get current index

        int index = 0;
        for (size_t i = 0; i < optionsParameter->values.size(); i++)
            if (optionsParameter->value() == optionsParameter->values[i])
                index = static_cast<int>(i);

        comboBox = new FocusComboBox;
        comboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        for (auto valueName : optionsParameter->valueNames)
            comboBox->addItem(valueName);
        comboBox->setCurrentIndex(index);

        connect(comboBox, QOverload<int>::of(&QComboBox::activated), this, [=, this](int index)
        {
            optionsParameter->setValue(index);
        });

        connect(comboBox, &FocusComboBox::focusIn, this, [=, this](){ emit focusIn(); });
        connect(comboBox, &FocusComboBox::focusOut, this, &ParameterWidget::focusOut);

        focusedWidget = comboBox;
    }

    QString getName() { return optionsParameter->name; }

private:
    OptionsParameter<T>* optionsParameter;
};



#endif // OPTIONSWIDGET_H
