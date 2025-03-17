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



#include "imageoperation.h"
#include "widgets/uniformwidget.h"
#include "widgets/uniformmat4widget.h"
#include "widgets/optionswidget.h"
#include "gridwidget.h"
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
#include <QOpenGLFunctions>



class OperationWidget : public QFrame
{
    Q_OBJECT

public:
    explicit OperationWidget(ImageOperation* operation, bool midiEnabled, QWidget* parent = nullptr);
    ~OperationWidget();

    void setup();

    void toggleEnableButton(bool checked);
    void toggleMidiButton(bool show);

signals:
    void linkWait(Number<float>* number);
    void linkWait(Number<int>* number);
    void linkWait(Number<unsigned int>* number);

    void linkBreak(Number<float>* number);
    void linkBreak(Number<int>* number);
    void linkBreak(Number<unsigned int>* number);

public slots:
    void recreate();

protected:
    void closeEvent(QCloseEvent* event) override;
    void focusInEvent(QFocusEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    ImageOperation* mOperation;
    bool mMidiEnabled;

    QVBoxLayout* mainLayout;

    QFrame* headerWidget;

    QPushButton* enableButton;
    QPushButton* editButton;
    QPushButton* toggleButton;
    QLabel* opNameLabel;
    QLineEdit* opNameLineEdit;

    QWidget* bodyWidget;

    GridWidget* gridWidget;

    QList<ParameterWidget<float>*> floatParamWidgets;
    QList<ParameterWidget<int>*> intParamWidgets;
    QList<ParameterWidget<unsigned int>*> uintParamWidgets;
    QList<ParameterWidget<float>*> mat4ParamWidgets;

    QList<UniformParameterWidget<float>*> uniformFloatParamWidgets;
    QList<UniformParameterWidget<int>*> uniformIntParamWidgets;
    QList<UniformParameterWidget<unsigned int>*> uniformUintParamWidgets;
    QList<UniformMat4ParameterWidget*> uniformMat4ParamWidgets;

    QList<OptionsParameterWidget<GLenum>*> glenumOptionsWidgets;

    QDial* selParamDial;
    FocusLineEdit* selParamMinLineEdit;
    FocusLineEdit* selParamMaxLineEdit;
    QValidator* minValidator = nullptr;
    QValidator* maxValidator = nullptr;

    FocusLineEdit* selParamInfLineEdit;
    FocusLineEdit* selParamSupLineEdit;
    QValidator* infValidator = nullptr;
    QValidator* supValidator = nullptr;

    QLineEdit* paramNameLineEdit;

    QComboBox* layoutComboBox;
    QComboBox* mat4TypeComboBox;

    QPushButton* midiLinkButton;

    QGroupBox* selParamGroupBox;

    OperationBuilder* mOpBuilder;
    bool editMode = false;

    Parameter* lastFocusedParameter;
    QWidget* lastFocusedWidget = nullptr;
    bool lastFocused = false;

    template <typename T>
    void setFocusedWidget(ParameterWidget<T>* widget);

    template <typename T>
    void connectParamWidgets(QList<ParameterWidget<T>*> widgets);

    template <typename T>
    void connectUniformParamWidgets(QList<UniformParameterWidget<T>*> widgets);

    void connectUniformFloatParamWidgets();

    void connectUniformMat4ParamWidgets();

    template <typename T>
    void updateSelParamControls(ParameterWidget<T>* widget);

    template <typename T>
    void updateSelParamEditControls(ParameterWidget<T>* widget);

    template <typename T>
    void setValidator(QValidator* validator, QLineEdit* lineEdit, T bottom, T top);

    template <typename T>
    void updateMidiButtons(ParameterWidget<T>* widget);

private slots:
    void updateWidgetRowCol(QWidget* widget, int row, int col);
    void toggleBody(bool visible);
    void toggleEdit(bool state);
    void setEditableWidgets(bool state);
};



#endif // OPERATIONWIDGET_H
