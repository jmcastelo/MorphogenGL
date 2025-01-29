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

#include "widgets/intwidget.h"
#include "widgets/intoddwidget.h"
#include "widgets/floatwidget.h"
#include "widgets/optionswidget.h"
#include "widgets/kernelwidget.h"
#include "widgets/matrixwidget.h"

#include "parameter.h"
#include "imageoperations.h"

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

        parametersLayout->addLayout(formLayout);

        QGroupBox* parametersGroupBox = new QGroupBox(operation->getName());
        parametersGroupBox->setLayout(parametersLayout);

        mainLayout->addWidget(parametersGroupBox);

        // Selected float or int parameter

        if (!slideableFloatParameterWidgets.empty() || !slideableIntParameterWidgets.empty())
        {
            FocusSlider* selectedParameterSlider = new FocusSlider(Qt::Horizontal);
            selectedParameterSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
            selectedParameterSlider->setRange(0, 10000);

            midiLinkButton = new QPushButton();
            midiLinkButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            midiLinkButton->setCheckable(true);
            midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");
            midiLinkButton->setVisible(midiEnabled);

            FocusLineEdit* selectedParameterMinLineEdit = new FocusLineEdit;
            selectedParameterMinLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            selectedParameterMinLineEdit->setPlaceholderText("Minimum");

            QDoubleValidator* selectedParameterMinValidator = new QDoubleValidator(selectedParameterMinLineEdit);
            selectedParameterMinValidator->setDecimals(10);
            selectedParameterMinLineEdit->setValidator(selectedParameterMinValidator);

            FocusLineEdit* selectedParameterMaxLineEdit = new FocusLineEdit;
            selectedParameterMaxLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            selectedParameterMaxLineEdit->setPlaceholderText("Maximum");

            QDoubleValidator* selectedParameterMaxValidator = new QDoubleValidator(selectedParameterMaxLineEdit);
            selectedParameterMaxValidator->setDecimals(10);
            selectedParameterMaxLineEdit->setValidator(selectedParameterMaxValidator);

            QHBoxLayout* sliderLayout = new QHBoxLayout;
            sliderLayout->addWidget(midiLinkButton);
            sliderLayout->addWidget(selectedParameterSlider);

            QHBoxLayout* minMaxLayout = new QHBoxLayout;
            minMaxLayout->setAlignment(Qt::AlignJustify);
            minMaxLayout->addWidget(selectedParameterMinLineEdit);
            minMaxLayout->addWidget(selectedParameterMaxLineEdit);

            QVBoxLayout* selectedParameterVBoxLayout = new QVBoxLayout;
            selectedParameterVBoxLayout->setSizeConstraint(QLayout::SetMaximumSize);
            selectedParameterVBoxLayout->addLayout(sliderLayout);
            selectedParameterVBoxLayout->addLayout(minMaxLayout);

            QGroupBox* selectedParameterGroupBox = new QGroupBox("No parameter selected");
            selectedParameterGroupBox->setLayout(selectedParameterVBoxLayout);

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

    void toggleMidiButton(bool show)
    {
        midiLinkButton->setVisible(show);
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

        connect(number, QOverload<float>::of(&Number<T>::currentValueChanged), this, [=](T currentValue)
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
