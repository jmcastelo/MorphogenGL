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
#include "plotswidget.h"
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
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSplitter>
#include <QScrollArea>
#include <QSizeGrip>
#include <QMediaFormat>

class Heart;
class ConfigurationParser;

// ControlWidget: contains MorphogenGL's controls

class ControlWidget : public QWidget
{
    Q_OBJECT

public:
    GeneratorGL* generator;
    QSizeGrip* grip;

    ControlWidget(Heart* theHeart, QWidget *parent = nullptr);
    ~ControlWidget();

    void computePlots();
    void initPlotsWidget(QOpenGLContext* context){ plotsWidget->init(context); }

signals:
    void seedDrawn();
    void selectedPointChanged(QPoint point);
    //void closing();

public slots:
    void updateWindowSizeLineEdits(int width, int height);

protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    Heart* heart;

    PlotsWidget* plotsWidget;

    ConfigurationParser* parser;

    GraphWidget* graphWidget;

    OperationNode* selectedOperationNode = nullptr;
    SeedNode* selectedSeedNode = nullptr;

    QToolBar* systemToolBar;
    QToolBar* nodesToolBar;

    QWidget* displayOptionsWidget;
    QWidget* recordingOptionsWidget;
    QWidget* sortedOperationsWidget;

    QTableWidget* sortedOperationsTable;
    QVector<QPair<QUuid, QString>> sortedOperationsData;

    QAction* iterateAction;
    QAction* recordAction;
    QAction* screenshotAction;
    QAction* displayOptionsAction;
    QAction* recordingOptionsAction;
    QAction* sortedOperationsAction;
    QAction* saveConfigAction;
    QAction* loadConfigAction;

    qreal framesPerSecond = 50.0;
    QString outputDir = QDir::homePath();
    QMediaFormat format;
    QList<QMediaFormat::VideoCodec> supportedVideoCodecs;
    QList<QMediaFormat::FileFormat> supportedFileFormats;
    QComboBox* fileFormatsComboBox;
    QComboBox* videoCodecsComboBox;

    QStatusBar* statusBar;

    QLabel* iterationNumberLabel;
    QLabel* timePerIterationLabel;
    QLabel* fpsLabel;

    //QLineEdit* imageWidthLineEdit;
    //QLineEdit* imageHeightLineEdit;
    FocusLineEdit* windowWidthLineEdit;
    FocusLineEdit* windowHeightLineEdit;

    QLabel* videoCaptureElapsedTimeLabel;

    QMap<QUuid, OperationsWidget*> operationsWidgets;

    QScrollArea* scrollArea;
    QWidget* scrollWidget;
    QHBoxLayout* scrollLayout;

    QSplitter* splitter;

    void constructSystemToolBar();
    void constructDisplayOptionsWidget();
    void constructRecordingOptionsWidget();
    void constructSortedOperationsWidget();

    void updateIterationNumberLabel();
    void updateMetricsLabels(std::chrono::microseconds time, unsigned int its);

    void setVideoCaptureElapsedTimeLabel();
    void populateFileFormatsComboBox();
    void populateVideoCodecsComboBox();

    void updateScrollLayout(QWidget* widget);
    void updateScrollArea();

private slots:
    void iterate();
    void reset();
    void record();
    void takeScreenshot();
    void setOutputDir();
    void toggleDisplayOptionsWidget();
    void toggleRecordingOptionsWidget();
    void toggleSortedOperationsWidget();
    void plotsActionTriggered();
    void loadConfig();
    void saveConfig();
    void about();

    void populateSortedOperationsTable(QVector<QPair<QUuid, QString>> data);
    void selectNodesToMark();

    void createParametersWidget(QUuid id);
    void showParametersWidget(QUuid id);
    void removeParametersWidget(QUuid id);
    void updateParametersWidget(QUuid id);
    void updateParametersWidgetsBorder(QWidget* widget);
    void removeAllParametersWidgetsBorder();
    void removeOneParametersWidgetBorder(QWidget* widget);

    void constructSingleNodeToolBar(Node* node);
    void constructMultipleNodesToolBar();

    void setNodeOperation(QAction* action);
    void enableNodeOperation(bool checked);
    void removeNodeOperation();
    void setSeedNodeType();
    void setSeedNodeFixed(bool checked);
    void removeSeedNode();
};
