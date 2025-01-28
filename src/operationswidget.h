/*
*  Copyright 2021 Jose Maria Castelo Ares
*
*  Contact: <jose.maria.castelo@gmail.com>
*  Repository: <https://github.com/jmcastelo/MorphogenGL>
*
*  This file is part of MorphogenGL.
*
*  MorphogenGL is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  MorphogenGL is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with MorphogenGL.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "parameter.h"
#include "focuslineedit.h"
#include "imageoperations.h"

#include <vector>
#include <cmath>
#include <QWidget>
#include <QFrame>
#include <QString>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QGroupBox>



// Parameter widget base class

class ParameterWidget : public QWidget
{
    Q_OBJECT

public:
    ParameterWidget(QWidget* parent = nullptr) : QWidget(parent){}

    QWidget* lastFocusedWidget() { return focusedWidget; }
    virtual QString getName() = 0;

signals:
    void focusIn(Number<float>* number);
    void focusIn(Number<int>* number);
    void focusIn();
    void focusOut();

protected:
    QWidget* focusedWidget;
};



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



// Integer parameter widget

class IntParameterWidget : public ParameterWidget
{
    Q_OBJECT

public:
    FocusLineEdit* lineEdit;

    IntParameterWidget(IntParameter* theIntParameter, QWidget* parent = nullptr) : ParameterWidget(parent), intParameter(theIntParameter)
    {
        lineEdit = new FocusLineEdit;
        lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

        QIntValidator* validator = new QIntValidator(intParameter->number->getMin(), intParameter->number->getMax(), lineEdit);
        lineEdit->setValidator(validator);
        lineEdit->setText(QString::number(intParameter->number->value));

        connect(lineEdit, &FocusLineEdit::editingFinished, this, [=, this]()
        {
            intParameter->number->setValue(lineEdit->text().toInt());
            intParameter->number->setIndex();
        });
        connect(lineEdit, &FocusLineEdit::focusIn, this, [=, this](){ emit focusIn(intParameter->number); });
        connect(lineEdit, &FocusLineEdit::focusIn, this, QOverload<>::of(&ParameterWidget::focusIn));
        connect(lineEdit, &FocusLineEdit::focusOut, this, &ParameterWidget::focusOut);

        focusedWidget = lineEdit;

        connect(intParameter->number, QOverload<int>::of(&Number<int>::currentValueChanged), this, [=, this](int newValue)
        {
            intParameter->setValue(newValue);
            lineEdit->setText(QString::number(newValue));
            emit focusIn();
        });
    }

    QString getName() { return intParameter->name; }

private:
    IntParameter* intParameter;
};



// Odd integer parameter widget

class IntOddParameterWidget : public ParameterWidget
{
public:
    FocusLineEdit* lineEdit;

    IntOddParameterWidget(IntParameter* theIntParameter, QWidget* parent = nullptr) : ParameterWidget(parent), intParameter(theIntParameter)
    {
        lineEdit = new FocusLineEdit;
        lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

        QIntValidator* validator = new QIntValidator(intParameter->number->getInf(), intParameter->number->getSup(), lineEdit);
        lineEdit->setValidator(validator);
        lineEdit->setText(QString::number(intParameter->number->value));

        connect(lineEdit, &FocusLineEdit::editingFinished, this, [=, this]()
        {
            int value = lineEdit->text().toInt();
            if (value > 0 && value % 2 == 0)
            {
                value--;
                lineEdit->setText(QString::number(value));
            }
            intParameter->number->setValue(value);
            intParameter->number->setIndex();
        });
        connect(lineEdit, &FocusLineEdit::focusIn, this, [=, this](){ emit focusIn(intParameter->number); });
        connect(lineEdit, &FocusLineEdit::focusIn, this, QOverload<>::of(&ParameterWidget::focusIn));
        connect(lineEdit, &FocusLineEdit::focusOut, this, &ParameterWidget::focusOut);

        focusedWidget = lineEdit;

        connect(intParameter->number, QOverload<int>::of(&Number<int>::currentValueChanged), this, [=, this](int newValue)
        {
            intParameter->setValue(newValue);
            lineEdit->setText(QString::number(newValue));
            emit focusIn();
        });
    }

    QString getName() { return intParameter->name; }

private:
    IntParameter* intParameter;
};



// Float parameter widget

class FloatParameterWidget : public ParameterWidget
{
    Q_OBJECT

public:
    FocusLineEdit* lineEdit;

    FloatParameterWidget(FloatParameter* theFloatParameter, QWidget* parent = nullptr) :
        ParameterWidget(parent),
        floatParameter(theFloatParameter)
    {
        // Focus line edit setup

        lineEdit = new FocusLineEdit;
        lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

        QDoubleValidator* validator = new QDoubleValidator(floatParameter->number->getInf(), floatParameter->number->getSup(), 10, lineEdit);
        validator->setLocale(QLocale::English);
        lineEdit->setValidator(validator);
        lineEdit->setText(QString::number(floatParameter->value()));

        connect(lineEdit, &FocusLineEdit::editingFinished, this, [&]()
        {
            floatParameter->number->setValue(lineEdit->text().toFloat());
            floatParameter->number->setIndex();
        });
        connect(lineEdit, &FocusLineEdit::focusIn, this, [&](){ emit focusIn(floatParameter->number); });
        connect(lineEdit, &FocusLineEdit::focusIn, this, QOverload<>::of(&ParameterWidget::focusIn));
        connect(lineEdit, &FocusLineEdit::focusOut, this, &ParameterWidget::focusOut);

        focusedWidget = lineEdit;

        connect(floatParameter->number, QOverload<float>::of(&Number<float>::currentValueChanged), this, [=, this](float newValue)
        {
            floatParameter->setValue(newValue);
            lineEdit->setText(QString::number(newValue));
            emit focusIn();
        });
    }

    QString getName() { return floatParameter->name; }

private:
    FloatParameter* floatParameter;
};



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

        connect(presetsComboBox, QOverload<int>::of(&QComboBox::activated), this, [=](int index)
        {
            for (size_t i = 0; i < presets[index].size(); i++)
            {
                matrixParameter->numbers[i]->setValue(presets[index][i]);
                matrixParameter->numbers[i]->setIndex();
                lineEdits[i]->setText(QString::number(presets[index][i]));
            }
            matrixParameter->setValues();
        });
        connect(presetsComboBox, &FocusComboBox::focusIn, this, [=](){ focusedWidget = presetsComboBox; });
        connect(presetsComboBox, &FocusComboBox::focusIn, this, QOverload<>::of(&ParameterWidget::focusIn));
        connect(presetsComboBox, &FocusComboBox::focusOut, this, &ParameterWidget::focusOut);
    }

    QString getName() { return matrixParameter->name; }

private:
    MatrixParameter* matrixParameter;
    std::vector<std::vector<float>> presets;
};



// Polar kernel parameter widget

class PolarKernelParameterWidget : public QWidget
{
    Q_OBJECT

public:
    QVBoxLayout* vBoxLayout;

    PolarKernelParameterWidget(PolarKernelParameter* thePolarKernelParameter, QWidget* parent = nullptr) : QWidget(parent), polarKernelParameter { thePolarKernelParameter }
    {
        // Polar kernel plot

        //plot = new PolarKernelPlot("Polar kernel");
              
        // Select kernel combo box

        QComboBox* selectKernelComboBox = new QComboBox;
        selectKernelComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

        for (size_t i = 0; i < polarKernelParameter->polarKernels.size(); i++)
            selectKernelComboBox->addItem(QString::number(i + 1));

        QFormLayout* selectKernelFormLayout = new QFormLayout;
        selectKernelFormLayout->addRow("Selected kernel:", selectKernelComboBox);

        // Add and remove kernel buttons

        QPushButton* addKernelPushButton = new QPushButton("Add new kernel");
        addKernelPushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

        QPushButton* removeKernelPushButton = new QPushButton("Remove selected");
        removeKernelPushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

        QHBoxLayout* buttonsHBoxLayout = new QHBoxLayout;
        buttonsHBoxLayout->setAlignment(Qt::AlignCenter);
        buttonsHBoxLayout->addWidget(addKernelPushButton);
        buttonsHBoxLayout->addWidget(removeKernelPushButton);

        // Parameters line edits

        numElementsLineEdit = new FocusLineEdit;
        QIntValidator* numElementsValidator = new QIntValidator(1, 100, numElementsLineEdit);
        numElementsLineEdit->setValidator(numElementsValidator);

        centerElementLineEdit = new FocusLineEdit;
        QDoubleValidator* centerElementValidator = new QDoubleValidator(-1000.0, 1000.0, 5, centerElementLineEdit);
        centerElementLineEdit->setValidator(centerElementValidator);

        radiusLineEdit = new FocusLineEdit;
        QDoubleValidator* radiusValidator = new QDoubleValidator(0.0, 1.0, 5, radiusLineEdit);
        radiusLineEdit->setValidator(radiusValidator);

        initialAngleLineEdit = new FocusLineEdit;
        QDoubleValidator* initialAngleValidator = new QDoubleValidator(0.0, 360.0, 5, initialAngleLineEdit);
        initialAngleLineEdit->setValidator(initialAngleValidator);

        frequencyLineEdit = new FocusLineEdit;
        QDoubleValidator* frequencyValidator = new QDoubleValidator(0.0, 100.0, 5, frequencyLineEdit);
        frequencyLineEdit->setValidator(frequencyValidator);

        phaseLineEdit = new FocusLineEdit;
        QDoubleValidator* phaseValidator = new QDoubleValidator(-360.0, 360.0, 5, phaseLineEdit);
        phaseLineEdit->setValidator(phaseValidator);

        minimumLineEdit = new FocusLineEdit;
        QDoubleValidator* minimumValidator = new QDoubleValidator(-1000.0, 1000.0, 5, minimumLineEdit);
        minimumLineEdit->setValidator(minimumValidator);

        maximumLineEdit = new FocusLineEdit;
        QDoubleValidator* maximumValidator = new QDoubleValidator(-1000.0, 1000.0, 5, maximumLineEdit);
        maximumLineEdit->setValidator(maximumValidator);

        if (!polarKernelParameter->polarKernels.empty())
        {
            setLineEditTexts();
            //setGeometryPlotData();
            //setKernelValuesPlotData();
        }
        
        QFormLayout* geometryFormLayout = new QFormLayout;
        geometryFormLayout->addRow("Elements:", numElementsLineEdit);
        geometryFormLayout->addRow("Center value:", centerElementLineEdit);
        geometryFormLayout->addRow("Radius:", radiusLineEdit);
        geometryFormLayout->addRow("Initial angle:", initialAngleLineEdit);

        QFormLayout* valuesFormLayout = new QFormLayout;
        valuesFormLayout->addRow("Frequency:", frequencyLineEdit);
        valuesFormLayout->addRow("Phase:", phaseLineEdit);
        valuesFormLayout->addRow("Minimum:", minimumLineEdit);
        valuesFormLayout->addRow("Maximum:", maximumLineEdit);

        QHBoxLayout* formsHBoxLayout = new QHBoxLayout;
        formsHBoxLayout->addLayout(geometryFormLayout);
        formsHBoxLayout->addLayout(valuesFormLayout);
        
        // Main layout

        vBoxLayout = new QVBoxLayout;
        vBoxLayout->addLayout(selectKernelFormLayout);
        vBoxLayout->addLayout(buttonsHBoxLayout);
        vBoxLayout->addLayout(formsHBoxLayout);
        //vBoxLayout->addLayout(plot->layout);

        // Signals + Slots

        connect(selectKernelComboBox, QOverload<int>::of(&QComboBox::activated), this, [&](int index)
        {
            kernelIndex = index;
            setLineEditTexts();
            //setKernelValuesPlotData();
        });

        connect(addKernelPushButton, &QPushButton::pressed, this, [=, this]()
        {
            if (polarKernelParameter->polarKernels.size() < 5)
            {
                polarKernelParameter->polarKernels.push_back(new PolarKernel(*polarKernelParameter->polarKernels.back()));
                polarKernelParameter->setValues();

                kernelIndex = static_cast<int>(polarKernelParameter->polarKernels.size()) - 1;

                selectKernelComboBox->addItem(QString::number(kernelIndex + 1));
                selectKernelComboBox->setCurrentIndex(kernelIndex);

                setLineEditTexts();
                //setGeometryPlotData();
                //setKernelValuesPlotData();
            }
        });
        connect(removeKernelPushButton, &QPushButton::pressed, this, [=]()
        {
            if (polarKernelParameter->polarKernels.size() > 1)
            {
                polarKernelParameter->polarKernels.erase(polarKernelParameter->polarKernels.begin() + kernelIndex);
                polarKernelParameter->setValues();

                selectKernelComboBox->removeItem(0);

                for (int i = 0; i < selectKernelComboBox->count(); i++)
                    selectKernelComboBox->setItemText(i, QString::number(i + 1));

                kernelIndex--;
                kernelIndex = kernelIndex < 0 ? 0 : kernelIndex;

                selectKernelComboBox->setCurrentIndex(kernelIndex);

                setLineEditTexts();
                //setGeometryPlotData();
                //setKernelValuesPlotData();
            }
        });

        connect(numElementsLineEdit, &FocusLineEdit::returnPressed, this, [&]()
        {
            polarKernelParameter->polarKernels[kernelIndex]->numElements = numElementsLineEdit->text().toInt();
            polarKernelParameter->setValues();
            //setGeometryPlotData();
            //setKernelValuesPlotData();
        });
        connect(numElementsLineEdit, &FocusLineEdit::focusOut, this, [=]()
        {
            numElementsLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->numElements));
        });

        connect(centerElementLineEdit, &FocusLineEdit::returnPressed, this, [=]()
        {
            polarKernelParameter->centerElement = centerElementLineEdit->text().toFloat();
            polarKernelParameter->setValues();
        });
        connect(centerElementLineEdit, &FocusLineEdit::focusOut, this, [=]()
        {
            centerElementLineEdit->setText(QString::number(polarKernelParameter->centerElement));
        });

        connect(radiusLineEdit, &FocusLineEdit::returnPressed, this, [&]()
        {
            polarKernelParameter->polarKernels[kernelIndex]->radius = radiusLineEdit->text().toFloat();
            polarKernelParameter->setValues();
            //setGeometryPlotData();
        });
        connect(radiusLineEdit, &FocusLineEdit::focusOut, this, [=]()
        {
            radiusLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->radius));
        });

        connect(initialAngleLineEdit, &FocusLineEdit::returnPressed, this, [&]()
        {
            polarKernelParameter->polarKernels[kernelIndex]->initialAngle = initialAngleLineEdit->text().toFloat();
            polarKernelParameter->setValues();
            //setGeometryPlotData();
        });
        connect(initialAngleLineEdit, &FocusLineEdit::focusOut, this, [=]()
        {
            initialAngleLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->initialAngle));
        });

        connect(frequencyLineEdit, &FocusLineEdit::returnPressed, this, [&]()
        {
            polarKernelParameter->polarKernels[kernelIndex]->frequency = frequencyLineEdit->text().toFloat();
            polarKernelParameter->setValues();
            //setKernelValuesPlotData();
        });
        connect(frequencyLineEdit, &FocusLineEdit::focusOut, this, [=]()
        {
            frequencyLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->frequency));
        });

        connect(phaseLineEdit, &FocusLineEdit::returnPressed, this, [&]()
        {
            polarKernelParameter->polarKernels[kernelIndex]->phase = phaseLineEdit->text().toFloat();
            polarKernelParameter->setValues();
            //setKernelValuesPlotData();
        });
        connect(phaseLineEdit, &FocusLineEdit::focusOut, this, [=]()
        {
            phaseLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->phase));
        });

        connect(minimumLineEdit, &FocusLineEdit::returnPressed, this, [&]()
        {
            polarKernelParameter->polarKernels[kernelIndex]->minimum = minimumLineEdit->text().toFloat();
            polarKernelParameter->setValues();
            //setKernelValuesPlotData();
        });
        connect(minimumLineEdit, &FocusLineEdit::focusOut, this, [=]()
        {
            minimumLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->minimum));
        });

        connect(maximumLineEdit, &FocusLineEdit::returnPressed, this, [&]()
        {
            polarKernelParameter->polarKernels[kernelIndex]->maximum = maximumLineEdit->text().toFloat();
            polarKernelParameter->setValues();
            //setKernelValuesPlotData();
        });
        connect(maximumLineEdit, &FocusLineEdit::focusOut, this, [=]()
        {
            maximumLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->maximum));
        });
    }

    QString getName() { return polarKernelParameter->name; }

private:
    PolarKernelParameter* polarKernelParameter;
    FocusLineEdit* numElementsLineEdit;
    FocusLineEdit* centerElementLineEdit;
    FocusLineEdit* radiusLineEdit;
    FocusLineEdit* initialAngleLineEdit;
    FocusLineEdit* frequencyLineEdit;
    FocusLineEdit* phaseLineEdit;
    FocusLineEdit* minimumLineEdit;
    FocusLineEdit* maximumLineEdit;
    //PolarKernelPlot* plot;
    int kernelIndex = 0;

    void setLineEditTexts()
    {
        numElementsLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->numElements));
        centerElementLineEdit->setText(QString::number(polarKernelParameter->centerElement));
        radiusLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->radius));
        initialAngleLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->initialAngle));
        frequencyLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->frequency));
        phaseLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->phase));
        minimumLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->minimum));
        maximumLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->maximum));
    }

    /*void setGeometryPlotData()
    {
        plot->setGeometryData(polarKernelParameter->polarKernels);
    }

    void setKernelValuesPlotData()
    {
        plot->setKernelValuesData(polarKernelParameter->polarKernels[kernelIndex]);
    }*/
};



// Operations widget

class OperationsWidget : public QFrame
{
    Q_OBJECT

public:
    OperationsWidget(ImageOperation* operation, bool midiEnabled, QWidget* parent) : QFrame(parent)
    {
        setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        mainLayout = new QVBoxLayout;
        mainLayout->setAlignment(Qt::AlignCenter);
        mainLayout->setSizeConstraint(QLayout::SetFixedSize);

        setFrameStyle(QFrame::Panel | QFrame::Plain);
        setLineWidth(0);
        setMidLineWidth(0);

        setup(operation, midiEnabled);
    }

    void setup(ImageOperation* operation, bool midiEnabled)
    {
        QVBoxLayout* parametersLayout = new QVBoxLayout;
        parametersLayout->setAlignment(Qt::AlignCenter);

        QFormLayout* formLayout = new QFormLayout;

        for (auto parameter : operation->getIntParameters())
        {
            if (parameter->isOdd)
            {
                IntOddParameterWidget* widget = new IntOddParameterWidget(parameter, this);
                parameterWidgets.push_back(widget);
                slideableIntParameterWidgets.push_back(widget);
                formLayout->addRow(parameter->name + ":", widget->lineEdit);
            }
            else
            {
                IntParameterWidget* widget = new IntParameterWidget(parameter, this);
                parameterWidgets.push_back(widget);
                slideableIntParameterWidgets.push_back(widget);
                formLayout->addRow(parameter->name + ":", widget->lineEdit);
            }
        }
        for (auto parameter : operation->getFloatParameters())
        {
            FloatParameterWidget* widget = new FloatParameterWidget(parameter, this);
            parameterWidgets.push_back(widget);
            slideableFloatParameterWidgets.push_back(widget);
            formLayout->addRow(parameter->name + ":", widget->lineEdit);
        }
        for (auto parameter : operation->getOptionsIntParameters())
        {
            OptionsParameterWidget<int>* widget = new OptionsParameterWidget<int>(parameter, this);
            parameterWidgets.push_back(widget);
            formLayout->addRow(parameter->name + ":", widget->comboBox);
        }
        for (auto parameter : operation->getOptionsGLenumParameters())
        {
            OptionsParameterWidget<GLenum>* widget = new OptionsParameterWidget<GLenum>(parameter, this);
            parameterWidgets.push_back(widget);
            formLayout->addRow(parameter->name + ":", widget->comboBox);
        }
        if (operation->getKernelParameter())
        {
            KernelParameterWidget* widget = new KernelParameterWidget(operation->getKernelParameter(), this);
            parameterWidgets.push_back(widget);
            slideableFloatParameterWidgets.push_back(widget);
            parametersLayout->addWidget(new QLabel(operation->getKernelParameter()->name + ":"));
            parametersLayout->addLayout(widget->gridLayout);
            parametersLayout->addWidget(widget->normalizePushButton);
            formLayout->addRow("Presets:", widget->presetsComboBox);
        }
        if (operation->getMatrixParameter())
        {
            MatrixParameterWidget* widget = new MatrixParameterWidget(operation->getMatrixParameter(), this);
            parameterWidgets.push_back(widget);
            slideableFloatParameterWidgets.push_back(widget);
            parametersLayout->addWidget(new QLabel(operation->getMatrixParameter()->name + ":"));
            parametersLayout->addLayout(widget->gridLayout);
            formLayout->addRow("Presets:", widget->presetsComboBox);
        }
        if (operation->getPolarKernelParameter())
        {
            PolarKernelParameterWidget* widget = new PolarKernelParameterWidget(operation->getPolarKernelParameter(), this);
            parametersLayout->addLayout(widget->vBoxLayout);
        }

        parametersLayout->addLayout(formLayout);

        QGroupBox* parametersGroupBox = new QGroupBox(operation->getName());
        parametersGroupBox->setLayout(parametersLayout);

        mainLayout->addWidget(parametersGroupBox);

        // Selected float or int parameter

        if (!slideableFloatParameterWidgets.empty() || !slideableIntParameterWidgets.empty())
        {
            FocusSlider* selectedParameterSlider = new FocusSlider(Qt::Horizontal);
            selectedParameterSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            selectedParameterSlider->setRange(0, 10000);

            midiLinkButton = new QPushButton();
            midiLinkButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            midiLinkButton->setCheckable(true);
            midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");
            midiLinkButton->setVisible(midiEnabled);

            FocusLineEdit* selectedParameterMinLineEdit = new FocusLineEdit;
            selectedParameterMinLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
            selectedParameterMinLineEdit->setPlaceholderText("Minimum");

            QDoubleValidator* selectedParameterMinValidator = new QDoubleValidator(selectedParameterMinLineEdit);
            selectedParameterMinValidator->setDecimals(10);
            selectedParameterMinLineEdit->setValidator(selectedParameterMinValidator);

            FocusLineEdit* selectedParameterMaxLineEdit = new FocusLineEdit;
            selectedParameterMaxLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
            selectedParameterMaxLineEdit->setPlaceholderText("Maximum");

            QDoubleValidator* selectedParameterMaxValidator = new QDoubleValidator(selectedParameterMaxLineEdit);
            selectedParameterMaxValidator->setDecimals(10);
            selectedParameterMaxLineEdit->setValidator(selectedParameterMaxValidator);

            /*QGridLayout* selectedParameterLayout = new QGridLayout;
            selectedParameterLayout->addWidget(selectedParameterSlider, 0, 0, 1, 5);
            selectedParameterLayout->addWidget(midiLinkButton, 1, 0, 1, 1);
            selectedParameterLayout->addWidget(selectedParameterMinLineEdit, 1, 1, 1, 2);
            selectedParameterLayout->addWidget(selectedParameterMaxLineEdit, 1, 3, 1, 2);*/

            QHBoxLayout* horizontalLayout = new QHBoxLayout;
            horizontalLayout->setSizeConstraint(QLayout::SetFixedSize);
            horizontalLayout->setAlignment(Qt::AlignJustify);
            horizontalLayout->addWidget(midiLinkButton);
            horizontalLayout->addWidget(selectedParameterMinLineEdit);
            horizontalLayout->addWidget(selectedParameterMaxLineEdit);

            QVBoxLayout* selectedParameterVBoxLayout = new QVBoxLayout;
            selectedParameterVBoxLayout->setSizeConstraint(QLayout::SetFixedSize);
            selectedParameterVBoxLayout->addWidget(selectedParameterSlider);
            selectedParameterVBoxLayout->addLayout(horizontalLayout);

            QGroupBox* selectedParameterGroupBox = new QGroupBox("No parameter selected");
            selectedParameterGroupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            selectedParameterGroupBox->setLayout(selectedParameterVBoxLayout);
            //selectedParameterGroupBox->setLayout(selectedParameterLayout);

            mainLayout->addWidget(selectedParameterGroupBox);

            foreach (auto widget, slideableFloatParameterWidgets)
            {
                connect(widget, QOverload<Number<float>*>::of(&ParameterWidget::focusIn), this, [=, this](Number<float>* number)
                {
                    updateSlideableParameterControls<float>(
                        widget->getName(),
                        number,
                        selectedParameterSlider,
                        selectedParameterMinLineEdit,
                        selectedParameterMinValidator,
                        selectedParameterMaxLineEdit,
                        selectedParameterMaxValidator,
                        selectedParameterGroupBox);

                    updateMidiButtons<float>(number);
                });
            }

            foreach (auto widget, slideableIntParameterWidgets)
            {
                connect(widget, QOverload<Number<int>*>::of(&ParameterWidget::focusIn), this, [=, this](Number<int>* number)
                {
                    updateSlideableParameterControls<int>(
                        widget->getName(),
                        number,
                        selectedParameterSlider,
                        selectedParameterMinLineEdit,
                        selectedParameterMinValidator,
                        selectedParameterMaxLineEdit,
                        selectedParameterMaxValidator,
                        selectedParameterGroupBox);

                    updateMidiButtons<int>(number);
                });
            }
        }

        // Connect widgets

        if (!parameterWidgets.isEmpty())
        {
            foreach (auto widget, parameterWidgets)
            {
                connect(widget, QOverload<>::of(&ParameterWidget::focusIn), this, [=, this](){
                    setLineWidth(1);
                    lastFocusedWidget = widget->lastFocusedWidget();
                    lastFocused = true;
                    emit focusIn(this);
                });
                connect(widget, &ParameterWidget::focusOut, this, [=, this](){
                    setLineWidth(0);
                    emit focusOut(this);
                });
            }

            lastFocusedWidget = parameterWidgets.front()->lastFocusedWidget();
        }

        // Enable button

        enableButton = new FocusPushButton;
        enableButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        enableButton->setFixedHeight(10);
        enableButton->setCheckable(true);
        enableButton->setChecked(operation->isEnabled());
        if (operation->isEnabled())
            enableButton->setStyleSheet("background-color: rgb(0, 255, 0); color: rgb(255, 255, 255)");
        else
            enableButton->setStyleSheet("background-color: rgb(32, 32, 32); color: rgb(255, 255, 255)");

        connect(enableButton, &QPushButton::toggled, this, [=, this](bool checked){
            operation->enable(checked);
            if (checked)
                enableButton->setStyleSheet("background-color: rgb(0, 255, 0); color: rgb(255, 255, 255)");
            else
                enableButton->setStyleSheet("background-color: rgb(32, 32, 32); color: rgb(255, 255, 255)");
            emit enableButtonToggled();
        });
        connect(enableButton, &FocusPushButton::focusIn, this, [=, this](){
            setLineWidth(1);
            lastFocusedWidget = enableButton;
            lastFocused = true;
            emit focusIn(this);
        });
        connect(enableButton, &FocusPushButton::focusOut, this, [=, this](){
            setLineWidth(0);
            emit focusOut(this);
        });

        mainLayout->addWidget(enableButton);

        setLayout(mainLayout);
    }

    void recreate(ImageOperation* operation, bool midiEnabled)
    {
        qDeleteAll(findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly));

        parameterWidgets.clear();
        slideableFloatParameterWidgets.clear();
        slideableIntParameterWidgets.clear();

        setup(operation, midiEnabled);
    }

    void toggleEnableButton(bool checked)
    {
        enableButton->setChecked(checked);
        if (checked)
            enableButton->setStyleSheet("background-color: rgb(0, 255, 0); color: rgb(255, 255, 255)");
        else
            enableButton->setStyleSheet("background-color: rgb(32, 32, 32); color: rgb(255, 255, 255)");
    }

    bool isFocused()
    {
        return lastFocusedWidget->hasFocus();
    }

    bool isLastFocused()
    {
        return lastFocused;
    }

    void setLastFocused(bool focus)
    {
        lastFocused = focus;
    }

    void toggleMidiButtons(bool show)
    {
        midiLinkButton->setHidden(!show);
    }

signals:
    void enableButtonToggled();
    void focusIn(QWidget* widget);
    void focusOut(QWidget* widget);
    void linkWait(Number<float>* number);
    void linkWait(Number<int>* number);
    void linkBreak(Number<float>* number);
    void linkBreak(Number<int>* number);

private:
    QVBoxLayout* mainLayout;

    QVector<ParameterWidget*> parameterWidgets;
    QVector<ParameterWidget*> slideableFloatParameterWidgets;
    QVector<ParameterWidget*> slideableIntParameterWidgets;

    QPushButton* midiLinkButton;

    FocusPushButton* enableButton;

    QWidget* lastFocusedWidget = nullptr;
    bool lastFocused = false;

    template <class T>
    void updateSlideableParameterControls(
            QString name,
            Number<T>* number,
            FocusSlider* selectedParameterSlider,
            FocusLineEdit* selectedParameterMinLineEdit,
            QDoubleValidator* selectedParameterMinValidator,
            FocusLineEdit* selectedParameterMaxLineEdit,
            QDoubleValidator* selectedParameterMaxValidator,
            QGroupBox* selectedParameterGroupBox)
    {
        // Slider

        selectedParameterSlider->disconnect();
        selectedParameterSlider->setRange(0, number->indexMax);
        selectedParameterSlider->setValue(number->getIndex());

        connect(selectedParameterSlider, &QAbstractSlider::sliderMoved, number, &Number<T>::setValueFromIndex);

        connect(number, &Number<T>::currentIndexChanged, selectedParameterSlider, &QAbstractSlider::setValue);
        /*connect(number, &Number<T>::currentIndexChanged, this, [=](int currentIndex)
        {
            //disconnect(selectedParameterSlider, &QAbstractSlider::valueChanged, nullptr, nullptr);
            selectedParameterSlider->setValue(currentIndex);
            //connect(selectedParameterSlider, &QAbstractSlider::valueChanged, number, &Number<T>::setValueFromIndex);
        });*/

        // Focus

        connect(selectedParameterSlider, &FocusSlider::focusIn, this, [=, this](){
            setLineWidth(1);
            lastFocusedWidget = selectedParameterSlider;
            lastFocused = true;
            emit focusIn(this);
        });
        connect(selectedParameterSlider, &FocusSlider::focusOut, this, [=, this](){
            setLineWidth(0);
            emit focusOut(this);
        });

        // Value changed: check if within min/max range and adjust controls

        connect(number, QOverload<float>::of(&Number<T>::currentValueChanged), this, [=](double currentValue)
        {
            if (currentValue < number->getMin())
            {
                number->setMin(currentValue);

                selectedParameterMinLineEdit->setText(QString::number(currentValue));

                disconnect(selectedParameterSlider, &QAbstractSlider::valueChanged, nullptr, nullptr);
                selectedParameterSlider->setValue(number->getIndex());
                connect(selectedParameterSlider, &QAbstractSlider::valueChanged, number, &Number<T>::setValueFromIndex);
            }
            else if (currentValue > number->getMax())
            {
                number->setMax(currentValue);

                selectedParameterMaxLineEdit->setText(QString::number(currentValue));

                disconnect(selectedParameterSlider, &QAbstractSlider::valueChanged, nullptr, nullptr);
                selectedParameterSlider->setValue(number->getIndex());
                connect(selectedParameterSlider, &QAbstractSlider::valueChanged, number, &Number<T>::setValueFromIndex);
            }
        });

        // Minimum

        selectedParameterMinLineEdit->disconnect();
        selectedParameterMinLineEdit->setText(QString::number(number->getMin()));

        connect(selectedParameterMinLineEdit, &FocusLineEdit::editingFinished, this, [=]()
        {
            number->setMin(selectedParameterMinLineEdit->text().toDouble());
            number->setIndex();
        });
        connect(selectedParameterMinLineEdit, &FocusLineEdit::focusOut, this, [=]()
        {
            selectedParameterMinLineEdit->setText(QString::number(number->getMin()));
        });

        // Maximum

        selectedParameterMaxLineEdit->disconnect();
        selectedParameterMaxLineEdit->setText(QString::number(number->getMax()));

        connect(selectedParameterMaxLineEdit, &FocusLineEdit::editingFinished, this, [=]()
        {
            number->setMax(selectedParameterMaxLineEdit->text().toDouble());
            number->setIndex();
        });
        connect(selectedParameterMaxLineEdit, &FocusLineEdit::focusOut, this, [=]()
        {
            selectedParameterMaxLineEdit->setText(QString::number(number->getMax()));
        });

        // Validators

        selectedParameterMinValidator->setBottom(number->getInf());
        selectedParameterMinValidator->setTop(number->getMax());

        selectedParameterMaxValidator->setBottom(number->getMin());
        selectedParameterMaxValidator->setTop(number->getSup());

        // Focus

        connect(selectedParameterMinLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
            setLineWidth(1);
            lastFocusedWidget = selectedParameterMinLineEdit;
            lastFocused = true;
            emit focusIn(this);
        });
        connect(selectedParameterMinLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
            setLineWidth(0);
            emit focusOut(this);
        });

        connect(selectedParameterMaxLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
            setLineWidth(1);
            lastFocusedWidget = selectedParameterMaxLineEdit;
            lastFocused = true;
            emit focusIn(this);
        });
        connect(selectedParameterMaxLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
            setLineWidth(0);
            emit focusOut(this);
        });

        // Title

        selectedParameterGroupBox->setTitle("Selected parameter: " + name);
    }

    template <typename T>
    void updateMidiButtons(Number<T>* number)
    {
        // MIDI Link button

        midiLinkButton->disconnect();

        midiLinkButton->setChecked(number->isMidiLinked());

        if (number->isMidiLinked())
            midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-green.png); }");
        else
            midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");

        connect(midiLinkButton, &QPushButton::clicked, this, [=, this](bool checked)
        {
            if (checked)
            {
                midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-orange.png); }");
                emit linkWait(number);
            }
            else
            {
                midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");
                emit linkBreak(number);
            }
        });

        connect(number, &Number<T>::linked, this, [=, this](bool set){
            if (set)
                midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-green.png); }");
            else
                midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");
        });
    }

    void focusInEvent(QFocusEvent *event) override
    {
        lastFocusedWidget->setFocus(Qt::MouseFocusReason);
        event->accept();
    }
};
