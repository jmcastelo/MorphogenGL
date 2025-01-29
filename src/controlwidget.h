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

#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H



#include "generator.h"
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
#include <QTime>
#include <QMessageBox>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSplitter>
#include <QScrollArea>
#include <QMediaFormat>



class ConfigurationParser;



// ControlWidget: contains MorphogenGL's controls

class ControlWidget : public QWidget
{
    Q_OBJECT

public:
    GeneratorGL* generator;

    ControlWidget(double itFPS, double updFPS, GeneratorGL* theGenerator, PlotsWidget* thePlotsWidget, QWidget *parent = nullptr);
    ~ControlWidget();

signals:
    void iterateStateChanged(bool state);
    void seedDrawn();
    void updateStateChanged(bool state);
    void showMidiWidget();
    void imageSizeChanged(int width, int height);
    void startRecording(QString recordFilename, int framesPerSecond, QMediaFormat format);
    void stopRecording();
    void takeScreenshot(QString filename);
    void iterationFPSChanged(double newFPS);
    void updateFPSChanged(double newFPS);


public slots:
    void reset();
    void updateWindowSizeLineEdits(int width, int height);
    void populateTexFormatComboBox(QList<TextureFormat> formats);
    void updateIterationNumberLabel();
    void updateIterationMetricsLabels(double uspf, double fps);
    void updateUpdateMetricsLabels(double uspf, double fps);
    void setVideoCaptureElapsedTimeLabel(int frameNumber);
    void setupMidi(QString portName, bool open);
    void showMidiButtons(bool show);
    void updateMidiLinks(QString portName, int key, int value);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
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
    QList<QPair<QUuid, QString>> sortedOperationsData;

    QAction* iterateAction;
    QAction* recordAction;
    QAction* screenshotAction;
    QAction* displayOptionsAction;
    QAction* recordingOptionsAction;
    QAction* sortedOperationsAction;
    QAction* saveConfigAction;
    QAction* loadConfigAction;

    qreal framesPerSecond = 60.0;
    QString outputDir = QDir::currentPath();
    QMediaFormat format;
    QList<QMediaFormat::VideoCodec> supportedVideoCodecs;
    QList<QMediaFormat::FileFormat> supportedFileFormats;
    QComboBox* fileFormatsComboBox;
    QComboBox* videoCodecsComboBox;

    QStatusBar* statusBar;

    QLabel* iterationNumberLabel;
    QLabel* timePerIterationLabel;
    QLabel* iterationFPSLabel;
    QLabel* timePerUpdateLabel;
    QLabel* updateFPSLabel;

    QLineEdit* windowWidthLineEdit;
    QLineEdit* windowHeightLineEdit;

    QComboBox* texFormatComboBox;

    QLabel* videoCaptureElapsedTimeLabel;

    QMap<QUuid, OperationsWidget*> operationsWidgets;
    QMap<QString, QMap<int, Number<float>*>> midiFloatLinks;
    QMap<QString, QMap<int, Number<int>*>> midiIntLinks;
    Number<float>* linkingFloat = nullptr;
    Number<int>* linkingInt = nullptr;
    bool anyMidiPortOpen = false;

    QList<BlendFactorWidget*> blendFactorWidgets;

    QScrollArea* scrollArea;
    QWidget* scrollWidget;
    QHBoxLayout* scrollLayout;

    QSplitter* splitter;

    void constructSystemToolBar();
    void constructDisplayOptionsWidget(double itsFPS, double updFPS);
    void constructRecordingOptionsWidget();
    void constructSortedOperationsWidget();

    QString textureFormatToString(TextureFormat format);

    void populateFileFormatsComboBox();
    void populateVideoCodecsComboBox();

    void updateScrollLayout(QWidget* widget);
    void updateScrollArea();

private slots:
    void iterate();
    void record();
    void setScreenshotFilename();
    void setOutputDir();
    void toggleDisplayOptionsWidget();
    void toggleRecordingOptionsWidget();
    void toggleSortedOperationsWidget();
    void plotsActionTriggered();
    void loadConfig();
    void saveConfig();
    void about();

    void populateSortedOperationsTable(QList<QPair<QUuid, QString>> sortedData, QList<QUuid> unsortedData);
    void populateScrollLayout(QList<QPair<QUuid, QString>> data, QList<QUuid> unsortedData);
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



#endif // CONTROLWIDGET_H
