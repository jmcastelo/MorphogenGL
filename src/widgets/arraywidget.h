#ifndef ARRAYWIDGET_H
#define ARRAYWIDGET_H



#include "parameterwidget.h"
#include "focuswidgets.h"

#include <QGridLayout>



template <typename T>
class ArrayParameterWidget : public ParameterWidget
{
    Q_OBJECT

public:
    ArrayParameterWidget(ArrayParameter<T>* theArrayParameter, QWidget* parent = nullptr) :
        ParameterWidget(parent),
        arrayParameter { theArrayParameter }
    {
        gridLayout = new QGridLayout;

        int row = 0;
        int col = 0;

        for (Number<T>* number : arrayParameter->numbers())
        {
            FocusLineEdit* lineEdit = new FocusLineEdit;
            lineEdit->setFixedWidth(75);

            QValidator* validator;
            if (std::is_same<T, float>::value)
                validator = new QDoubleValidator(number->inf(), number->sup(), 5, lineEdit);
            else if (std::is_same<T, int>::value)
                validator = new QIntValidator(number->inf(), number->sup(), lineEdit);

            lineEdit->setValidator(validator);
            lineEdit->setText(QString::number(number->value()));

            gridLayout->addWidget(lineEdit, row, col, Qt::AlignCenter);

            col++;
            if (col == 3)
            {
                row++;
                col = 0;
            }

            lineEdits.push_back(lineEdit);
        }

        for (size_t i = 0; i < arrayParameter->numbers().size(); i++)
        {
            connect(lineEdits[i], &FocusLineEdit::editingFinished, this, [=, this](){
                if (std::is_same<T, float>::value)
                    arrayParameter->setValue(i, lineEdits[i]->text().toFloat());
                else if (std::is_same<T, int>::value)
                    arrayParameter->setValue(i, lineEdits[i]->text().toInt());
            });

            connect(lineEdits[i], &FocusLineEdit::focusIn, this, [=, this](){ focusedWidget = lineEdits[i]; });
            connect(lineEdits[i], &FocusLineEdit::focusIn, this, [=, this](){ emit focusIn(arrayParameter->numbers(i)); });
            connect(lineEdits[i], &FocusLineEdit::focusIn, this, QOverload<>::of(&ParameterWidget::focusIn));
            connect(lineEdits[i], &FocusLineEdit::focusOut, this, &ParameterWidget::focusOut);

            connect(arrayParameter, &Parameter::valueChanged, this, [=, this](int i, QVariant newValue){
                if (std::is_same<T, float>::value)
                    lineEdits[i]->setText(QString::number(newValue.toFloat()));
                else if (std::is_same<T, int>::value)
                    lineEdits[i]->setText(QString::number(newValue.toInt()));
                emit focusIn();
            });
        }

        focusedWidget = lineEdits[0];
    }

    QString getName() { return arrayParameter->name; }

protected:
    ArrayParameter<T>* arrayParameter;
    QList<FocusLineEdit*> lineEdits;
    QGridLayout* gridLayout;
};



#endif // ARRAYWIDGET_H
