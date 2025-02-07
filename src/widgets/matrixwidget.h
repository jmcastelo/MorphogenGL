#ifndef MATRIXWIDGET_H
#define MATRIXWIDGET_H



#include "uniformwidget.h"



// Matrix parameter widget

class MatrixParameterWidget : public ArrayParameterWidget
{
    Q_OBJECT

public:
    FocusComboBox* presetsComboBox;

    MatrixParameterWidget(MatrixParameter* theMatrixParameter, QWidget* parent = nullptr) :
        ArrayParameterWidget(theMatrixParameter, parent),
        matrixParameter { theMatrixParameter }
    {
        presets = {
            { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f }, // RGB to RGB
            { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }, // RGB to GRB
            { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f }, // RGB to BGR
            { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f }, // RGB to RBG
            { 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f }, // RGB to BRG
            { 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f }  // RGB to GBR
        };

        presetsComboBox = new FocusComboBox;
        presetsComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        presetsComboBox->addItem("RGB to RGB");
        presetsComboBox->addItem("RGB to GRB");
        presetsComboBox->addItem("RGB to BGR");
        presetsComboBox->addItem("RGB to RBG");
        presetsComboBox->addItem("RGB to BRG");
        presetsComboBox->addItem("RGB to GBR");
        presetsComboBox->setCurrentIndex(0);

        connect(presetsComboBox, QOverload<int>::of(&QComboBox::activated), this, [=, this](int index)
                {
                    for (size_t i = 0; i < presets[index].size(); i++)
                    {
                        matrixParameter->numbers[i]->setValue(presets[index][i]);
                        matrixParameter->numbers[i]->setIndex();
                        lineEdits[i]->setText(QString::number(presets[index][i]));
                    }
                    matrixParameter->setValues();
                });
        connect(presetsComboBox, &FocusComboBox::focusIn, this, [=, this](){ focusedWidget = presetsComboBox; });
        connect(presetsComboBox, &FocusComboBox::focusIn, this, QOverload<>::of(&ParameterWidget::focusIn));
        connect(presetsComboBox, &FocusComboBox::focusOut, this, &ParameterWidget::focusOut);
    }

    QString getName() { return matrixParameter->name; }

private:
    MatrixParameter* matrixParameter;
    std::vector<std::vector<float>> presets;
};



#endif // MATRIXWIDGET_H
