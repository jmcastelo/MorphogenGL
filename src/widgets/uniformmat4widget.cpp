


#include "uniformmat4widget.h"



UniformMat4ParameterWidget::UniformMat4ParameterWidget(UniformMat4Parameter* theUniformMat4Parameter, QObject* parent) :
    ParameterWidget<float>(parent),
    mUniformMat4Parameter { theUniformMat4Parameter }
{
    mGroupBox->setTitle(mUniformMat4Parameter->name());

    // Set up line edits

    foreach (Number<float>* number, mUniformMat4Parameter->numbers())
    {
        FocusLineEdit* lineEdit = new FocusLineEdit;

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



QString UniformMat4ParameterWidget::name() { return mUniformMat4Parameter->name(); }



void UniformMat4ParameterWidget::setName(QString theName)
{
    mUniformMat4Parameter->setName(theName);
    mGroupBox->setTitle(theName);
}



void UniformMat4ParameterWidget::setMin(float theMin) { mUniformMat4Parameter->setMin(theMin); }
void UniformMat4ParameterWidget::setMax(float theMax) { mUniformMat4Parameter->setMax(theMax); }
void UniformMat4ParameterWidget::setInf(float theInf) { mUniformMat4Parameter->setInf(theInf); }
void UniformMat4ParameterWidget::setSup(float theSup) { mUniformMat4Parameter->setSup(theSup); }



void UniformMat4ParameterWidget::setRow(int i) { mUniformMat4Parameter->setRow(i); }
void UniformMat4ParameterWidget::setCol(int i) { mUniformMat4Parameter->setCol(i); }



void UniformMat4ParameterWidget::setValueFromIndex(int index) { mUniformMat4Parameter->setValueFromIndex(mLastIndex, index); }



QGroupBox* UniformMat4ParameterWidget::widget() { return mGroupBox; }
