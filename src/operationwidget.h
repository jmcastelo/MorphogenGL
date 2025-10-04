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



#include "midisignals.h"
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
#include <QSlider>
#include <QFrame>
#include <QToolBar>
#include <QAction>
#include <QOpenGLFunctions>



class OperationWidget : public QFrame
{
    Q_OBJECT

public:
    explicit OperationWidget(ImageOperation* operation, bool midiEnabled, bool editMode, QWidget* parent = nullptr);
    ~OperationWidget();

    void setup();

    MidiSignals* midiSignals();

signals:
    /*void linkWait(Number<float>* number);
    void linkWait(Number<int>* number);
    void linkWait(Number<unsigned int>* number);

    void linkBreak(Number<float>* number);
    void linkBreak(Number<int>* number);
    void linkBreak(Number<unsigned int>* number);*/

    void outputChanged(bool checked);
    void connectTo();
    void remove();

public slots:
    void recreate();
    void toggleOutputAction(QWidget* widget);
    void toggleMidiButton(bool show);

protected:
    void closeEvent(QCloseEvent* event) override;
    void focusInEvent(QFocusEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    ImageOperation* mOperation;

    bool mMidiEnabled;
    MidiSignals* mMidiSignals;

    QVBoxLayout* mainLayout;

    QWidget* headerWidget;

    QToolBar* headerToolBar;

    QAction* enableAction;
    QAction* outputAction;
    QAction* editAction;
    QAction* toggleBodyAction;

    QLabel* opNameLabel;
    QLineEdit* opNameLineEdit;

    QFrame* separator[2];

    GridWidget* gridWidget;

    QWidget* selParamWidget;

    QList<ParameterWidget<float>*> floatParamWidgets;
    QList<ParameterWidget<int>*> intParamWidgets;
    QList<ParameterWidget<unsigned int>*> uintParamWidgets;
    QList<ParameterWidget<float>*> mat4ParamWidgets;

    QList<UniformParameterWidget<float>*> uniformFloatParamWidgets;
    QList<UniformParameterWidget<int>*> uniformIntParamWidgets;
    QList<UniformParameterWidget<unsigned int>*> uniformUintParamWidgets;

    QList<UniformMat4ParameterWidget*> uniformMat4ParamWidgets;

    QList<OptionsParameterWidget<GLenum>*> glenumOptionsWidgets;

    QSlider* selParamSlider;
    FocusLineEdit* selParamMinLineEdit;
    FocusLineEdit* selParamMaxLineEdit;
    QValidator* minValidator = nullptr;
    QValidator* maxValidator = nullptr;

    FocusLineEdit* selParamInfLineEdit;
    FocusLineEdit* selParamSupLineEdit;
    QValidator* infValidator = nullptr;
    QValidator* supValidator = nullptr;

    QLabel* paramNameLabel;
    QLineEdit* paramNameLineEdit;

    QComboBox* layoutComboBox;
    QComboBox* mat4TypeComboBox;

    QPushButton* midiLinkButton;

    QPushButton* removePresetButton;
    QPushButton* addPresetButton;
    QLineEdit* presetNameLineEdit;

    OperationBuilder* mOpBuilder;
    bool mEditMode;

    Parameter* lastFocusedParameter = nullptr;
    QWidget* lastFocusedWidget = nullptr;

    QMetaObject::Connection layoutComboBoxConn;
    QMetaObject::Connection mat4TypeComboBoxConn;
    QMetaObject::Connection paramNameLineEditConn;
    QList<QMetaObject::Connection> selParamSliderConns;
    QList<QMetaObject::Connection> selParamInfLineEditConns;
    QList<QMetaObject::Connection> selParamMinLineEditConns;
    QList<QMetaObject::Connection> selParamMaxLineEditConns;
    QList<QMetaObject::Connection> selParamSupLineEditConns;
    QMetaObject::Connection removePresetButtonConn;
    QMetaObject::Connection addPresetButtonConn;
    QList<QMetaObject::Connection> midiLinkButtonConns;


    template <typename T>
    void setFocusedWidget(ParameterWidget<T>* widget);

    template <typename T>
    void connectParamWidgets(QList<ParameterWidget<T>*> widgets);

    template <typename T>
    void connectUniformParamWidgets(QList<UniformParameterWidget<T>*> widgets);

    void connectUniformFloatParamWidgets();

    void connectUniformMat4ParamWidgets();

    template <typename T>
    void setSelParamNameWidgets(ParameterWidget<T>* widget);

    template <typename T>
    void updateSelParamControls(ParameterWidget<T>* widget);

    template <typename T>
    void updateSelParamEditControls(ParameterWidget<T>* widget);

    template <typename T>
    void setValidator(QValidator* validator, QLineEdit* lineEdit, T bottom, T top);

    template <typename T>
    void updateMidiButton(ParameterWidget<T>* widget);

    void toggleSelParamWidgets(bool visible);

    void addInterpolation();
    void removeInterpolation();

private slots:
    void updateWidgetRowCol(QWidget* widget, int row, int col);
    void toggleBody(bool visible);
    void toggleEditMode(bool mode);
    void enableOperation(bool checked);
};



#endif // OPERATIONWIDGET_H
