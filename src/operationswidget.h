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



#ifndef OPERATIONSWIDGET_H
#define OPERATIONSWIDGET_H



#include "widgets/uniformwidget.h"
#include "widgets/optionswidget.h"
#include "widgets/uniformmat4widget.h"
#include "gridwidget.h"
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
    OperationsWidget(ImageOperation* operation, bool midiEnabled, QWidget* parent = nullptr) : QFrame(parent)
    {
        mainLayout = new QVBoxLayout;
        //mainLayout->setAlignment(Qt::AlignCenter);
        //mainLayout->setSizeConstraint(QLayout::SetFixedSize);
        mainLayout->setSizeConstraint(QLayout::SetMaximumSize);

        gridWidget = new GridWidget;
        gridWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        connect(gridWidget, &GridWidget::itemRowColChanged, this, &OperationsWidget::updateWidgetRowCol);

        mainLayout->addWidget(gridWidget);

        selectedParameterSlider = new FocusSlider(Qt::Horizontal);
        selectedParameterSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        selectedParameterSlider->setRange(0, 100'000);

        midiLinkButton = new QPushButton();
        midiLinkButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        midiLinkButton->setCheckable(true);
        midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");

        selectedParameterMinLineEdit = new FocusLineEdit;
        selectedParameterMinLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        selectedParameterMinLineEdit->setPlaceholderText("Minimum");

        selectedParameterMaxLineEdit = new FocusLineEdit;
        selectedParameterMaxLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        selectedParameterMaxLineEdit->setPlaceholderText("Maximum");

        QHBoxLayout* sliderLayout = new QHBoxLayout;
        sliderLayout->addWidget(midiLinkButton);
        sliderLayout->addWidget(selectedParameterSlider);

        QHBoxLayout* minMaxLayout = new QHBoxLayout;
        minMaxLayout->setAlignment(Qt::AlignJustify);
        minMaxLayout->addWidget(selectedParameterMinLineEdit);
        minMaxLayout->addWidget(selectedParameterMaxLineEdit);

        QVBoxLayout* selectedParameterVBoxLayout = new QVBoxLayout;
        //selectedParameterVBoxLayout->setSizeConstraint(QLayout::SetMaximumSize);
        selectedParameterVBoxLayout->addLayout(sliderLayout);
        selectedParameterVBoxLayout->addLayout(minMaxLayout);

        selectedParameterGroupBox = new QGroupBox("No parameter selected");
        selectedParameterGroupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        selectedParameterGroupBox->setLayout(selectedParameterVBoxLayout);
        selectedParameterGroupBox->setVisible(false);

        mainLayout->addWidget(selectedParameterGroupBox);

        // Enable button

        enableButton = new FocusPushButton;
        enableButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        enableButton->setFixedHeight(10);
        enableButton->setCheckable(true);

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

        setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        setFrameStyle(QFrame::Panel | QFrame::Plain);
        setLineWidth(0);
        setMidLineWidth(0);

        setup(operation, midiEnabled);
    }

    void setup(ImageOperation* operation, bool midiEnabled)
    {
        // Parameter widgets

        gridWidget->clear();

        foreach (auto parameter, operation->uniformParameters<float>())
        {
            UniformParameterWidget<float>* widget = new UniformParameterWidget<float>(parameter);
            gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());
            floatParameterWidgets.append(widget);
        }

        foreach (auto parameter, operation->uniformParameters<int>())
        {
            UniformParameterWidget<int>* widget = new UniformParameterWidget<int>(parameter);
            gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());
            intParameterWidgets.append(widget);
        }

        foreach (auto parameter, operation->uniformParameters<unsigned int>())
        {
            UniformParameterWidget<unsigned int>* widget = new UniformParameterWidget<unsigned int>(parameter);
            gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());
            uintParameterWidgets.append(widget);
        }

        foreach (auto parameter, operation->optionsParameters<GLenum>())
        {
            OptionsParameterWidget<GLenum>* widget = new OptionsParameterWidget<GLenum>(parameter);
            gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());
            glenumOptionsWidgets.append(widget);
        }

        foreach (auto parameter, operation->mat4UniformParameters())
        {
            UniformMat4ParameterWidget* widget = new UniformMat4ParameterWidget(parameter);
            gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());
            floatParameterWidgets.append(widget);
        }

        // Selected parameter widget

        if (!floatParameterWidgets.empty() || !intParameterWidgets.empty() || !uintParameterWidgets.empty())
        {
            selectedParameterGroupBox->setVisible(true);

            connectUniformParameterWidgets<float>(floatParameterWidgets);
            connectUniformParameterWidgets<int>(intParameterWidgets);
            connectUniformParameterWidgets<unsigned int>(uintParameterWidgets);

            lastFocusedWidget = nullptr;
        }
        else
        {
            selectedParameterGroupBox->setVisible(false);
        }

        // Midi link button

        midiLinkButton->setVisible(midiEnabled);

        // Enable button

        enableButton->setChecked(operation->isEnabled());

        if (operation->isEnabled())
            enableButton->setStyleSheet("background-color: rgb(0, 255, 0); color: rgb(255, 255, 255)");
        else
            enableButton->setStyleSheet("background-color: rgb(32, 32, 32); color: rgb(255, 255, 255)");

        disconnect(enableButton, &QPushButton::toggled, this, nullptr);

        connect(enableButton, &QPushButton::toggled, this, [=, this](bool checked){
            operation->enable(checked);

            if (checked)
                enableButton->setStyleSheet("background-color: rgb(0, 255, 0); color: rgb(255, 255, 255)");
            else
                enableButton->setStyleSheet("background-color: rgb(32, 32, 32); color: rgb(255, 255, 255)");

            emit enableButtonToggled();
        });
    }

    template <typename T>
    void connectUniformParameterWidgets(QList<ParameterWidget<T>*> widgets)
    {
        foreach (auto widget, widgets)
        {
            connect(widget, &ParameterWidget<T>::focusIn, this, [=, this](){
                updateSelectedParameterControls<T>(widget);
                updateMidiButtons<T>(widget);
            });

            connect(widget, &ParameterWidget<T>::focusIn, this, [=, this](){
                setLineWidth(1);
                lastFocusedWidget = widget->lastFocusedWidget();
                lastFocused = true;
                emit focusIn(this);
            });

            connect(widget, &ParameterWidget<T>::focusOut, this, [=, this](){
                setLineWidth(0);
                emit focusOut(this);
            });
        }
    }

    void recreate(ImageOperation* operation, bool midiEnabled)
    {
        minValidator = nullptr;
        maxValidator = nullptr;

        qDeleteAll(floatParameterWidgets);
        floatParameterWidgets.clear();

        qDeleteAll(intParameterWidgets);
        intParameterWidgets.clear();

        qDeleteAll(uintParameterWidgets);
        uintParameterWidgets.clear();

        qDeleteAll(glenumOptionsWidgets);
        glenumOptionsWidgets.clear();

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
        if (lastFocusedWidget)
            return lastFocusedWidget->hasFocus();
        return false;
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
    void linkWait(Number<unsigned int>* number);

    void linkBreak(Number<float>* number);
    void linkBreak(Number<int>* number);
    void linkBreak(Number<unsigned int>* number);

private:
    QVBoxLayout* mainLayout;

    GridWidget* gridWidget;

    QList<ParameterWidget<float>*> floatParameterWidgets;
    QList<ParameterWidget<int>*> intParameterWidgets;
    QList<ParameterWidget<unsigned int>*> uintParameterWidgets;
    QList<OptionsParameterWidget<GLenum>*> glenumOptionsWidgets;

    FocusSlider* selectedParameterSlider;
    FocusLineEdit* selectedParameterMinLineEdit;
    FocusLineEdit* selectedParameterMaxLineEdit;
    QValidator* minValidator = nullptr;
    QValidator* maxValidator = nullptr;

    QPushButton* midiLinkButton;

    QGroupBox* selectedParameterGroupBox;

    FocusPushButton* enableButton;

    QWidget* lastFocusedWidget = nullptr;
    bool lastFocused = false;

    template <class T>
    void updateSelectedParameterControls(ParameterWidget<T>* widget)
    {
        Number<T>* number = widget->selectedNumber();

        // Slider

        selectedParameterSlider->disconnect();
        selectedParameterSlider->setRange(0, number->indexMax());
        selectedParameterSlider->setValue(number->index());

        connect(selectedParameterSlider, &QAbstractSlider::sliderMoved, widget, &ParameterWidget<T>::setValueFromIndex);
        connect(number, &Number<T>::indexChanged, selectedParameterSlider, &QAbstractSlider::setValue);

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

        // Minimum

        selectedParameterMinLineEdit->disconnect();

        if (minValidator)
        {
            delete minValidator;
            minValidator = nullptr;
        }

        if (std::is_same<T, float>::value)
            minValidator = new QDoubleValidator(number->inf(), number->sup(), 5, selectedParameterMinLineEdit);
        else if (std::is_same<T, int>::value || std::is_same<T, unsigned int>::value)
            minValidator = new QIntValidator(number->inf(), number->sup(), selectedParameterMinLineEdit);

        if (minValidator)
            selectedParameterMinLineEdit->setValidator(minValidator);

        selectedParameterMinLineEdit->setText(QString::number(number->min()));

        connect(selectedParameterMinLineEdit, &FocusLineEdit::editingFinished, this, [=, this]()
        {
            if (std::is_same<T, float>::value)
                number->setMin(selectedParameterMinLineEdit->text().toFloat());
            else if (std::is_same<T, int>::value)
                number->setMin(selectedParameterMinLineEdit->text().toInt());
            else if (std::is_same<T, unsigned int>::value)
                number->setMin(selectedParameterMinLineEdit->text().toUInt());

            number->setIndex();
        });

        // Maximum

        selectedParameterMaxLineEdit->disconnect();

        if (maxValidator)
        {
            delete maxValidator;
            maxValidator = nullptr;
        }

        if (std::is_same<T, float>::value)
            maxValidator = new QDoubleValidator(number->inf(), number->sup(), 5, selectedParameterMaxLineEdit);
        else if (std::is_same<T, int>::value || std::is_same<T, unsigned int>::value)
            maxValidator = new QIntValidator(number->inf(), number->sup(), selectedParameterMaxLineEdit);

        if (maxValidator)
            selectedParameterMaxLineEdit->setValidator(maxValidator);

        selectedParameterMaxLineEdit->setText(QString::number(number->max()));

        connect(selectedParameterMaxLineEdit, &FocusLineEdit::editingFinished, this, [=, this](){
            if (std::is_same<T, float>::value)
                number->setMax(selectedParameterMaxLineEdit->text().toFloat());
            else if (std::is_same<T, int>::value)
                number->setMax(selectedParameterMaxLineEdit->text().toInt());
            else if (std::is_same<T, unsigned int>::value)
                number->setMax(selectedParameterMaxLineEdit->text().toUInt());

            number->setIndex();
        });

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

        selectedParameterGroupBox->setTitle("Selected parameter: " + widget->name());
    }

    template <typename T>
    void updateMidiButtons(ParameterWidget<T>* widget)
    {
        Number<T>* number = widget->selectedNumber();

        // MIDI Link button

        midiLinkButton->disconnect();

        midiLinkButton->setChecked(number->midiLinked());

        if (number->midiLinked())
            midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-green.png); }");
        else
            midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");

        connect(midiLinkButton, &QPushButton::clicked, this, [=, this](bool checked){
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

private slots:
    void updateWidgetRowCol(QWidget* widget, int row, int col)
    {
        foreach (auto paramWidget, floatParameterWidgets)
        {
            if (paramWidget->widget() == widget)
            {
                paramWidget->setRow(row);
                paramWidget->setCol(col);
                return;
            }
        }
        foreach (auto paramWidget, intParameterWidgets)
        {
            if (paramWidget->widget() == widget)
            {
                paramWidget->setRow(row);
                paramWidget->setCol(col);
                return;
            }
        }
        foreach (auto paramWidget, uintParameterWidgets)
        {
            if (paramWidget->widget() == widget)
            {
                paramWidget->setRow(row);
                paramWidget->setCol(col);
                return;
            }
        }
        foreach (auto paramWidget, glenumOptionsWidgets)
        {
            if (paramWidget->widget() == widget)
            {
                paramWidget->setRow(row);
                paramWidget->setCol(col);
                return;
            }
        }
    }
};



#endif // OPERATIONSWIDGET_H
