#ifndef ARRAYWIDGET_H
#define ARRAYWIDGET_H



#include "parameterwidget.h"
#include "focuswidgets.h"

#include <QGridLayout>



// Array parameter widget

class ArrayParameterWidget : public ParameterWidget
{
    Q_OBJECT

public:
    QGridLayout* gridLayout;

    ArrayParameterWidget(ArrayParameter* theArrayParameter, QWidget* parent = nullptr) :
        ParameterWidget(parent),
        arrayParameter { theArrayParameter }
    {
        gridLayout = new QGridLayout;

        int row = 0;
        int col = 0;

        for (auto element : arrayParameter->numbers)
        {
            FocusLineEdit* lineEdit = new FocusLineEdit;
            lineEdit->setFixedWidth(75);

            QDoubleValidator* validator = new QDoubleValidator(element->getInf(), element->getSup(), 5, lineEdit);
            lineEdit->setValidator(validator);
            lineEdit->setText(QString::number(element->value));

            gridLayout->addWidget(lineEdit, row, col, Qt::AlignCenter);

            col++;
            if (col == 3)
            {
                row++;
                col = 0;
            }

            lineEdits.push_back(lineEdit);
        }
        for (size_t i = 0; i < arrayParameter->numbers.size(); i++)
        {
            connect(lineEdits[i], &FocusLineEdit::editingFinished, this, [=, this]()
                    {
                        arrayParameter->numbers[i]->setValue(lineEdits[i]->text().toFloat());
                        arrayParameter->numbers[i]->setIndex();
                        arrayParameter->setValues();
                    });
            connect(lineEdits[i], &FocusLineEdit::focusIn, this, [=, this](){ focusedWidget = lineEdits[i]; });
            connect(lineEdits[i], &FocusLineEdit::focusIn, this, [=, this](){ emit focusIn(arrayParameter->numbers[i]); });
            connect(lineEdits[i], &FocusLineEdit::focusIn, this, QOverload<>::of(&ParameterWidget::focusIn));
            connect(lineEdits[i], &FocusLineEdit::focusOut, this, &ParameterWidget::focusOut);
            connect(arrayParameter->numbers[i], QOverload<float>::of(&Number<float>::currentValueChanged), this, [=, this](float newValue)
                    {
                        arrayParameter->setValues();
                        lineEdits[i]->setText(QString::number(newValue));
                        emit focusIn();
                    });
        }

        focusedWidget = lineEdits[0];
    }

    QString getName() { return arrayParameter->name; }

protected:
    ArrayParameter* arrayParameter;
    std::vector<FocusLineEdit*> lineEdits;
};



#endif // ARRAYWIDGET_H
