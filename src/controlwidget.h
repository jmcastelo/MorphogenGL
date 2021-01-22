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

#include "generator.h"
#include "focuslineedit.h"
#include "operationswidget.h"
#include "graphwidget.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QPixmap>
#include <QImage>
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

class Heart;
class ConfigurationParser;

// ControlWidget: contains MorphogenGL's controls

class ControlWidget : public QWidget
{
    Q_OBJECT

public:
    GeneratorGL* generator;

    ControlWidget(Heart* theHeart, QWidget *parent = nullptr);
    ~ControlWidget();

signals:
    void imageSizeChanged();
    void seedDrawn();
    void closing();

public slots:
    void uncheckRGBGraphAction();
    void updateWindowSizeLineEdits(int width, int height);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Heart* heart;

    ConfigurationParser* parser;

    GraphWidget* graphWidget;

    QWidget* displayOptionsWidget;
    QWidget* recordingOptionsWidget;

    QAction* iterateAction;
    QAction* recordAction;
    QAction* screenshotAction;
    QAction* displayOptionsAction;
    QAction* recordingOptionsAction;
    QAction* rgbAction;
    QAction* saveConfigAction;
    QAction* loadConfigAction;

    int framesPerSecond = 50;
    QString recordFilename;
    QString preset = "ultrafast";
    int crf = 17;

    QStatusBar* statusBar;

    QLabel* iterationNumberLabel;
    QLabel* timePerIterationLabel;
    QLabel* fpsLabel;

    FocusLineEdit* imageWidthLineEdit;
    FocusLineEdit* imageHeightLineEdit;
    FocusLineEdit* windowWidthLineEdit;
    FocusLineEdit* windowHeightLineEdit;

    QLabel* videoCaptureElapsedTimeLabel;
    QLabel* videoCaptureFilenameLabel;

    QMap<QUuid, OperationsWidget*> operationsWidgets;

    void constructDisplayOptionsWidget();
    void constructRecordingOptionsWidget();

    void updateIterationNumberLabel();
    void updateMetricsLabels(long int time);

    void setVideoCaptureElapsedTimeLabel();
    void setVideoFilename();

private slots:
    void iterate();
    void record();
    void takeScreenshot();
    void toggleDisplayOptionsWidget();
    void toggleRecordingOptionsWidget();
    void toggleRGBGraph();
    void loadConfig();
    void saveConfig();
    void about();

    void showParametersWidget(QUuid id);
    void removeParametersWidget(QUuid id);
    void updateParametersWidget(QUuid id);
};
