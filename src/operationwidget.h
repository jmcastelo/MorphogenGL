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



#ifndef OPERATIONWIDGET_H
#define OPERATIONWIDGET_H



#include "widgets/uniformwidget.h"
#include "widgets/optionswidget.h"
#include "widgets/uniformmat4widget.h"
#include "gridwidget.h"
#include "parameter.h"
#include "imageoperation.h"
#include "operationbuilder.h"

#include <QWidget>
#include <QString>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QGroupBox>
#include <QDial>
#include <QFrame>



// Operations widget

class OperationWidget : public QWidget
{
    Q_OBJECT

public:
    OperationWidget(ImageOperation* operation, bool midiEnabled, QWidget* parent = nullptr) : QWidget(parent)
    {
        mOpBuilder = new OperationBuilder(operation, this);
        mOpBuilder->setVisible(false);

        mainLayout = new QVBoxLayout;
        mainLayout->setSizeConstraint(QLayout::SetFixedSize);

        // Header widget

        headerWidget = new QFrame;
        headerWidget->setFrameStyle(QFrame::Box | QFrame::Plain);
        headerWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

        // Enable button

        enableButton = new QPushButton;
        enableButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        enableButton->setCheckable(true);

        // Edit button

        editButton = new QPushButton;
        editButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        editButton->setCheckable(true);
        editButton->setChecked(false);
        editButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/applications-development.png); }");

        connect(editButton, &QPushButton::clicked, mOpBuilder, &QWidget::setVisible);

        // Operation name label

        opNameLabel = new QLabel;
        opNameLabel->setMargin(10);
        opNameLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

        // Toggle body button

        toggleButton = new QPushButton;
        toggleButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        toggleButton->setCheckable(true);
        toggleButton->setChecked(false);

        connect(toggleButton, &QPushButton::clicked, this, &OperationWidget::toggleBody);

        QHBoxLayout* headerLayout = new QHBoxLayout;
        headerLayout->addWidget(enableButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
        headerLayout->addWidget(editButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
        headerLayout->addWidget(opNameLabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
        headerLayout->addWidget(toggleButton, 0, Qt::AlignRight | Qt::AlignVCenter);

        headerWidget->setLayout(headerLayout);
        mainLayout->addWidget(headerWidget, 0, Qt::AlignTop | Qt::AlignLeft);

        // Body widget

        bodyWidget = new QWidget;
        bodyWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        mainLayout->addWidget(bodyWidget, 0, Qt::AlignTop | Qt::AlignHCenter);

        QVBoxLayout* bodyLayout = new QVBoxLayout;
        bodyWidget->setLayout(bodyLayout);

        // Grid widget containing parameter widgets

        gridWidget = new GridWidget;
        gridWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        connect(gridWidget, &GridWidget::itemRowColChanged, this, &OperationWidget::updateWidgetRowCol);

        bodyLayout->addWidget(gridWidget, 0, Qt::AlignTop | Qt::AlignLeft);

        selParamDial = new QDial;
        selParamDial->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        selParamDial->setRange(0, 100'000);

        midiLinkButton = new QPushButton;
        midiLinkButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        midiLinkButton->setCheckable(true);
        midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");

        selectedParameterMinLineEdit = new FocusLineEdit;
        selectedParameterMinLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        selectedParameterMinLineEdit->setPlaceholderText("Minimum");

        selectedParameterMaxLineEdit = new FocusLineEdit;
        selectedParameterMaxLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        selectedParameterMaxLineEdit->setPlaceholderText("Maximum");

        QHBoxLayout* selParamLayout = new QHBoxLayout;
        selParamLayout->addWidget(midiLinkButton);
        selParamLayout->addWidget(selParamDial);
        selParamLayout->addWidget(selectedParameterMinLineEdit);
        selParamLayout->addWidget(selectedParameterMaxLineEdit);

        selectedParameterGroupBox = new QGroupBox("No parameter selected");
        selectedParameterGroupBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
        selectedParameterGroupBox->setLayout(selParamLayout);
        selectedParameterGroupBox->setVisible(false);

        bodyLayout->addWidget(selectedParameterGroupBox, 0, Qt::AlignTop | Qt::AlignHCenter);

        toggleBody(false);

        setLayout(mainLayout);

        setup(operation, midiEnabled);
    }



    ~OperationWidget()
    {
        delete mOpBuilder;
    }



    void setup(ImageOperation* operation, bool midiEnabled)
    {
        // Parameter widgets

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

        gridWidget->optimizeLayout();

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

        // Operation name label

        opNameLabel->setText(operation->name());

        // Midi link button

        midiLinkButton->setVisible(midiEnabled);

        // Enable button

        toggleEnableButton(operation->isEnabled());

        disconnect(enableButton, &QPushButton::toggled, this, nullptr);

        connect(enableButton, &QPushButton::toggled, this, [=, this](bool checked){
            operation->enable(checked);
            setEnableButtonStyle(checked);
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
                lastFocusedWidget = widget->lastFocusedWidget();
                lastFocused = true;
            });

            /*connect(widget, &ParameterWidget<T>::focusOut, this, [=, this](){
                emit focusOut(this);
            });*/
        }
    }

    void recreate(ImageOperation* operation, bool midiEnabled)
    {
        minValidator = nullptr;
        maxValidator = nullptr;

        gridWidget->clear();

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
        setEnableButtonStyle(checked);
    }

    void setEnableButtonStyle(bool enabled)
    {
        if (enabled)
            enableButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-green.png); }");
        else
            enableButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");
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
    //void focusIn(QWidget* widget);
    //void focusOut(QWidget* widget);

    void sizeChanged();

    void linkWait(Number<float>* number);
    void linkWait(Number<int>* number);
    void linkWait(Number<unsigned int>* number);

    void linkBreak(Number<float>* number);
    void linkBreak(Number<int>* number);
    void linkBreak(Number<unsigned int>* number);

protected:
    void closeEvent(QCloseEvent* event) override
    {
        mOpBuilder->close();
        event->accept();
    }

    void resizeEvent(QResizeEvent* event) override
    {
        QWidget::resizeEvent(event);
        emit sizeChanged();
    }

private:
    QVBoxLayout* mainLayout;

    QFrame* headerWidget;

    QPushButton* enableButton;
    QPushButton* editButton;
    QPushButton* toggleButton;
    QLabel* opNameLabel;

    QWidget* bodyWidget;

    GridWidget* gridWidget;

    QList<ParameterWidget<float>*> floatParameterWidgets;
    QList<ParameterWidget<int>*> intParameterWidgets;
    QList<ParameterWidget<unsigned int>*> uintParameterWidgets;
    QList<OptionsParameterWidget<GLenum>*> glenumOptionsWidgets;

    QDial* selParamDial;
    FocusLineEdit* selectedParameterMinLineEdit;
    FocusLineEdit* selectedParameterMaxLineEdit;
    QValidator* minValidator = nullptr;
    QValidator* maxValidator = nullptr;

    QPushButton* midiLinkButton;

    QGroupBox* selectedParameterGroupBox;

    OperationBuilder* mOpBuilder;

    QWidget* lastFocusedWidget = nullptr;
    bool lastFocused = false;

    template <class T>
    void updateSelectedParameterControls(ParameterWidget<T>* widget)
    {
        Number<T>* number = widget->selectedNumber();

        // Slider

        selParamDial->disconnect();
        selParamDial->setRange(0, number->indexMax());
        selParamDial->setValue(number->index());

        connect(selParamDial, &QAbstractSlider::sliderMoved, widget, &ParameterWidget<T>::setValueFromIndex);
        connect(number, &Number<T>::indexChanged, selParamDial, &QAbstractSlider::setValue);

        // Focus

        /*connect(selParamDial, &FocusSlider::focusIn, this, [=, this](){
            lastFocusedWidget = selParamDial;
            lastFocused = true;
            emit focusIn(this);
        });
        connect(selParamDial, &FocusSlider::focusOut, this, [=, this](){
            emit focusOut(this);
        });*/

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
            lastFocusedWidget = selectedParameterMinLineEdit;
            lastFocused = true;
            //emit focusIn(this);
        });
        /*connect(selectedParameterMinLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
            emit focusOut(this);
        });*/

        connect(selectedParameterMaxLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
            lastFocusedWidget = selectedParameterMaxLineEdit;
            lastFocused = true;
            //emit focusIn(this);
        });
        /*connect(selectedParameterMaxLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
            emit focusOut(this);
        });*/

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

    void toggleBody(bool visible)
    {
        bodyWidget->setVisible(visible);

        if (visible)
        {
            toggleButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/go-up.png); }");
            resize(headerWidget->size() + bodyWidget->size());
        }
        else
        {
            toggleButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/go-down.png); }");
            resize(headerWidget->size());
        }
    }
};



#endif // OPERATIONWIDGET_H
