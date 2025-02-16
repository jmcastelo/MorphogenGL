#ifndef UNIFORMMAT4WIDGET_H
#define UNIFORMMAT4WIDGET_H



#include "../parameter.h"
#include "parameterwidget.h"
#include "focuswidgets.h"

#include <QGroupBox>
#include <QFormLayout>



class UniformMat4ParameterWidget : public ParameterWidget<float>
{
public:
    UniformMat4ParameterWidget(UniformMat4Parameter* theUniformMat4Parameter, QObject* parent = nullptr) :
        ParameterWidget<float>(parent),
        mUniformMat4Parameter { theUniformMat4Parameter }
    {
        mGroupBox = new QGroupBox(mUniformMat4Parameter->name());

        // Set up line edits

        foreach (Number<float>* number, mUniformMat4Parameter->numbers())
        {
            FocusLineEdit* lineEdit = new FocusLineEdit;
            lineEdit->setFixedWidth(75);

            QDoubleValidator* validator = new QDoubleValidator(number->inf(), number->sup(), 5, lineEdit);
            lineEdit->setValidator(validator);

            lineEdit->setText(QString::number(number->value()));

            mLineEdits.append(lineEdit);
        }

        mLastFocusedWidget = mLineEdits[0];
        mLastIndex = 0;

        // Set up form layout

        QFormLayout* formLayout = new QFormLayout;

        int i = 0;
        foreach (QString numberName, mUniformMat4Parameter->numberNames())
            formLayout->addRow(numberName, mLineEdits[i++]);

        mGroupBox->setLayout(formLayout);

        // Connections

        for (int i = 0; i < mUniformMat4Parameter->numbers().size(); i++)
        {
            connect(mLineEdits[i], &FocusLineEdit::editingFinished, this, [=, this](){
                mUniformMat4Parameter->setValue(i, mLineEdits[i]->text().toFloat());
            });

            connect(mLineEdits[i], &FocusLineEdit::focusIn, this, [=, this](){
                mSelectedNumber = mUniformMat4Parameter->number(i);
                mLastFocusedWidget = mLineEdits[i];
                mLastIndex = i;
            });
            connect(mLineEdits[i], &FocusLineEdit::focusIn, this, &UniformMat4ParameterWidget::focusIn);
            connect(mLineEdits[i], &FocusLineEdit::focusOut, this, &UniformMat4ParameterWidget::focusOut);

            connect(mUniformMat4Parameter, QOverload<int, QVariant>::of(&Parameter::valueChanged), this, [=, this](int i, QVariant newValue){
                    mLineEdits[i]->setText(QString::number(newValue.toFloat()));
            });
        }
    }

    QString name() { return mUniformMat4Parameter->name(); }

    void setValueFromIndex(int index)
    {
        mUniformMat4Parameter->setValueFromIndex(mLastIndex, index);
    }

    QGroupBox* widget() { return mGroupBox; }

    //Number<float>* selectedNumber() { return mSelectedNumber; }

    //FocusLineEdit* lastFocusedWidget() { return mLastFocusedWidget; }

private:
    UniformMat4Parameter* mUniformMat4Parameter;
    QGroupBox* mGroupBox;
    QList<FocusLineEdit*> mLineEdits;
    int mLastIndex;
    //Number<float>* mSelectedNumber;
//    FocusLineEdit* mLastFocusedWidget;
};




#endif // UNIFORMMAT4WIDGET_H
