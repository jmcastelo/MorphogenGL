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

#include "heart.h"
#include "configparser.h"
#include "node.h"
#include "controlwidget.h"

ControlWidget::ControlWidget(Heart* theHeart, QWidget *parent) : QWidget(parent), heart { theHeart }
{
    // Generator

    generator = new GeneratorGL();

    // Contruct nodes toolbar

    nodesToolBar = new QToolBar;
    nodesToolBar->setOrientation(Qt::Vertical);
    nodesToolBar->hide();

    constructSystemToolBar();
    constructDisplayOptionsWidget();
    constructRecordingOptionsWidget();
    constructSortedOperationsWidget();

    // Graph widget

    graphWidget = new GraphWidget(generator);

    connect(graphWidget, &GraphWidget::singleNodeSelected, this, &ControlWidget::constructSingleNodeToolBar);
    connect(graphWidget, &GraphWidget::multipleNodesSelected, this, &ControlWidget::constructMultipleNodesToolBar);
    connect(graphWidget, &GraphWidget::showOperationParameters, this, &ControlWidget::showParametersWidget);
    connect(graphWidget, &GraphWidget::removeOperationParameters, this, &ControlWidget::removeParametersWidget);
    connect(graphWidget, &GraphWidget::updateOperationParameters, this, &ControlWidget::updateParametersWidget);

    // Parser

    parser = new ConfigurationParser(generator, graphWidget, heart);

    // Status bar

    statusBar = new QStatusBar;
    statusBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    statusBar->setSizeGripEnabled(false);

    iterationNumberLabel = new QLabel("Frame: 0");
    timePerIterationLabel = new QLabel("mSPF: 0");
    fpsLabel = new QLabel("FPS: 0");

    statusBar->insertWidget(0, iterationNumberLabel, 1);
    statusBar->insertWidget(1, timePerIterationLabel, 1);
    statusBar->insertWidget(2, fpsLabel, 1);

    // Style

    QString pipelineButtonStyle = "QPushButton#pipelineButton{ color: #ffffff; background-color: #6600a6; } QPushButton:checked#pipelineButton { color: #000000; background-color: #fcff59; }";
    QString statusBarStyle = "QStatusBar::item{ border: 0px solid black; }";
    setStyleSheet(pipelineButtonStyle + statusBarStyle);

    // Main layout

    QHBoxLayout* mainHBoxLayout = new QHBoxLayout;
    mainHBoxLayout->addWidget(graphWidget);
    mainHBoxLayout->addWidget(nodesToolBar);

    QVBoxLayout* mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addWidget(systemToolBar);
    mainVBoxLayout->addLayout(mainHBoxLayout);
    mainVBoxLayout->addWidget(statusBar);

    setLayout(mainVBoxLayout);

    // Window icon

    setWindowIcon(QIcon(":/icons/morphogengl.png"));

    // Signals + Slots

    connect(heart, &Heart::iterationPerformed, this, &ControlWidget::updateIterationNumberLabel);
    connect(heart, &Heart::iterationTimeMeasured, this, &ControlWidget::updateMetricsLabels);
    connect(parser, &ConfigurationParser::updateImageSize, [&](int width, int height)
    {
        generator->resize(width, height);
        imageWidthLineEdit->setText(QString::number(width));
        imageHeightLineEdit->setText(QString::number(height));
    });
}

ControlWidget::~ControlWidget()
{
    delete displayOptionsWidget;
    delete recordingOptionsWidget;
    delete sortedOperationsWidget;
    delete parser;
    delete graphWidget;
    delete generator;
    qDeleteAll(operationsWidgets);
}

void ControlWidget::closeEvent(QCloseEvent* event)
{
    displayOptionsWidget->close();
    recordingOptionsWidget->close();

    disconnect(generator, &GeneratorGL::sortedOperationsChanged, this, &ControlWidget::populateSortedOperationsTable);
    sortedOperationsWidget->close();

    foreach(OperationsWidget* opWidget, operationsWidgets)
        opWidget->close();

    graphWidget->closeWidgets();

    emit closing();
    event->accept();
}

void ControlWidget::constructSystemToolBar()
{
    systemToolBar = new QToolBar;
    iterateAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/media-playback-start.png")), "Start/pause feedback loop");
    iterateAction->setCheckable(true);
    QAction* resetAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/view-refresh.png")), "Reset");
    recordAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/media-record.png")), "Record video");
    recordAction->setCheckable(true);
    screenshotAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/digikam.png")), "Take screenshot");
    systemToolBar->addSeparator();
    displayOptionsAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/video-display.png")), "Display options");
    recordingOptionsAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/emblem-videos.png")), "Recording options");
    systemToolBar->addSeparator();
    systemToolBar->addAction(QIcon(QPixmap(":/icons/format-list-ordered.png")), "List sorted operations", this, &ControlWidget::toggleSortedOperationsWidget);
    systemToolBar->addSeparator();
    systemToolBar->addAction(QIcon(QPixmap(":/icons/office-chart-area-stacked.png")), "Show plots", this, &ControlWidget::plotsActionTriggered);
    systemToolBar->addSeparator();
    loadConfigAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/document-open.png")), "Load configuration");
    saveConfigAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/document-save.png")), "Save configuration");
    systemToolBar->addSeparator();
    QAction* aboutAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/help-about.png")), "About");

    connect(iterateAction, &QAction::triggered, this, &ControlWidget::iterate);
    connect(resetAction, &QAction::triggered, this, &ControlWidget::reset);
    connect(recordAction, &QAction::triggered, this, &ControlWidget::record);
    connect(screenshotAction, &QAction::triggered, this, &ControlWidget::takeScreenshot);
    connect(displayOptionsAction, &QAction::triggered, this, &ControlWidget::toggleDisplayOptionsWidget);
    connect(recordingOptionsAction, &QAction::triggered, this, &ControlWidget::toggleRecordingOptionsWidget);
    connect(loadConfigAction, &QAction::triggered, this, &ControlWidget::loadConfig);
    connect(saveConfigAction, &QAction::triggered, this, &ControlWidget::saveConfig);
    connect(aboutAction, &QAction::triggered, this, &ControlWidget::about);
}

void ControlWidget::iterate()
{
    if (iterateAction->isChecked())
    {
        iterateAction->setIcon(QIcon(QPixmap(":/icons/media-playback-pause.png")));
        heart->setStartTime();
        generator->active = true;
    }
    else
    {
        iterateAction->setIcon(QIcon(QPixmap(":/icons/media-playback-start.png")));
        generator->active = false;
    }
}

void ControlWidget::reset()
{
    generator->clearAllOperations();
    generator->drawAllSeeds();
    generator->resetIterationNumer();
}

void ControlWidget::record()
{
    if (recordAction->isChecked())
    {
        if (recordFilename.isEmpty())
        {
            QMessageBox messageBox;
            messageBox.setText("Please select output file.");
            messageBox.exec();

            recordAction->setChecked(false);
        }
        else
        {
            recordAction->setIcon(QIcon(QPixmap(":/icons/media-playback-stop.png")));
            videoCaptureElapsedTimeLabel->setText("00:00:00.000");
            heart->startRecording(recordFilename, framesPerSecond, preset, crf);
        }
    }
    else
    {
        recordAction->setIcon(QIcon(QPixmap(":/icons/media-record.png")));
        heart->stopRecording();
        recordFilename.clear();
    }
}

void ControlWidget::takeScreenshot()
{   
    QImage screenshot = heart->grabMorphoWidgetFramebuffer();
    
    QString filename = QFileDialog::getSaveFileName(this, "Save image", "", "Images (*.bmp *.ico *.jpeg *.jpg *.png *.tif *.tiff)");
    if (!filename.isEmpty())
    {
        screenshot.save(filename);
    }
}

void ControlWidget::toggleDisplayOptionsWidget()
{
    displayOptionsWidget->setVisible(!displayOptionsWidget->isVisible());
}

void ControlWidget::toggleRecordingOptionsWidget()
{
    recordingOptionsWidget->setVisible(!recordingOptionsWidget->isVisible());
}

void ControlWidget::toggleSortedOperationsWidget()
{
    sortedOperationsWidget->setVisible(!sortedOperationsWidget->isVisible());
}

void ControlWidget::loadConfig()
{
    QString filename = QFileDialog::getOpenFileName(this, "Load configuration", "", "MorphogenGL configurations (*.morph)");

    if (!filename.isEmpty())
    {
        parser->setFilename(filename);
        parser->read();

        // ToDo: connect with graph widget

        generator->resetIterationNumer();

        updateIterationNumberLabel();
        timePerIterationLabel->setText(QString("mSPF: 0"));
        fpsLabel->setText(QString("FPS: 0"));
    }
}

void ControlWidget::saveConfig()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save configuration", "", "MorphogenGL configurations (*.morph)");

    if (!filename.isEmpty())
    {
        parser->setFilename(filename);
        parser->write();
    }
}

void ControlWidget::about()
{
    QMessageBox* aboutBox = new QMessageBox(this);

    aboutBox->setTextFormat(Qt::RichText);

    aboutBox->setWindowTitle("About");

    QStringList lines;
    lines.append(QString("<h2>MorphogenGL %1</h2>").arg(generator->version));
    lines.append("<h4>Videofeedback simulation software.</h4>");
    lines.append("<h5>Let the pixels come alive!</h5><br>");
    lines.append("Looking for help? Please visit:<br>");
    lines.append("<a href='https://github.com/jmcastelo/MorphogenGL'>https://github.com/jmcastelo/MorphogenGL</a>");

    QString text = lines.join("");

    aboutBox->setText(text);

    aboutBox->setInformativeText("Copyright 2021 Jose Maria Castelo Ares\njose.maria.castelo@gmail.com\nLicense: GPLv3");

    aboutBox->exec();
}

void ControlWidget::updateIterationNumberLabel()
{
    iterationNumberLabel->setText(QString("Frame: %1").arg(generator->getIterationNumber()));
}

void ControlWidget::updateMetricsLabels(long int iterationTime)
{
    timePerIterationLabel->setText(QString("mSPF: %1").arg(iterationTime / 10.0));
    fpsLabel->setText(QString("FPS: %1").arg(10000.0 / iterationTime));
}

// Public slots

void ControlWidget::updateWindowSizeLineEdits(int width, int height)
{
    windowWidthLineEdit->setText(QString::number(width));
    windowHeightLineEdit->setText(QString::number(height));
}

// Nodes toolbar: single node selected

void ControlWidget::constructSingleNodeToolBar(Node* node)
{
    nodesToolBar->clear();

    if (node)
    {
        nodesToolBar->show();

        if (OperationNode* opNode = qgraphicsitem_cast<OperationNode*>(node))
        {
            selectedOperationNode = opNode;

            QMenu* operationsMenu = new QMenu("Set operation");

            for (QString opName : generator->availableOperations)
                operationsMenu->addAction(opName);

            connect(operationsMenu, &QMenu::triggered, this, &ControlWidget::setNodeOperation);

            QAction* setOperationAction = nodesToolBar->addAction(QIcon(QPixmap(":/icons/bookmark.png")), "Set operation");
            setOperationAction->setMenu(operationsMenu);

            if (generator->hasOperationParamaters(opNode->id))
                nodesToolBar->addAction(QIcon(QPixmap(":/icons/applications-system.png")), "Set parameters", opNode, &OperationNode::setParameters);

            QAction* enableAction = nodesToolBar->addAction(generator->isOperationEnabled(opNode->id) ? QIcon(QPixmap(":/icons/circle-green.png")) : QIcon(QPixmap(":/icons/circle-grey.png")), generator->isOperationEnabled(opNode->id) ? "Enabled" : "Disabled", this, &ControlWidget::enableNodeOperation);
            enableAction->setCheckable(true);
            enableAction->setChecked(generator->isOperationEnabled(opNode->id));

            if (opNode->hasInputs())
                nodesToolBar->addAction(QIcon(QPixmap(":/icons/preferences-desktop.png")), "Equalize blend factors", opNode, &OperationNode::equalizeBlendFactors);

            nodesToolBar->addSeparator();

            nodesToolBar->addAction(QIcon(QPixmap(":/icons/eye.png")), "Set as output", opNode, &OperationNode::setAsOutput);

            if (graphWidget->moreThanOneNode())
                nodesToolBar->addAction(QIcon(QPixmap(":/icons/network-connect.png")), "Connect to", opNode, &OperationNode::selectToConnect);

            nodesToolBar->addSeparator();

            nodesToolBar->addAction(QIcon(QPixmap(":/icons/edit-clear.png")), "Clear", opNode, &OperationNode::clear);

            nodesToolBar->addSeparator();

            nodesToolBar->addAction(QIcon(QPixmap(":/icons/edit-copy.png")), "Copy", opNode, &OperationNode::copy);
            nodesToolBar->addAction(QIcon(QPixmap(":/icons/edit-delete.png")), "Remove", this, &ControlWidget::removeNodeOperation);
        }
        else if (SeedNode* seedNode = qgraphicsitem_cast<SeedNode*>(node))
        {
            selectedSeedNode = seedNode;

            nodesToolBar->addAction(QIcon(QPixmap(":/icons/applications-graphics.png")), "Draw", seedNode, &SeedNode::draw);

            nodesToolBar->addSeparator();

            QAction* colorAction = nodesToolBar->addAction(QIcon(QPixmap(":/icons/color-chooser.png")), "Random: color", this, &ControlWidget::setSeedNodeType);
            colorAction->setCheckable(true);
            colorAction->setData(QVariant(0));

            QAction* grayscaleAction = nodesToolBar->addAction(QIcon(QPixmap(":/icons/gray-chooser.png")), "Random: grayscale", this, &ControlWidget::setSeedNodeType);
            grayscaleAction->setCheckable(true);
            grayscaleAction->setData(QVariant(1));

            QAction* imageAction = nodesToolBar->addAction(QIcon(QPixmap(":/icons/image-x-generic.png")), "Image", this, &ControlWidget::setSeedNodeType);
            imageAction->setCheckable(true);
            imageAction->setData(QVariant(2));

            QActionGroup* type = new QActionGroup(this);
            type->addAction(colorAction);
            type->addAction(grayscaleAction);
            type->addAction(imageAction);

            colorAction->setChecked(generator->getSeedType(seedNode->id) == 0);
            grayscaleAction->setChecked(generator->getSeedType(seedNode->id) == 1);
            imageAction->setChecked(generator->getSeedType(seedNode->id) == 2);

            nodesToolBar->addSeparator();

            nodesToolBar->addAction(QIcon(QPixmap(":/icons/folder-image.png")), "Load image", seedNode, &SeedNode::loadImage);

            QAction* fixedAction = nodesToolBar->addAction(generator->isSeedFixed(seedNode->id) ? QIcon(QPixmap(":/icons/document-encrypt.png")) : QIcon(QPixmap(":/icons/document-decrypt.png")), generator->isSeedFixed(seedNode->id) ? "Fixed" : "Not fixed", this, &ControlWidget::setSeedNodeFixed);
            fixedAction->setCheckable(true);
            fixedAction->setChecked(generator->isSeedFixed(seedNode->id));

            nodesToolBar->addSeparator();

            nodesToolBar->addAction(QIcon(QPixmap(":/icons/eye.png")), "Set as output", seedNode, &SeedNode::setAsOutput);

            if (graphWidget->moreThanOneNode())
                nodesToolBar->addAction(QIcon(QPixmap(":/icons/network-connect.png")), "Connect to", seedNode, &SeedNode::selectToConnect);

            nodesToolBar->addSeparator();

            nodesToolBar->addAction(QIcon(QPixmap(":/icons/edit-copy.png")), "Copy", seedNode, &SeedNode::copy);
            nodesToolBar->addAction(QIcon(QPixmap(":/icons/edit-delete.png")), "Remove", this, &ControlWidget::removeSeedNode);
        }
    }
    else
    {
        nodesToolBar->hide();
    }
}

void ControlWidget::setNodeOperation(QAction* action)
{
    selectedOperationNode->setOperation(action);
    constructSingleNodeToolBar(selectedOperationNode);
}

void ControlWidget::enableNodeOperation(bool checked)
{
    selectedOperationNode->enableOperation(checked);
    constructSingleNodeToolBar(selectedOperationNode);
}

void ControlWidget::removeNodeOperation()
{
    selectedOperationNode->remove();
    selectedOperationNode = nullptr;
    constructSingleNodeToolBar(nullptr);
}

void ControlWidget::setSeedNodeType()
{
    QAction* action = qobject_cast<QAction*>(sender());
    generator->setSeedType(selectedSeedNode->id, action->data().toInt());
}

void ControlWidget::setSeedNodeFixed(bool checked)
{
    selectedSeedNode->setFixed(checked);
    constructSingleNodeToolBar(selectedSeedNode);
}

void ControlWidget::removeSeedNode()
{
    selectedSeedNode->remove();
    selectedSeedNode = nullptr;
    constructSingleNodeToolBar(nullptr);
}

// Nodes toolbar: multiple nodes selected

void ControlWidget::constructMultipleNodesToolBar()
{
    nodesToolBar->clear();
    nodesToolBar->show();

    if (graphWidget->seedNodesSelected() > 0)
    {
        nodesToolBar->addAction(QIcon(QPixmap(":/icons/applications-graphics.png")), graphWidget->seedNodesSelected() > 1 ? "Draw seeds" : "Draw seed", graphWidget, &GraphWidget::drawSelectedSeeds);
        nodesToolBar->addSeparator();
    }

    if (graphWidget->operationNodesSelected())
    {
        nodesToolBar->addAction(QIcon(QPixmap(":/icons/applications-system.png")), "Set parameters", graphWidget, &GraphWidget::setSelectedOperationsParameters);
        nodesToolBar->addAction(QIcon(QPixmap(":/icons/circle-green.png")), "Enable", graphWidget, &GraphWidget::enableSelectedOperations);
        nodesToolBar->addAction(QIcon(QPixmap(":/icons/circle-grey.png")), "Disable", graphWidget, &GraphWidget::disableSelectedOperations);
        nodesToolBar->addAction(QIcon(QPixmap(":/icons/preferences-desktop.png")), "Equalize blend factors", graphWidget, &GraphWidget::equalizeSelectedBlendFactors);
        nodesToolBar->addSeparator();
        nodesToolBar->addAction(QIcon(QPixmap(":/icons/edit-clear.png")), "Clear", graphWidget, &GraphWidget::clearSelectedOperationNodes);
        nodesToolBar->addSeparator();
    }

    if (graphWidget->nodesSelected())
    {
        nodesToolBar->addAction(QIcon(QPixmap(":/icons/edit-copy.png")), "Copy", graphWidget, &GraphWidget::makeNodeSnapshot);
        nodesToolBar->addAction(QIcon(QPixmap(":/icons/edit-delete.png")), "Remove", graphWidget, &GraphWidget::removeSelectedNodes);
    }
}

// Display

void ControlWidget::constructDisplayOptionsWidget()
{
    FocusLineEdit* fpsLineEdit = new FocusLineEdit;
    fpsLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* fpsIntValidator = new QIntValidator(1, 1000, fpsLineEdit);
    fpsIntValidator->setLocale(QLocale::English);
    fpsLineEdit->setValidator(fpsIntValidator);
    fpsLineEdit->setText(QString::number(static_cast<int>(1000.0 / heart->getTimerInterval())));

    imageWidthLineEdit = new FocusLineEdit;
    imageWidthLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* imageWidthIntValidator = new QIntValidator(0, 8192, imageWidthLineEdit);
    imageWidthIntValidator->setLocale(QLocale::English);
    imageWidthLineEdit->setValidator(imageWidthIntValidator);
    imageWidthLineEdit->setText(QString::number(generator->getWidth()));

    imageHeightLineEdit = new FocusLineEdit;
    imageHeightLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* imageHeightIntValidator = new QIntValidator(0, 8192, imageHeightLineEdit);
    imageHeightIntValidator->setLocale(QLocale::English);
    imageHeightLineEdit->setValidator(imageHeightIntValidator);
    imageHeightLineEdit->setText(QString::number(generator->getHeight()));

    windowWidthLineEdit = new FocusLineEdit;
    windowWidthLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* windowWidthIntValidator = new QIntValidator(0, 8192, windowWidthLineEdit);
    windowWidthIntValidator->setLocale(QLocale::English);
    windowWidthLineEdit->setValidator(windowWidthIntValidator);
    windowWidthLineEdit->setText(QString::number(heart->getMorphoWidgetWidth()));

    windowHeightLineEdit = new FocusLineEdit;
    windowHeightLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* windowHeightIntValidator = new QIntValidator(0, 8192, windowHeightLineEdit);
    windowHeightIntValidator->setLocale(QLocale::English);
    windowHeightLineEdit->setValidator(windowHeightIntValidator);
    windowHeightLineEdit->setText(QString::number(heart->getMorphoWidgetHeight()));

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("FPS:", fpsLineEdit);
    formLayout->addRow("Image width (px):", imageWidthLineEdit);
    formLayout->addRow("Image height (px):", imageHeightLineEdit);
    formLayout->addRow("Window width (px):", windowWidthLineEdit);
    formLayout->addRow("Window height (px):", windowHeightLineEdit);

    displayOptionsWidget = new QWidget;
    displayOptionsWidget->setLayout(formLayout);
    displayOptionsWidget->setWindowTitle("Display options");

    // Signals + Slots

    connect(fpsLineEdit, &FocusLineEdit::returnPressed, [&heart = this->heart, fpsLineEdit]()
    {
        heart->setTimerInterval(static_cast<int>(1000.0 / fpsLineEdit->text().toInt()));
    });
    connect(fpsLineEdit, &FocusLineEdit::focusOut, [&heart = this->heart, fpsLineEdit]()
    {
        fpsLineEdit->setText(QString::number(static_cast<int>(1000.0 / heart->getTimerInterval())));
    });

    connect(imageWidthLineEdit, &FocusLineEdit::returnPressed, [=]()
    {
        generator->resize(imageWidthLineEdit->text().toInt(), generator->getHeight());
    });
    connect(imageWidthLineEdit, &FocusLineEdit::focusOut, [&generator = this->generator, &imageWidthLineEdit = this->imageWidthLineEdit]()
    {
        imageWidthLineEdit->setText(QString::number(generator->getWidth()));
    });
    connect(imageHeightLineEdit, &FocusLineEdit::returnPressed, [=]()
    {
        generator->resize(generator->getWidth(), imageHeightLineEdit->text().toInt());
    });
    connect(imageHeightLineEdit, &FocusLineEdit::focusOut, [&generator = this->generator, &imageHeightLineEdit = this->imageHeightLineEdit]()
    {
        imageHeightLineEdit->setText(QString::number(generator->getHeight()));
    });

    connect(windowWidthLineEdit, &FocusLineEdit::returnPressed, [&heart = this->heart, &windowWidthLineEdit = this->windowWidthLineEdit, &windowHeightLineEdit = this->windowHeightLineEdit]()
    {
        heart->resizeMorphoWidget(windowWidthLineEdit->text().toInt(), windowHeightLineEdit->text().toInt());
    });
    connect(windowWidthLineEdit, &FocusLineEdit::focusOut, [&heart = this->heart, &windowWidthLineEdit = this->windowWidthLineEdit]()
    {
        windowWidthLineEdit->setText(QString::number(heart->getMorphoWidgetWidth()));
    });
    connect(windowHeightLineEdit, &FocusLineEdit::returnPressed, [&heart = this->heart, &windowWidthLineEdit = this->windowWidthLineEdit, &windowHeightLineEdit = this->windowHeightLineEdit]()
    {
        heart->resizeMorphoWidget(windowWidthLineEdit->text().toInt(), windowHeightLineEdit->text().toInt());
    });
    connect(windowHeightLineEdit, &FocusLineEdit::focusOut, [&heart = this->heart, &windowHeightLineEdit = this->windowHeightLineEdit]()
    {
        windowHeightLineEdit->setText(QString::number(heart->getMorphoWidgetHeight()));
    });
}

void ControlWidget::constructRecordingOptionsWidget()
{
    QPushButton* videoFilenamePushButton = new QPushButton(QIcon(QPixmap(":/icons/document-open.png")), "Select output file");
    videoFilenamePushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QComboBox* presetsVideoComboBox = new QComboBox;
    presetsVideoComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    presetsVideoComboBox->addItem("ultrafast");
    presetsVideoComboBox->addItem("superfast");
    presetsVideoComboBox->addItem("veryfast");
    presetsVideoComboBox->addItem("faster");
    presetsVideoComboBox->addItem("fast");
    presetsVideoComboBox->addItem("medium");
    presetsVideoComboBox->addItem("slow");
    presetsVideoComboBox->addItem("slower");
    presetsVideoComboBox->addItem("veryslow");
    presetsVideoComboBox->setCurrentIndex(0);

    FocusLineEdit* crfVideoLineEdit = new FocusLineEdit;
    crfVideoLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* crfVideoValidator = new QIntValidator(0, 51, crfVideoLineEdit);
    crfVideoValidator->setLocale(QLocale::English);
    crfVideoLineEdit->setValidator(crfVideoValidator);
    crfVideoLineEdit->setText(QString::number(crf));

    FocusLineEdit* fpsVideoLineEdit = new FocusLineEdit;
    fpsVideoLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* fpsVideoValidator = new QIntValidator(1, 1000, fpsVideoLineEdit);
    fpsVideoValidator->setLocale(QLocale::English);
    fpsVideoLineEdit->setValidator(fpsVideoValidator);
    fpsVideoLineEdit->setText(QString::number(framesPerSecond));

    videoCaptureElapsedTimeLabel = new QLabel("00:00:00.000");
    videoCaptureElapsedTimeLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    videoCaptureFilenameLabel = new QLabel;
    videoCaptureFilenameLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    QFormLayout* videoFormLayout = new QFormLayout;
    videoFormLayout->addRow("Filename:", videoCaptureFilenameLabel);
    videoFormLayout->addRow("Speed:", presetsVideoComboBox);
    videoFormLayout->addRow("Quality:", crfVideoLineEdit);
    videoFormLayout->addRow("FPS:", fpsVideoLineEdit);
    videoFormLayout->addRow("Elapsed time:", videoCaptureElapsedTimeLabel);

    QVBoxLayout* videoVBoxLayout = new QVBoxLayout;
    videoVBoxLayout->addWidget(videoFilenamePushButton);
    videoVBoxLayout->addLayout(videoFormLayout);

    recordingOptionsWidget = new QWidget;
    recordingOptionsWidget->setLayout(videoVBoxLayout);
    recordingOptionsWidget->setWindowTitle("Recording options");

    // Signals + Slot

    connect(videoFilenamePushButton, &QPushButton::clicked, this, &ControlWidget::setVideoFilename);
    connect(presetsVideoComboBox, &QComboBox::currentTextChanged, [&preset = this->preset](QString thePreset)
    {
        preset = thePreset;
    });
    connect(crfVideoLineEdit, &FocusLineEdit::returnPressed, [&crf = this->crf, crfVideoLineEdit]()
    {
        crf = crfVideoLineEdit->text().toInt();
    });
    connect(crfVideoLineEdit, &FocusLineEdit::focusOut, [&crf = this->crf, crfVideoLineEdit]()
    {
        crfVideoLineEdit->setText(QString::number(crf));
    });
    connect(fpsVideoLineEdit, &FocusLineEdit::returnPressed, [&framesPerSecond = this->framesPerSecond, fpsVideoLineEdit]()
    {
        framesPerSecond = fpsVideoLineEdit->text().toInt();
    });
    connect(fpsVideoLineEdit, &FocusLineEdit::focusOut, [&framesPerSecond = this->framesPerSecond, fpsVideoLineEdit]()
    {
        fpsVideoLineEdit->setText(QString::number(framesPerSecond));
    });
    connect(heart, &Heart::frameRecorded, this, &ControlWidget::setVideoCaptureElapsedTimeLabel);
}

void ControlWidget::setVideoFilename()
{
    QString videoPath = QFileDialog::getSaveFileName(this, "Output video file", "", "Videos (*.mp4 *.mkv *.mov)");

    if (!videoPath.isEmpty())
    {
        recordFilename = videoPath;
        videoCaptureFilenameLabel->setText(videoPath.section('/', -1));
    }
}

void ControlWidget::setVideoCaptureElapsedTimeLabel()
{
    int milliseconds = static_cast<int>(1000.0 * heart->getFrameCount() / framesPerSecond);

    QTime start(0, 0, 0, 0);

    videoCaptureElapsedTimeLabel->setText(start.addMSecs(milliseconds).toString("hh:mm:ss.zzz"));
}

void ControlWidget::constructSortedOperationsWidget()
{
     sortedOperationsTable = new QTableWidget;
     sortedOperationsTable->setColumnCount(1);
     sortedOperationsTable->setHorizontalHeaderLabels(QStringList("Operation"));
     sortedOperationsTable->horizontalHeader()->setStretchLastSection(true);
     sortedOperationsTable->resizeColumnsToContents();
     sortedOperationsTable->setSelectionMode(QAbstractItemView::MultiSelection);

     QVBoxLayout *layout = new QVBoxLayout;
     layout->setAlignment(Qt::AlignCenter);
     layout->addWidget(sortedOperationsTable);

     sortedOperationsWidget = new QWidget;
     sortedOperationsWidget->setLayout(layout);
     sortedOperationsWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
     sortedOperationsWidget->setWindowTitle("Sorted operations");
     sortedOperationsWidget->setWindowFlags(Qt::WindowStaysOnTopHint);

     connect(generator, &GeneratorGL::sortedOperationsChanged, this, &ControlWidget::populateSortedOperationsTable);
     connect(sortedOperationsTable, &QTableWidget::itemSelectionChanged, this, &ControlWidget::selectNodesToMark);
}

void ControlWidget::populateSortedOperationsTable(QVector<QPair<QUuid, QString>> data)
{
    sortedOperationsTable->clearContents();
    sortedOperationsTable->setRowCount(data.size());

    for (int row = 0; row < data.size(); row++)
    {
        QTableWidgetItem* item = new QTableWidgetItem(data[row].second);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        sortedOperationsTable->setItem(row, 0, item);
    }

    sortedOperationsData = data;
}

void ControlWidget::selectNodesToMark()
{
    QVector<QUuid> nodeIds;

    for (QTableWidgetItem* item : sortedOperationsTable->selectedItems())
        nodeIds.push_back(sortedOperationsData[item->row()].first);

    graphWidget->markNodes(nodeIds);
}

void ControlWidget::showParametersWidget(QUuid id)
{
    if (generator->hasOperationParamaters(id))
    {
        if (operationsWidgets.contains(id))
        {
            // If it exists, recreate it, perhaps with new operation

            operationsWidgets.value(id)->recreate(generator->getOperation(id));
        }
        else
        {
            // Create it

            operationsWidgets.insert(id, new OperationsWidget(generator->getOperation(id)));
        }

        if (!operationsWidgets.value(id)->isVisible())
            operationsWidgets.value(id)->show();
    }
}

void ControlWidget::removeParametersWidget(QUuid id)
{
    if (operationsWidgets.contains(id))
    {
        operationsWidgets.value(id)->deleteLater();
        operationsWidgets.remove(id);
    }
}

void ControlWidget::updateParametersWidget(QUuid id)
{
    if (operationsWidgets.contains(id))
    {
        if (generator->getOperation(id)->hasParameters())
            operationsWidgets.value(id)->recreate(generator->getOperation(id));
        else
            removeParametersWidget(id);
    }
}
