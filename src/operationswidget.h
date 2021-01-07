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
#include "polarkernelplot.h"
#include <vector>
#include <string>
#include <cmath>
#include <QWidget>
#include <QString>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QCloseEvent>

// Bool parameter widget: QCheckBox

class BoolParameterWidget : public QWidget
{
public:
    QCheckBox* checkBox;

    BoolParameterWidget(BoolParameter* theBoolParameter, QWidget* parent = nullptr) : QWidget(parent), boolParameter(theBoolParameter)
    {
        checkBox = new QCheckBox(boolParameter->name);
        checkBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        checkBox->setChecked(boolParameter->value);

        connect(checkBox, &QCheckBox::stateChanged, [=](int state) { boolParameter->value = (state == Qt::Checked); });
    }

private:
    BoolParameter* boolParameter;
};

// Options parameter widget: QComboBox

template <class T>
class OptionsParameterWidget : public QWidget
{
public:
    QComboBox* comboBox;

    OptionsParameterWidget(OptionsParameter<T>* theOptionsParameter, QWidget* parent = nullptr) : QWidget(parent), optionsParameter(theOptionsParameter)
    {
        // Get current index

        int index = 0;
        for (size_t i = 0; i < optionsParameter->values.size(); i++)
            if (optionsParameter->value == optionsParameter->values[i])
                index = static_cast<int>(i);

        comboBox = new QComboBox;
        comboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        for (auto valueName : optionsParameter->valueNames)
            comboBox->addItem(valueName);
        comboBox->setCurrentIndex(index);

        connect(comboBox, QOverload<int>::of(&QComboBox::activated), [&optionsParameter = this->optionsParameter](int index)
        {
            optionsParameter->setValue(index);
        });
    }

private:
    OptionsParameter<T>* optionsParameter;
};

// Integer parameter widget

class IntParameterWidget : public QWidget
{
public:
    FocusLineEdit* lineEdit;

    IntParameterWidget(IntParameter* theIntParameter, QWidget* parent = nullptr) : QWidget(parent), intParameter(theIntParameter)
    {
        lineEdit = new FocusLineEdit;
        lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

        QIntValidator* validator = new QIntValidator(intParameter->min, intParameter->max, lineEdit);
        lineEdit->setValidator(validator);
        lineEdit->setText(QString::number(intParameter->value));

        connect(lineEdit, &FocusLineEdit::returnPressed, [&intParameter = this->intParameter, &lineEdit = this->lineEdit]()
        {
            intParameter->value = lineEdit->text().toInt();
        });
        connect(lineEdit, &FocusLineEdit::focusOut, [&intParameter = this->intParameter, &lineEdit = this->lineEdit]()
        {
            lineEdit->setText(QString::number(intParameter->value));
        });
    }

private:
    IntParameter* intParameter;
};

// Odd integer parameter widget

class IntOddParameterWidget : public QWidget
{
public:
    FocusLineEdit* lineEdit;

    IntOddParameterWidget(IntParameter* theIntParameter, QWidget* parent = nullptr) : QWidget(parent), intParameter(theIntParameter)
    {
        lineEdit = new FocusLineEdit;
        lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

        QIntValidator* validator = new QIntValidator(intParameter->min, intParameter->max, lineEdit);
        lineEdit->setValidator(validator);
        lineEdit->setText(QString::number(intParameter->value));

        connect(lineEdit, &FocusLineEdit::returnPressed, [&intParameter = this->intParameter, &lineEdit = this->lineEdit]()
        {
            int value = lineEdit->text().toInt();
            if (value > 0 && value % 2 == 0)
            {
                value--;
                lineEdit->setText(QString::number(value));
            }
            intParameter->value = value;
        });
        connect(lineEdit, &FocusLineEdit::focusOut, [&intParameter = this->intParameter, &lineEdit = this->lineEdit]()
        {
            lineEdit->setText(QString::number(intParameter->value));
        });
    }

private:
    IntParameter* intParameter;
};

// Float parameter widget

class FloatParameterWidget : public QWidget
{
    Q_OBJECT

public:
    FocusLineEdit* lineEdit;

    int indexMax;

    FloatParameterWidget(FloatParameter* theFloatParameter, int theIndexMax, QWidget* parent = nullptr) :
        QWidget(parent),
        indexMax(theIndexMax),
        floatParameter(theFloatParameter)
    {
        // Focus line edit setup

        lineEdit = new FocusLineEdit;
        lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

        QDoubleValidator* validator = new QDoubleValidator(floatParameter->inf, floatParameter->sup, 10, lineEdit);
        validator->setLocale(QLocale::English);
        lineEdit->setValidator(validator);
        lineEdit->setText(QString::number(floatParameter->value));

        connect(lineEdit, &FocusLineEdit::returnPressed, [&]()
        {
            floatParameter->setValue(lineEdit->text().toFloat());
            emit currentValueChanged(floatParameter->value);
            setIndex();
        });
        connect(lineEdit, &FocusLineEdit::focusOut, [&floatParameter = this->floatParameter, &lineEdit = this->lineEdit]()
        {
            lineEdit->setText(QString::number(floatParameter->value));
        });
        connect(lineEdit, &FocusLineEdit::focusIn, [&]()
        {
            emit focusIn();
        });
    }

    QString getName() { return floatParameter->name; }

    void setValue(int newIndex)
    {
        float newValue = floatParameter->min + (floatParameter->max - floatParameter->min) * newIndex / indexMax;
        floatParameter->setValue(newValue);
        lineEdit->setText(QString::number(newValue));
    }

    void setIndex()
    {
        int index = static_cast<int>(indexMax * (floatParameter->value - floatParameter->min) / (floatParameter->max - floatParameter->min));
        emit currentIndexChanged(index);
    }

    int getIndex()
    {
        return static_cast<int>(indexMax * (floatParameter->value - floatParameter->min) / (floatParameter->max - floatParameter->min));
    }

    void setMin(float min) { floatParameter->min = min; }
    float getMin() { return floatParameter->min; }

    void setMax(float max) { floatParameter->max = max; }
    float getMax() { return floatParameter->max; }

    float getInf() { return floatParameter->inf; }
    float getSup() { return floatParameter->sup; }

private:
    FloatParameter* floatParameter;

signals:
    void focusIn();
    void currentValueChanged(float currentValue);
    void currentIndexChanged(int currentIndex);
};

// Array parameter widget

class ArrayParameterWidget : public QWidget
{
    Q_OBJECT

public:
    QGridLayout* gridLayout;

    ArrayParameterWidget(ArrayParameter* theArrayParameter, QWidget* parent = nullptr) : QWidget(parent), arrayParameter{ theArrayParameter }
    {
        gridLayout = new QGridLayout;

        int row = 0;
        int col = 0;

        for (auto element : arrayParameter->values)
        {
            FocusLineEdit* lineEdit = new FocusLineEdit;
            lineEdit->setFixedWidth(50);

            QDoubleValidator* validator = new QDoubleValidator(arrayParameter->min, arrayParameter->max, 5, lineEdit);
            lineEdit->setValidator(validator);
            lineEdit->setText(QString::number(element));

            gridLayout->addWidget(lineEdit, row, col, Qt::AlignCenter);

            col++;
            if (col == 3)
            {
                row++;
                col = 0;
            }

            lineEdits.push_back(lineEdit);
        }
        for (size_t i = 0; i < arrayParameter->values.size(); i++)
        {
            connect(lineEdits[i], &FocusLineEdit::returnPressed, [&arrayParameter = this->arrayParameter, &lineEdits = this->lineEdits, i]()
            {
                arrayParameter->values[i] = lineEdits[i]->text().toFloat(); arrayParameter->setValues();
            });
            connect(lineEdits[i], &FocusLineEdit::focusOut, [&arrayParameter = this->arrayParameter, &lineEdits = this->lineEdits, i]()
            {
                lineEdits[i]->setText(QString::number(arrayParameter->values[i]));
            });
        }
    }

protected:
    ArrayParameter* arrayParameter;
    std::vector<FocusLineEdit*> lineEdits;
};

// Kernel parameter widget

class KernelParameterWidget : public ArrayParameterWidget
{
    Q_OBJECT

public:
    QPushButton* normalizePushButton;
    QComboBox* presetsComboBox;

    KernelParameterWidget(KernelParameter* theKernelParameter, QWidget* parent = nullptr) : ArrayParameterWidget(theKernelParameter, parent), kernelParameter{ theKernelParameter }
    {
        normalizePushButton = new QPushButton("Normalize");
        normalizePushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        normalizePushButton->setVisible(kernelParameter->normalize);

        connect(normalizePushButton, &QPushButton::clicked, [=]()
            {
                float sum = 0.0;
                for (auto element : kernelParameter->values)
                    sum += fabs(element);
                if (sum > 0)
                {
                    for (size_t i = 0; i < kernelParameter->values.size(); i++)
                    {
                        kernelParameter->values[i] /= sum;
                        lineEdits[i]->setText(QString::number(kernelParameter->values[i]));
                    }

                    kernelParameter->setValues();
                }
            });

        presets = {
            { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f }, // Identity
            { 0.0f, -1.0f, 0.0f, -1.0f, 4.0f, -1.0f, 0.0f, -1.0f, 0.0f }, // Soft edge detection
            { -1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f }, // Hard edge detection
            { 0.0f, -1.0f, 0.0f, -1.0f, 5.0f, -1.0f, 0.0f, -1.0f, 0.0f }, // Sharpen
            { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f }, // Box blur
            { 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f, 2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f } // Gaussian blur
        };

        presetsComboBox = new QComboBox;
        presetsComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        presetsComboBox->addItem("Identity");
        presetsComboBox->addItem("Laplacian");
        presetsComboBox->addItem("Laplacian diagonals");
        presetsComboBox->addItem("Sharpen");
        presetsComboBox->addItem("Box blur");
        presetsComboBox->addItem("Gaussian blur");
        presetsComboBox->setCurrentIndex(0);

        connect(presetsComboBox, QOverload<int>::of(&QComboBox::activated), [&kernelParameter = this->kernelParameter, &lineEdits = this->lineEdits, &presets = this->presets](int index)
        {
            for (size_t i = 0; i < presets[index].size(); i++)
                lineEdits[i]->setText(QString::number(presets[index][i]));

            kernelParameter->values = presets[index];
            kernelParameter->setValues();
        });
    }

private:
    KernelParameter* kernelParameter;
    std::vector<std::vector<float>> presets;
};

// Matrix parameter widget

class MatrixParameterWidget : public ArrayParameterWidget
{
    Q_OBJECT

public:
    QComboBox* presetsComboBox;

    MatrixParameterWidget(MatrixParameter* theMatrixParameter, QWidget* parent = nullptr) : ArrayParameterWidget(theMatrixParameter, parent), matrixParameter{ theMatrixParameter }
    {
        presets = {
            { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f }, // RGB to RGB
            { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }, // RGB to GRB
            { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f }, // RGB to BGR
            { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f }, // RGB to RBG
            { 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f }, // RGB to BRG
            { 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f }  // RGB to GBR
        };

        presetsComboBox = new QComboBox;
        presetsComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        presetsComboBox->addItem("RGB to RGB");
        presetsComboBox->addItem("RGB to GRB");
        presetsComboBox->addItem("RGB to BGR");
        presetsComboBox->addItem("RGB to RBG");
        presetsComboBox->addItem("RGB to BRG");
        presetsComboBox->addItem("RGB to GBR");

        connect(presetsComboBox, QOverload<int>::of(&QComboBox::activated), [&matrixParameter = this->matrixParameter, &lineEdits = this->lineEdits, &presets = this->presets](int index)
        {
            for (size_t i = 0; i < presets[index].size(); i++)
                lineEdits[i]->setText(QString::number(presets[index][i]));

            matrixParameter->values = presets[index];
            matrixParameter->setValues();
        });
    }

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

    PolarKernelParameterWidget(PolarKernelParameter* thePolarKernelParameter, QWidget* parent = nullptr) : QWidget(parent), polarKernelParameter{ thePolarKernelParameter }
    {
        // Polar kernel plot

        plot = new PolarKernelPlot("Polar kernel");
              
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
            setGeometryPlotData();
            setKernelValuesPlotData();
        }
        
        QFormLayout* geometryFormLayout = new QFormLayout;
        geometryFormLayout->addRow("#Elements:", numElementsLineEdit);
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
        vBoxLayout->addLayout(plot->layout);

        // Signals + Slots

        connect(selectKernelComboBox, QOverload<int>::of(&QComboBox::activated), [&](int index)
        {
            kernelIndex = index;
            setLineEditTexts();
            setKernelValuesPlotData();
        });

        connect(addKernelPushButton, &QPushButton::pressed, [=]()
        {
            if (polarKernelParameter->polarKernels.size() < 5)
            {
                polarKernelParameter->polarKernels.push_back(new PolarKernel(*polarKernelParameter->polarKernels.back()));
                polarKernelParameter->setValues();

                kernelIndex = static_cast<int>(polarKernelParameter->polarKernels.size()) - 1;

                selectKernelComboBox->addItem(QString::number(kernelIndex + 1));
                selectKernelComboBox->setCurrentIndex(kernelIndex);

                setLineEditTexts();
                setGeometryPlotData();
                setKernelValuesPlotData();
            }
        });
        connect(removeKernelPushButton, &QPushButton::pressed, [=]()
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
                setGeometryPlotData();
                setKernelValuesPlotData();
            }
        });

        connect(numElementsLineEdit, &FocusLineEdit::returnPressed, [&]()
        {
            polarKernelParameter->polarKernels[kernelIndex]->numElements = numElementsLineEdit->text().toInt();
            polarKernelParameter->setValues();
            setGeometryPlotData();
            setKernelValuesPlotData();
        });
        connect(numElementsLineEdit, &FocusLineEdit::focusOut, [&polarKernelParameter = this->polarKernelParameter, &kernelIndex = this->kernelIndex, &numElementsLineEdit = this->numElementsLineEdit]()
        {
            numElementsLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->numElements));
        });

        connect(centerElementLineEdit, &FocusLineEdit::returnPressed, [&polarKernelParameter = this->polarKernelParameter, &centerElementLineEdit = this->centerElementLineEdit]()
        {
            polarKernelParameter->centerElement = centerElementLineEdit->text().toFloat();
            polarKernelParameter->setValues();
        });
        connect(centerElementLineEdit, &FocusLineEdit::focusOut, [&polarKernelParameter = this->polarKernelParameter, &centerElementLineEdit = this->centerElementLineEdit]()
        {
            centerElementLineEdit->setText(QString::number(polarKernelParameter->centerElement));
        });

        connect(radiusLineEdit, &FocusLineEdit::returnPressed, [&]()
        {
            polarKernelParameter->polarKernels[kernelIndex]->radius = radiusLineEdit->text().toFloat();
            polarKernelParameter->setValues();
            setGeometryPlotData();
        });
        connect(radiusLineEdit, &FocusLineEdit::focusOut, [&polarKernelParameter = this->polarKernelParameter, &kernelIndex = this->kernelIndex, &radiusLineEdit = this->radiusLineEdit]()
        {
            radiusLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->radius));
        });

        connect(initialAngleLineEdit, &FocusLineEdit::returnPressed, [&]()
        {
            polarKernelParameter->polarKernels[kernelIndex]->initialAngle = initialAngleLineEdit->text().toFloat();
            polarKernelParameter->setValues();
            setGeometryPlotData();
        });
        connect(initialAngleLineEdit, &FocusLineEdit::focusOut, [&polarKernelParameter = this->polarKernelParameter, &kernelIndex = this->kernelIndex, &initialAngleLineEdit = this->initialAngleLineEdit]()
        {
            initialAngleLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->initialAngle));
        });

        connect(frequencyLineEdit, &FocusLineEdit::returnPressed, [&]()
        {
            polarKernelParameter->polarKernels[kernelIndex]->frequency = frequencyLineEdit->text().toFloat();
            polarKernelParameter->setValues();
            setKernelValuesPlotData();
        });
        connect(frequencyLineEdit, &FocusLineEdit::focusOut, [&polarKernelParameter = this->polarKernelParameter, &kernelIndex = this->kernelIndex, &frequencyLineEdit = this->frequencyLineEdit]()
        {
            frequencyLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->frequency));
        });

        connect(phaseLineEdit, &FocusLineEdit::returnPressed, [&]()
        {
            polarKernelParameter->polarKernels[kernelIndex]->phase = phaseLineEdit->text().toFloat();
            polarKernelParameter->setValues();
            setKernelValuesPlotData();
        });
        connect(phaseLineEdit, &FocusLineEdit::focusOut, [&polarKernelParameter = this->polarKernelParameter, &kernelIndex = this->kernelIndex, &phaseLineEdit = this->phaseLineEdit]()
        {
            phaseLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->phase));
        });

        connect(minimumLineEdit, &FocusLineEdit::returnPressed, [&]()
        {
            polarKernelParameter->polarKernels[kernelIndex]->minimum = minimumLineEdit->text().toFloat();
            polarKernelParameter->setValues();
            setKernelValuesPlotData();
        });
        connect(minimumLineEdit, &FocusLineEdit::focusOut, [&polarKernelParameter = this->polarKernelParameter, &kernelIndex = this->kernelIndex, &minimumLineEdit = this->minimumLineEdit]()
        {
            minimumLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->minimum));
        });

        connect(maximumLineEdit, &FocusLineEdit::returnPressed, [&]()
        {
            polarKernelParameter->polarKernels[kernelIndex]->maximum = maximumLineEdit->text().toFloat();
            polarKernelParameter->setValues();
            setKernelValuesPlotData();
        });
        connect(maximumLineEdit, &FocusLineEdit::focusOut, [&polarKernelParameter = this->polarKernelParameter, &kernelIndex = this->kernelIndex, &maximumLineEdit = this->maximumLineEdit]()
        {
            maximumLineEdit->setText(QString::number(polarKernelParameter->polarKernels[kernelIndex]->maximum));
        });
    }

    ~PolarKernelParameterWidget()
    {
        delete plot;
    }

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
    PolarKernelPlot* plot;
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

    void setGeometryPlotData()
    {
        plot->setGeometryData(polarKernelParameter->polarKernels);
    }

    void setKernelValuesPlotData()
    {
        plot->setKernelValuesData(polarKernelParameter->polarKernels[kernelIndex]);
    }
};

// Operations widget

class OperationsWidget : public QWidget
{
    Q_OBJECT

public:
    std::vector<FloatParameterWidget*> floatParameterWidget;
    std::vector<PolarKernelParameterWidget*> polarKernelParameterWidget;

    OperationsWidget(ImageOperation* operation)
    {
        QVBoxLayout* vBoxLayout = new QVBoxLayout;
        vBoxLayout->setAlignment(Qt::AlignCenter);
        QFormLayout* formLayout = new QFormLayout;

        for (auto parameter : operation->getIntParameters())
        {
            if (parameter->isOdd)
            {
                IntOddParameterWidget* widget = new IntOddParameterWidget(parameter, this);
                formLayout->addRow(parameter->name + ":", widget->lineEdit);
            }
            else
            {
                IntParameterWidget* widget = new IntParameterWidget(parameter, this);
                formLayout->addRow(parameter->name + ":", widget->lineEdit);
            }
        }
        for (auto parameter : operation->getFloatParameters())
        {
            FloatParameterWidget* widget = new FloatParameterWidget(parameter, 100000, this);
            floatParameterWidget.push_back(widget);
            formLayout->addRow(parameter->name + ":", widget->lineEdit);
        }
        for (auto parameter : operation->getBoolParameters())
        {
            BoolParameterWidget* widget = new BoolParameterWidget(parameter, this);
            formLayout->addRow("", widget->checkBox);
        }
        for (auto parameter : operation->getOptionsIntParameters())
        {
            OptionsParameterWidget<int>* widget = new OptionsParameterWidget<int>(parameter, this);
            formLayout->addRow(parameter->name + ":", widget->comboBox);
        }
        for (auto parameter : operation->getOptionsGLenumParameters())
        {
            OptionsParameterWidget<GLenum>* widget = new OptionsParameterWidget<GLenum>(parameter, this);
            formLayout->addRow(parameter->name + ":", widget->comboBox);
        }
        if (operation->getKernelParameter())
        {
            KernelParameterWidget* widget = new KernelParameterWidget(operation->getKernelParameter(), this);
            vBoxLayout->addWidget(new QLabel(operation->getKernelParameter()->name + ":"));
            vBoxLayout->addLayout(widget->gridLayout);
            vBoxLayout->addWidget(widget->normalizePushButton);
            formLayout->addRow("Presets:", widget->presetsComboBox);
        }
        if (operation->getMatrixParameter())
        {
            MatrixParameterWidget* widget = new MatrixParameterWidget(operation->getMatrixParameter(), this);
            vBoxLayout->addWidget(new QLabel(operation->getMatrixParameter()->name + ":"));
            vBoxLayout->addLayout(widget->gridLayout);
            formLayout->addRow("Presets:", widget->presetsComboBox);
        }
        if (operation->getPolarKernelParameter())
        {
            PolarKernelParameterWidget* widget = new PolarKernelParameterWidget(operation->getPolarKernelParameter(), this);
            polarKernelParameterWidget.push_back(widget);
            vBoxLayout->addLayout(widget->vBoxLayout);
        }

        QCheckBox* enabledCheckBox = new QCheckBox("Enabled");
        enabledCheckBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        enabledCheckBox->setChecked(operation->isEnabled());

        formLayout->addWidget(enabledCheckBox);

        vBoxLayout->addLayout(formLayout);

        setLayout(vBoxLayout);

        connect(enabledCheckBox, &QCheckBox::stateChanged, [=](int state)
        {
            operation->enable(state == Qt::Checked);
        });
    }

    ~OperationsWidget()
    {
        for (auto& widget : polarKernelParameterWidget)
            delete widget;
    }
};
