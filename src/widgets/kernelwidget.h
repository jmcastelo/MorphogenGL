#ifndef KERNELWIDGET_H
#define KERNELWIDGET_H



#include "uniformwidget.h"



// Kernel parameter widget

class KernelParameterWidget : public ArrayParameterWidget
{
    Q_OBJECT

public:
    FocusPushButton* normalizePushButton;
    FocusComboBox* presetsComboBox;

    KernelParameterWidget(KernelParameter* theKernelParameter, QWidget* parent = nullptr) :
        ArrayParameterWidget(theKernelParameter, parent),
        kernelParameter { theKernelParameter }
    {
        normalizePushButton = new FocusPushButton;
        normalizePushButton->setText("Normalize");
        normalizePushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        normalizePushButton->setVisible(kernelParameter->normalize);

        connect(normalizePushButton, &QPushButton::clicked, this, [=, this]()
                {
                    float sum = 0.0;
                    for (auto element : kernelParameter->numbers)
                        sum += fabs(element->value);
                    if (sum > 0)
                    {
                        for (size_t i = 0; i < kernelParameter->numbers.size(); i++)
                        {
                            kernelParameter->numbers[i]->setValue(kernelParameter->numbers[i]->value / sum);
                            kernelParameter->numbers[i]->setIndex();
                            lineEdits[i]->setText(QString::number(kernelParameter->numbers[i]->value));
                        }

                        kernelParameter->setValues();
                    }
                });
        connect(normalizePushButton, &FocusPushButton::focusIn, this, [=, this](){ focusedWidget = normalizePushButton; });
        connect(normalizePushButton, &FocusPushButton::focusIn, this, QOverload<>::of(&ParameterWidget::focusIn));
        connect(normalizePushButton, &FocusPushButton::focusOut, this, &ParameterWidget::focusOut);

        presets = {
            { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f }, // Identity
            { 0.0f, -1.0f, 0.0f, -1.0f, 4.0f, -1.0f, 0.0f, -1.0f, 0.0f }, // Soft edge detection
            { -1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f }, // Hard edge detection
            { 0.0f, -1.0f, 0.0f, -1.0f, 5.0f, -1.0f, 0.0f, -1.0f, 0.0f }, // Sharpen
            { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f }, // Box blur
            { 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f, 2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f } // Gaussian blur
        };

        presetsComboBox = new FocusComboBox;
        presetsComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        presetsComboBox->addItem("Identity");
        presetsComboBox->addItem("Laplacian");
        presetsComboBox->addItem("Laplacian diagonals");
        presetsComboBox->addItem("Sharpen");
        presetsComboBox->addItem("Box blur");
        presetsComboBox->addItem("Gaussian blur");
        presetsComboBox->setCurrentIndex(0);

        connect(presetsComboBox, QOverload<int>::of(&QComboBox::activated), this, [=, this](int index)
                {
                    for (size_t i = 0; i < presets[index].size(); i++)
                    {
                        kernelParameter->numbers[i]->setValue(presets[index][i]);
                        lineEdits[i]->setText(QString::number(presets[index][i]));
                    }
                    kernelParameter->setValues();
                });
        connect(presetsComboBox, &FocusComboBox::focusIn, this, [=, this](){ focusedWidget = presetsComboBox; });
        connect(presetsComboBox, &FocusComboBox::focusIn, this, QOverload<>::of(&ParameterWidget::focusIn));
        connect(presetsComboBox, &FocusComboBox::focusOut, this, &ParameterWidget::focusOut);
    }

    QString getName() { return kernelParameter->name; }

private:
    KernelParameter* kernelParameter;
    std::vector<std::vector<float>> presets;
};



#endif // KERNELWIDGET_H
