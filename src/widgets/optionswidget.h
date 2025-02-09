#ifndef OPTIONSWIDGET_H
#define OPTIONSWIDGET_H



#include "../parameter.h"
#include "parameterwidget.h"
#include "focuswidgets.h"

#include <QGroupBox>
#include <QVBoxLayout>



template <class T>
class OptionsParameterWidget : public ParameterWidgetSignals
{
public:
    OptionsParameterWidget(OptionsParameter<T>* theOptionsParameter, QObject* parent = nullptr) :
        ParameterWidgetSignals(parent),
        mOptionsParameter { theOptionsParameter }
    {
        mGroupBox = new QGroupBox(mOptionsParameter->name());

        // Set up combo box

        mComboBox = new FocusComboBox;
        mComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        for (QString valueName : mOptionsParameter->valueNames())
            mComboBox->addItem(valueName);
        mComboBox->setCurrentIndex(mOptionsParameter->indexOf());

        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(mComboBox);
        mGroupBox->setLayout(layout);

        // Connections

        connect(mComboBox, QOverload<int>::of(&QComboBox::activated), this, [=, this](int index)
        {
            mOptionsParameter->setValue(index);
        });
    }

    QString name() { return mOptionsParameter->name; }

    QGroupBox* widget() { return mGroupBox; }

private:
    OptionsParameter<T>* mOptionsParameter;
    QGroupBox* mGroupBox;
    FocusComboBox* mComboBox;
};



#endif // OPTIONSWIDGET_H
