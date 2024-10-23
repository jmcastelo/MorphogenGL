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
#include "focuslineedit.h"
#include <QTimer>
#include <QActionGroup>
#include <QHeaderView>
#include <QScrollBar>

ControlWidget::ControlWidget(Heart* theHeart, QWidget *parent) : QWidget(parent), heart { theHeart }
{
    // Generator

    generator = new GeneratorGL();

    // Scroll area

    scrollWidget = new QWidget;
    scrollWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    scrollLayout = new QHBoxLayout;
    scrollLayout->setSizeConstraint(QLayout::SetFixedSize);
    scrollLayout->setAlignment(Qt::AlignLeft);
    scrollLayout->setDirection(QBoxLayout::RightToLeft);
    scrollWidget->setLayout(scrollLayout);

    scrollArea = new QScrollArea;
    scrollArea->setWidget(scrollWidget);

    // Contruct nodes toolbar

    nodesToolBar = new QToolBar;
    nodesToolBar->setOrientation(Qt::Vertical);
    nodesToolBar->hide();

    constructSystemToolBar();
    constructDisplayOptionsWidget();
    constructRecordingOptionsWidget();
    constructSortedOperationsWidget();

    updateScrollArea();

    // Plots widget

    plotsWidget = new PlotsWidget(FBO::width, FBO::height);
    plotsWidget->setVisible(false);

    // Graph widget

    graphWidget = new GraphWidget(generator);

    connect(graphWidget, &GraphWidget::singleNodeSelected, this, &ControlWidget::constructSingleNodeToolBar);
    connect(graphWidget, &GraphWidget::operationNodeSelected, this, &ControlWidget::showParametersWidget);
    connect(graphWidget, &GraphWidget::noOperationNodesSelected, this, &ControlWidget::removeAllParametersWidgetsBorder);
    connect(graphWidget, &GraphWidget::multipleNodesSelected, this, &ControlWidget::constructMultipleNodesToolBar);
    connect(graphWidget, &GraphWidget::showOperationParameters, this, &ControlWidget::createParametersWidget);
    connect(graphWidget, &GraphWidget::removeOperationParameters, this, &ControlWidget::removeParametersWidget);
    connect(graphWidget, &GraphWidget::updateOperationParameters, this, &ControlWidget::updateParametersWidget);
    connect(graphWidget, &GraphWidget::blendFactorWidgetCreated, this, [&](QWidget* widget)
    {
        widget->setParent(scrollWidget);
        updateScrollArea();
    });
    connect(graphWidget, &GraphWidget::blendFactorWidgetToggled, this, [&](QWidget* widget)
    {
        if (widget->isVisible())
        {
            if (scrollLayout->indexOf(widget) == -1)
            {
                scrollLayout->addWidget(widget);
                updateScrollArea();
            }
            scrollArea->ensureWidgetVisible(widget);
        }
        else
        {
            scrollLayout->removeWidget(widget);
            updateScrollArea();
        }
    });
    connect(graphWidget, &GraphWidget::operationEnabled, this, [&](QUuid id, bool enabled){
        if (operationsWidgets.contains(id))
            operationsWidgets.value(id)->toggleEnableButton(enabled);
    });

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

    // Grip

    grip = new QSizeGrip(this);
    grip->setVisible(true);

    // Style

    QString pipelineButtonStyle = "QPushButton#pipelineButton{ color: #ffffff; background-color: #6600a6; } QPushButton:checked#pipelineButton { color: #000000; background-color: #fcff59; }";
    QString statusBarStyle = "QStatusBar::item{ border: 0px solid black; }";
    setStyleSheet(pipelineButtonStyle + statusBarStyle);

    // Main layout

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addWidget(graphWidget);
    hLayout->addWidget(nodesToolBar);

    QWidget* widget = new QWidget;
    widget->setLayout(hLayout);

    QSplitter* splitter = new QSplitter;
    splitter->setOrientation(Qt::Vertical);
    splitter->setChildrenCollapsible(true);
    splitter->addWidget(widget);
    splitter->addWidget(plotsWidget);

    QVBoxLayout* mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addWidget(systemToolBar);
    mainVBoxLayout->addWidget(splitter);
    mainVBoxLayout->addWidget(scrollArea);
    mainVBoxLayout->addWidget(statusBar);
    mainVBoxLayout->addWidget(grip, 0, Qt::AlignLeft | Qt::AlignBottom);

    setLayout(mainVBoxLayout);

    // Window icon

    setWindowIcon(QIcon(":/icons/morphogengl.png"));

    // Needed over morphoWidget

    setAutoFillBackground(true);
    setWindowFlags(Qt::SubWindow);

    // Signals + Slots

    connect(heart, &Heart::iterationPerformed, this, &ControlWidget::updateIterationNumberLabel);
    connect(heart, &Heart::iterationTimeMeasured, this, &ControlWidget::updateMetricsLabels);
    connect(parser, &ConfigurationParser::updateImageSize, this, [&](int width, int height)
    {
        generator->resize(width, height);
        //imageWidthLineEdit->setText(QString::number(width));
        //imageHeightLineEdit->setText(QString::number(height));
    });
    connect(this, &ControlWidget::selectedPointChanged, plotsWidget, &PlotsWidget::setSelectedPoint);
    connect(generator, &GeneratorGL::outputTextureChanged, plotsWidget, &PlotsWidget::setTextureID);
    connect(generator, &GeneratorGL::imageSizeChanged, plotsWidget, &PlotsWidget::setImageSize);
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

void ControlWidget::computePlots()
{
    if (generator->active && plotsWidget->plotsActive())
        plotsWidget->getPixels();
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

    //emit closing();
    event->accept();
}

void ControlWidget::resizeEvent(QResizeEvent* event)
{
    updateScrollArea();
    event->accept();
}

void ControlWidget::constructSystemToolBar()
{
    systemToolBar = new QToolBar;
    iterateAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/media-playback-start.png")), "Start/pause feedback loop");
    iterateAction->setCheckable(true);
    QAction* resetAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/view-refresh.png")), "Reset");
    systemToolBar->addSeparator();
    screenshotAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/digikam.png")), "Take screenshot");
    recordAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/media-record.png")), "Record video");
    recordAction->setCheckable(true);
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
    connect(screenshotAction, &QAction::triggered, this, &ControlWidget::takeScreenshot);
    connect(recordAction, &QAction::triggered, this, &ControlWidget::record);
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
        recordAction->setIcon(QIcon(QPixmap(":/icons/media-playback-stop.png")));
        videoCaptureElapsedTimeLabel->setText("00:00:00.000");

        QString filename = QDir::toNativeSeparators(outputDir + '/' + QDateTime::currentDateTime().toString(Qt::ISODate) + ".avi");
        heart->startRecording(filename, framesPerSecond);
    }
    else
    {
        recordAction->setIcon(QIcon(QPixmap(":/icons/media-record.png")));
        heart->stopRecording();
    }
}

void ControlWidget::takeScreenshot()
{   
    QImage screenshot = heart->grabMorphoWidgetFramebuffer();
    QString filename = QDir::toNativeSeparators(outputDir + '/' + QDateTime::currentDateTime().toString(Qt::ISODate) + ".png");
    screenshot.save(filename);
}

void ControlWidget::toggleDisplayOptionsWidget()
{
    displayOptionsWidget->setVisible(!displayOptionsWidget->isVisible());

    if (displayOptionsWidget->isVisible())
        scrollLayout->addWidget(displayOptionsWidget);
    else
        scrollLayout->removeWidget(displayOptionsWidget);

    updateScrollArea();
}

void ControlWidget::toggleRecordingOptionsWidget()
{
    recordingOptionsWidget->setVisible(!recordingOptionsWidget->isVisible());

    if (recordingOptionsWidget->isVisible())
        scrollLayout->addWidget(recordingOptionsWidget);
    else
        scrollLayout->removeWidget(recordingOptionsWidget);

    updateScrollArea();
}

void ControlWidget::toggleSortedOperationsWidget()
{
    sortedOperationsWidget->setVisible(!sortedOperationsWidget->isVisible());

    if (sortedOperationsWidget->isVisible())
        scrollLayout->addWidget(sortedOperationsWidget);
    else
        scrollLayout->removeWidget(sortedOperationsWidget);

    updateScrollArea();
}

void ControlWidget::plotsActionTriggered()
{
    plotsWidget->setVisible(!plotsWidget->isVisible());
}

void ControlWidget::loadConfig()
{
    QString filename = QFileDialog::getOpenFileName(this, "Load configuration", outputDir, "MorphogenGL configurations (*.morph)");

    if (!filename.isEmpty())
    {
        QList<QUuid> opIDs = generator->getOperationNodesIDs();
        for (QUuid id : opIDs)
            removeParametersWidget(id);

        parser->setFilename(filename);
        parser->read();

        opIDs = generator->getOperationNodesIDs();
        for (QUuid id : opIDs)
            createParametersWidget(id);

        // ToDo: connect with graph widget

        generator->resetIterationNumer();

        updateIterationNumberLabel();
        timePerIterationLabel->setText(QString("mSPF: 0"));
        fpsLabel->setText(QString("FPS: 0"));
    }
}

void ControlWidget::saveConfig()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save configuration", outputDir, "MorphogenGL configurations (*.morph)");

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

            for (QString opName : qAsConst(generator->availableOperations))
                operationsMenu->addAction(opName);

            connect(operationsMenu, &QMenu::triggered, this, &ControlWidget::setNodeOperation);

            QAction* setOperationAction = nodesToolBar->addAction(QIcon(QPixmap(":/icons/applications-system.png")), "Set operation");
            setOperationAction->setMenu(operationsMenu);

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
    if (operationsWidgets.contains(selectedOperationNode->id))
        operationsWidgets.value(selectedOperationNode->id)->toggleEnableButton(checked);
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
        nodesToolBar->addAction(QIcon(QPixmap(":/icons/circle-green.png")), "Enable", graphWidget, &GraphWidget::enableSelectedOperations);
        nodesToolBar->addAction(QIcon(QPixmap(":/icons/circle-grey.png")), "Disable", graphWidget, &GraphWidget::disableSelectedOperations);
        nodesToolBar->addAction(QIcon(QPixmap(":/icons/preferences-desktop.png")), "Equalize blend factors", graphWidget, &GraphWidget::equalizeSelectedBlendFactors);
        nodesToolBar->addSeparator();
        nodesToolBar->addAction(QIcon(QPixmap(":/icons/edit-clear.png")), "Clear", graphWidget, &GraphWidget::clearSelectedOperationNodes);
        nodesToolBar->addSeparator();

        if (graphWidget->twoOperationNodesSelected())
        {
            nodesToolBar->addAction(QIcon(QPixmap(":/icons/emblem-synchronized.png")), "Swap", graphWidget, &GraphWidget::swapSelectedOperationNodes);
            nodesToolBar->addSeparator();
        }
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

    /*imageWidthLineEdit = new FocusLineEdit;
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
    imageHeightLineEdit->setText(QString::number(generator->getHeight()));*/

    windowWidthLineEdit = new FocusLineEdit;
    windowWidthLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* windowWidthIntValidator = new QIntValidator(0, 8192, windowWidthLineEdit);
    windowWidthIntValidator->setLocale(QLocale::English);
    windowWidthLineEdit->setValidator(windowWidthIntValidator);
    //windowWidthLineEdit->setText(QString::number(heart->getMorphoWidgetWidth()));

    windowHeightLineEdit = new FocusLineEdit;
    windowHeightLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* windowHeightIntValidator = new QIntValidator(0, 8192, windowHeightLineEdit);
    windowHeightIntValidator->setLocale(QLocale::English);
    windowHeightLineEdit->setValidator(windowHeightIntValidator);
    //windowHeightLineEdit->setText(QString::number(heart->getMorphoWidgetHeight()));

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("FPS:", fpsLineEdit);
    //formLayout->addRow("Image width (px):", imageWidthLineEdit);
    //formLayout->addRow("Image height (px):", imageHeightLineEdit);
    formLayout->addRow("Width (px):", windowWidthLineEdit);
    formLayout->addRow("Height (px):", windowHeightLineEdit);

    QGroupBox* displayGroupBox = new QGroupBox("Display options");
    displayGroupBox->setLayout(formLayout);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addWidget(displayGroupBox);

    displayOptionsWidget = new QWidget(scrollWidget);
    displayOptionsWidget->setLayout(mainLayout);
    //displayOptionsWidget->setWindowTitle("Display options");
    displayOptionsWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    displayOptionsWidget->setVisible(false);

    // Signals + Slots

    connect(fpsLineEdit, &FocusLineEdit::returnPressed, this, [=]()
    {
        heart->setTimerInterval(static_cast<int>(1000.0 / fpsLineEdit->text().toInt()));
    });
    connect(fpsLineEdit, &FocusLineEdit::focusOut, this, [=]()
    {
        fpsLineEdit->setText(QString::number(static_cast<int>(1000.0 / heart->getTimerInterval())));
    });

    /*connect(imageWidthLineEdit, &FocusLineEdit::returnPressed, [=]()
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
    });*/

    connect(windowWidthLineEdit, &FocusLineEdit::returnPressed, this, [=]()
    {
        heart->resizeMorphoWidget(windowWidthLineEdit->text().toInt(), windowHeightLineEdit->text().toInt());
    });
    connect(windowWidthLineEdit, &FocusLineEdit::focusOut, this, [=]()
    {
        windowWidthLineEdit->setText(QString::number(heart->getMorphoWidgetWidth()));
    });
    connect(windowHeightLineEdit, &FocusLineEdit::returnPressed, this, [=]()
    {
        heart->resizeMorphoWidget(windowWidthLineEdit->text().toInt(), windowHeightLineEdit->text().toInt());
    });
    connect(windowHeightLineEdit, &FocusLineEdit::focusOut, this, [=]()
    {
        windowHeightLineEdit->setText(QString::number(heart->getMorphoWidgetHeight()));
    });
}

void ControlWidget::constructRecordingOptionsWidget()
{
    QPushButton* videoFilenamePushButton = new QPushButton(QIcon(QPixmap(":/icons/document-open.png")), "Select output dir");
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

    QFormLayout* videoFormLayout = new QFormLayout;
    videoFormLayout->addRow("Speed:", presetsVideoComboBox);
    videoFormLayout->addRow("Quality:", crfVideoLineEdit);
    videoFormLayout->addRow("FPS:", fpsVideoLineEdit);
    videoFormLayout->addRow("Elapsed time:", videoCaptureElapsedTimeLabel);

    QVBoxLayout* videoVBoxLayout = new QVBoxLayout;
    videoVBoxLayout->addWidget(videoFilenamePushButton);
    videoVBoxLayout->addLayout(videoFormLayout);

    QGroupBox* videoGroupBox = new QGroupBox("Capture options");
    videoGroupBox->setLayout(videoVBoxLayout);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addWidget(videoGroupBox);

    recordingOptionsWidget = new QWidget(scrollWidget);
    recordingOptionsWidget->setLayout(mainLayout);
    recordingOptionsWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    recordingOptionsWidget->setVisible(false);

    // Signals + Slots

    connect(videoFilenamePushButton, &QPushButton::clicked, this, &ControlWidget::setOutputDir);
    connect(presetsVideoComboBox, &QComboBox::currentTextChanged, this, [=](QString thePreset)
    {
        preset = thePreset;
    });
    connect(crfVideoLineEdit, &FocusLineEdit::returnPressed, this, [=]()
    {
        crf = crfVideoLineEdit->text().toInt();
    });
    connect(crfVideoLineEdit, &FocusLineEdit::focusOut, this, [=]()
    {
        crfVideoLineEdit->setText(QString::number(crf));
    });
    connect(fpsVideoLineEdit, &FocusLineEdit::returnPressed, this, [=]()
    {
        framesPerSecond = fpsVideoLineEdit->text().toInt();
    });
    connect(fpsVideoLineEdit, &FocusLineEdit::focusOut, this, [=]()
    {
        fpsVideoLineEdit->setText(QString::number(framesPerSecond));
    });
    connect(heart, &Heart::frameRecorded, this, &ControlWidget::setVideoCaptureElapsedTimeLabel);
}

void ControlWidget::setOutputDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select output directory", QDir::homePath());

    if (!dir.isEmpty())
        outputDir = dir;
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

     sortedOperationsWidget = new QWidget(scrollWidget);
     sortedOperationsWidget->setLayout(layout);
     sortedOperationsWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
     sortedOperationsWidget->setVisible(false);

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

void ControlWidget::createParametersWidget(QUuid id)
{
    if (generator->hasOperationParamaters(id) && !operationsWidgets.contains(id))
    {
        operationsWidgets.insert(id, new OperationsWidget(generator->getOperation(id), scrollWidget));

        connect(operationsWidgets.value(id), &OperationsWidget::enableButtonToggled, this, [=]()
        {
            if (nodesToolBar->isVisible() && graphWidget->singleOperationNodeSelected() && graphWidget->isOperationNodeSelected(id))
                constructSingleNodeToolBar(graphWidget->getNode(id));
        });
        connect(operationsWidgets.value(id), &OperationsWidget::focusOut, this, &ControlWidget::removeOneParametersWidgetBorder);
        connect(operationsWidgets.value(id), &OperationsWidget::focusIn, this, &ControlWidget::updateParametersWidgetsBorder);

        operationsWidgets.value(id)->setVisible(true);
        //updateScrollLayout(operationsWidgets.value(id));
        scrollLayout->addWidget(operationsWidgets.value(id));
        scrollArea->ensureWidgetVisible(operationsWidgets.value(id));
        //operationsWidgets.value(id)->setFocus();
        updateScrollArea();
    }
}

void ControlWidget::updateParametersWidgetsBorder(QWidget* widget)
{
    QMapIterator<QUuid, OperationsWidget*> it(operationsWidgets);
    while (it.hasNext())
    {
        it.next();
        if (it.value() == widget)
        {
            if (!graphWidget->isOperationNodeSelected(it.key()))
            {
                graphWidget->selectNode(it.key(), true);
                if (!it.value()->isFocused())
                    it.value()->setFocus();
            }
        }
        else
        {
            if (graphWidget->isOperationNodeSelected(it.key()))
                graphWidget->selectNode(it.key(), false);
        }
    }
}

void ControlWidget::removeAllParametersWidgetsBorder()
{
    foreach (OperationsWidget* widget, operationsWidgets)
        widget->setLastFocused(false);
}

void ControlWidget::removeOneParametersWidgetBorder(QWidget* widget)
{
    QMapIterator<QUuid, OperationsWidget*> it(operationsWidgets);
    while (it.hasNext())
    {
        it.next();
        if (it.value() == widget && !it.value()->isLastFocused())
        {
            graphWidget->selectNode(it.key(), false);
            break;
        }
    }
}

void ControlWidget::showParametersWidget(QUuid id)
{
    if (generator->hasOperationParamaters(id) && operationsWidgets.contains(id))
    {
        /*if (operationsWidgets.value(id)->isVisible())
        {
            scrollArea->ensureWidgetVisible(operationsWidgets.value(id));
        }
        else
        {
            operationsWidgets.value(id)->setVisible(true);
            updateScrollLayout(operationsWidgets.value(id));
        }*/

        //updateParametersWidgetsBorder(operationsWidgets.value(id));
        if (!operationsWidgets.value(id)->isFocused())
            operationsWidgets.value(id)->setFocus();
        scrollArea->ensureWidgetVisible(operationsWidgets.value(id));
        updateScrollArea();
    }
}

void ControlWidget::removeParametersWidget(QUuid id)
{
    if (operationsWidgets.contains(id))
    {
        operationsWidgets.value(id)->setVisible(false);
        //updateScrollLayout(operationsWidgets.value(id));
        scrollLayout->removeWidget(operationsWidgets.value(id));
        updateScrollArea();

        operationsWidgets.value(id)->deleteLater();
        operationsWidgets.remove(id);
    }
}

void ControlWidget::updateParametersWidget(QUuid id)
{
    if (operationsWidgets.contains(id))
    {
        if (generator->getOperation(id)->hasParameters())
        {
            operationsWidgets.value(id)->recreate(generator->getOperation(id));

            if (operationsWidgets.value(id)->isVisible())
                QTimer::singleShot(10, this, [&]{ updateScrollArea(); });
        }
        else
            removeParametersWidget(id);
    }
}

void ControlWidget::updateScrollLayout(QWidget* widget)
{
    if (widget->isVisible())
        scrollLayout->addWidget(widget);
    else
        scrollLayout->removeWidget(widget);

    updateScrollArea();
}

void ControlWidget::updateScrollArea()
{
    int visible = false;
    QList<QWidget*> children = scrollWidget->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget* child : qAsConst(children))
    {
        if (child->isVisible())
        {
            visible = true;
            break;
        }
    }

    scrollWidget->adjustSize();

    if (visible)
    {
        scrollArea->setFixedHeight(scrollWidget->height() + scrollLayout->contentsMargins().top());
        if (scrollArea->horizontalScrollBar()->isVisible())
            scrollArea->setFixedHeight(scrollWidget->height() + scrollLayout->contentsMargins().top() + scrollArea->horizontalScrollBar()->height());
    }
    else
    {
        scrollArea->setFixedHeight(0);
    }
}
