/*
*  Copyright 2020 José María Castelo Ares
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

#include "generator.h"
#include "focuslineedit.h"
#include "operationswidget.h"
#include "configparser.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QPixmap>
#include <QTabWidget>
#include <QStatusBar>
#include <QApplication>
#include <QPushButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QIntValidator>
#include <QFormLayout>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QListWidget>
#include <QSlider>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollArea>
#include <QTime>
#include <QMessageBox>
#include <QRect>
#include <QPoint>
#include <QSize>

class MorphoWidget;

// ControlWidget: contains MorphogenGL's controls

class ControlWidget : public QWidget
{
    Q_OBJECT

public:
    GeneratorGL* generator;

    ControlWidget(MorphoWidget* theMorphoWidget, QWidget *parent = nullptr);
    ~ControlWidget();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    MorphoWidget* morphoWidget;
    ConfigurationParser* parser;
    
    OperationsWidget* operationsWidget = nullptr;

    QAction* iterateAction;
    QAction* recordAction;
    QAction* screenshotAction;
    QAction* saveConfigAction;
    QAction* loadConfigAction;

    QWidget* generalControlsWidget;
    QWidget* pipelineControlsWidget;

    QTabWidget* mainTabWidget;

    QStatusBar* statusBar;

    QLabel* iterationNumberLabel;
    QLabel* timePerIterationLabel;
    QLabel* fpsLabel;

    FocusLineEdit* imageWidthLineEdit;
    FocusLineEdit* imageHeightLineEdit;
    FocusLineEdit* windowWidthLineEdit;
    FocusLineEdit* windowHeightLineEdit;

    QCheckBox* applyCircularMaskCheckBox;

    QLabel* videoCaptureElapsedTimeLabel;
    QLabel* videoCaptureFilenameLabel;

    QComboBox* newImageOperationComboBox;
    QPushButton* insertImageOperationPushButton;
    QPushButton* removeImageOperationPushButton;

    QPushButton* outputPipelinePushButton;
    QGridLayout* pipelinesGridLayout;
    QLabel* pipelinesLabel;
    QLabel* blendFactorsLabel;
    QButtonGroup* pipelinesButtonGroup;
    std::vector<QPushButton*> pipelinePushButton;
    std::vector<QLineEdit*> pipelineBlendFactorLineEdit;
    QPushButton* equalizeBlendFactorsPushButton;

    QListWidget* imageOperationsListWidget;
    int rowSize;
    std::map<int, int> currentImageOperationIndex;

    QSlider* selectedParameterSlider;
    FocusLineEdit* selectedParameterMinLineEdit;
    FocusLineEdit* selectedParameterMaxLineEdit;
    QDoubleValidator* selectedParameterMinValidator;
    QDoubleValidator* selectedParameterMaxValidator;
    QGroupBox* selectedParameterGroupBox;

    QVBoxLayout* parametersLayout;

    void iterate();
    void record();
    void takeScreenshot();
    void loadConfig();
    void saveConfig();
    void about();

    void constructGeneralControls();
    void constructPipelineControls();
    
    void resizeMainTabs(int index);

    void updateIterationNumberLabel();
    void updateMetricsLabels(long int time);

    void setVideoCaptureElapsedTimeLabel();
    void setVideoFilename();

    void initPipelineControls(int selectedPipelineIndex);
    void setPipelineBlendFactorLineEditText(int pipelineIndex);
    void initNewImageOperationComboBox();
    void initImageOperationsListWidget(int pipelineIndex);
    void onImageOperationsListWidgetCurrentRowChanged(int currentRow);
    void onFloatParameterWidgetFocusIn(FloatParameterWidget* widget);
    void onRowsMoved(QModelIndex parent, int start, int end, QModelIndex destination, int row);
    void insertImageOperation();
    void removeImageOperation();
};
