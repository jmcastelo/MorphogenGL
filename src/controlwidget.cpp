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

#include "configparser.h"
//#include "node.h"
//#include "blendfactorwidget.h"
#include "controlwidget.h"

#include <QTimer>
#include <QActionGroup>
#include <QHeaderView>
#include <QScrollBar>
#include <QSplitter>
#include <QDebug>



ControlWidget::ControlWidget(double itFPS, double updFPS, NodeManager *nodeManager, RenderManager *renderManager, PlotsWidget *thePlotsWidget, QWidget *parent) :
    QWidget(parent),
    mNodeManager { nodeManager },
    mRenderManager { renderManager },
    plotsWidget { thePlotsWidget }
{
    // Scroll area

    /*scrollLayout = new QHBoxLayout;
    scrollLayout->setSizeConstraint(QLayout::SetFixedSize);
    scrollLayout->setAlignment(Qt::AlignBottom);
    scrollLayout->setDirection(QBoxLayout::RightToLeft);

    scrollWidget = new QWidget;
    scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    scrollWidget->setLayout(scrollLayout);

    scrollArea = new QScrollArea;
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrollWidget);

    // Contruct nodes toolbar

    nodesToolBar = new QToolBar;
    nodesToolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    nodesToolBar->setOrientation(Qt::Vertical);
    nodesToolBar->hide();*/

    constructSystemToolBar();
    constructDisplayOptionsWidget(itFPS, updFPS);
    constructRecordingOptionsWidget();
    constructSortedOperationWidget();

    //updateScrollArea();

    // Graph widget

    graphWidget = new GraphWidget(mNodeManager);
    graphWidget->setMinimumSize(0, 0);
    graphWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    /*connect(graphWidget, &GraphWidget::singleNodeSelected, this, &ControlWidget::constructSingleNodeToolBar);
    connect(graphWidget, &GraphWidget::operationNodeSelected, this, &ControlWidget::showParametersWidget);
    connect(graphWidget, &GraphWidget::noOperationNodesSelected, this, &ControlWidget::removeAllParametersWidgetsBorder);
    connect(graphWidget, &GraphWidget::multipleNodesSelected, this, &ControlWidget::constructMultipleNodesToolBar);
    connect(graphWidget, &GraphWidget::showOperationParameters, this, &ControlWidget::createParametersWidget);
    connect(graphWidget, &GraphWidget::removeOperationParameters, this, &ControlWidget::removeParametersWidget);
    connect(graphWidget, &GraphWidget::updateOperationParameters, this, &ControlWidget::updateParametersWidget);
    connect(graphWidget, &GraphWidget::blendFactorWidgetCreated, this, &ControlWidget::setUpBlendFactorWidget);
    connect(graphWidget, &GraphWidget::blendFactorWidgetToggled, this, [&](BlendFactorWidget* widget){
        if (widget->isVisible())
        {
            if (scrollLayout->indexOf(widget) == -1)
            {
                scrollLayout->addWidget(widget, 0, Qt::AlignLeft | Qt::AlignBottom);
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
    connect(graphWidget, &GraphWidget::blendFactorWidgetDeleted, this, [&](BlendFactorWidget* widget){
        blendFactorWidgets.remove(widget->id);
    });
    connect(graphWidget, &GraphWidget::operationEnabled, this, [&](QUuid id, bool enabled){
        if (operationsWidgets.contains(id))
            operationsWidgets.value(id)->toggleEnableButton(enabled);
    });*/

    // Parser

    parser = new ConfigurationParser(mNodeManager, mRenderManager, graphWidget);

    // Status bar

    statusBar = new QStatusBar;
    statusBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    statusBar->setSizeGripEnabled(false);

    iterationNumberLabel = new QLabel("Frame: 0");
    timePerIterationLabel = new QLabel("uSPF: 0");
    iterationFPSLabel = new QLabel("FPS: 0");
    timePerUpdateLabel = new QLabel("uSPF: 0");
    updateFPSLabel = new QLabel("FPS: 0");

    statusBar->insertWidget(0, iterationNumberLabel, 4);
    statusBar->insertWidget(1, timePerIterationLabel, 1);
    statusBar->insertWidget(2, iterationFPSLabel, 1);
    statusBar->insertWidget(3, timePerUpdateLabel, 1);
    statusBar->insertWidget(4, updateFPSLabel, 1);

    // Main layout

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setSizeConstraint(QLayout::SetNoConstraint);
    hLayout->addWidget(graphWidget);
    //hLayout->addWidget(nodesToolBar);

    QWidget* widget = new QWidget;
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    widget->setLayout(hLayout);

    QSplitter* splitter = new QSplitter;
    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    splitter->setOrientation(Qt::Vertical);
    splitter->setChildrenCollapsible(true);
    splitter->addWidget(widget);
    splitter->addWidget(plotsWidget);

    QVBoxLayout* mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->setSizeConstraint(QLayout::SetNoConstraint);
    mainVBoxLayout->addWidget(systemToolBar);
    mainVBoxLayout->addWidget(splitter);
    //mainVBoxLayout->addWidget(scrollArea);
    mainVBoxLayout->addWidget(statusBar);

    setLayout(mainVBoxLayout);

    // Needed over morphoWidget

    setAutoFillBackground(true);
    setWindowFlags(Qt::SubWindow);

    // Signals + Slots

    connect(parser, &ConfigurationParser::newImageSizeRead, this, &ControlWidget::imageSizeChanged);
}



ControlWidget::~ControlWidget()
{
    delete displayOptionsWidget;
    delete recordingOptionsWidget;
    delete parser;
    delete graphWidget;
    delete sortedOperationWidget;
    qDeleteAll(operationsWidgets);
    //qDeleteAll(blendFactorWidgets);
}



void ControlWidget::closeEvent(QCloseEvent* event)
{
    displayOptionsWidget->close();
    graphWidget->close();

    event->accept();
}



/*void ControlWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateScrollArea();
}*/



void ControlWidget::constructSystemToolBar()
{
    systemToolBar = new QToolBar;
    systemToolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    systemToolBar->setMinimumSize(0, 0);

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

    //systemToolBar->addSeparator();

    //systemToolBar->addAction(QIcon(QPixmap(":/icons/format-list-ordered.png")), "List sorted operations", this, &ControlWidget::toggleSortedOperationWidget);

    systemToolBar->addSeparator();

    systemToolBar->addAction(QIcon(QPixmap(":/icons/office-chart-area-stacked.png")), "Show plots", this, &ControlWidget::plotsActionTriggered);

    systemToolBar->addSeparator();

    loadConfigAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/document-open.png")), "Load configuration");
    saveConfigAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/document-save.png")), "Save configuration");

    systemToolBar->addSeparator();

    QAction* midiAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/control-knob.png")), "MIDI Controller");
    QAction* overlayAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/align-horizontal-left.png")), "Overlay");

    systemToolBar->addSeparator();

    QAction* aboutAction = systemToolBar->addAction(QIcon(QPixmap(":/icons/help-about.png")), "About");

    connect(iterateAction, &QAction::triggered, this, &ControlWidget::iterate);
    connect(resetAction, &QAction::triggered, this, &ControlWidget::reset);
    connect(screenshotAction, &QAction::triggered, this, &ControlWidget::setScreenshotFilename);
    connect(recordAction, &QAction::triggered, this, &ControlWidget::record);
    //connect(displayOptionsAction, &QAction::toggled, displayOptionsWidget, &QWidget::setVisible);
    connect(displayOptionsAction, &QAction::triggered, this, &ControlWidget::toggleDisplayOptionsWidget);
    //connect(recordingOptionsAction, &QAction::triggered, this, &ControlWidget::toggleRecordingOptionsWidget);
    //connect(loadConfigAction, &QAction::triggered, this, &ControlWidget::loadConfig);
    //connect(saveConfigAction, &QAction::triggered, this, &ControlWidget::saveConfig);
    connect(midiAction, &QAction::triggered, this, &ControlWidget::showMidiWidget);
    connect(overlayAction, &QAction::triggered, this, &ControlWidget::toggleOverlay);
    connect(aboutAction, &QAction::triggered, this, &ControlWidget::about);
}



void ControlWidget::iterate()
{
    if (iterateAction->isChecked())
        iterateAction->setIcon(QIcon(QPixmap(":/icons/media-playback-pause.png")));
    else
        iterateAction->setIcon(QIcon(QPixmap(":/icons/media-playback-start.png")));

    emit iterateStateChanged(iterateAction->isChecked());
}



void ControlWidget::reset()
{
    mNodeManager->clearAllOperations();
    mNodeManager->drawAllSeeds();
    mRenderManager->resetIterationNumer();
}



void ControlWidget::record()
{
    if (recordAction->isChecked())
    {
        recordAction->setIcon(QIcon(QPixmap(":/icons/media-playback-stop.png")));
        videoCaptureElapsedTimeLabel->setText("00:00:00.000");
        QString filename = QDir::toNativeSeparators(outputDir + '/' + QDateTime::currentDateTime().toString(Qt::ISODate));
        emit startRecording(filename, framesPerSecond, format);
    }
    else
    {
        recordAction->setIcon(QIcon(QPixmap(":/icons/media-record.png")));
        emit stopRecording();
    }
}



void ControlWidget::setScreenshotFilename()
{   
    QString filename = QDir::toNativeSeparators(outputDir + '/' + QDateTime::currentDateTime().toString(Qt::ISODate) + ".png");
    emit takeScreenshot(filename);
}



void ControlWidget::toggleDisplayOptionsWidget()
{
    displayOptionsWidget->setVisible(!displayOptionsWidget->isVisible());
}



/*void ControlWidget::toggleRecordingOptionsWidget()
{
    recordingOptionsWidget->setVisible(!recordingOptionsWidget->isVisible());

    if (recordingOptionsWidget->isVisible())
    {
        scrollLayout->addWidget(recordingOptionsWidget);
        scrollArea->ensureWidgetVisible(recordingOptionsWidget);
    }
    else
        scrollLayout->removeWidget(recordingOptionsWidget);

    updateScrollArea();
}*/



/*void ControlWidget::toggleSortedOperationWidget()
{
    sortedOperationWidget->setVisible(!sortedOperationWidget->isVisible());

    if (sortedOperationWidget->isVisible())
    {
        scrollLayout->addWidget(sortedOperationWidget);
        scrollArea->ensureWidgetVisible(sortedOperationWidget);
    }
    else
        scrollLayout->removeWidget(sortedOperationWidget);

    updateScrollArea();
}*/



void ControlWidget::plotsActionTriggered()
{
    plotsWidget->setVisible(!plotsWidget->isVisible());
}



/*void ControlWidget::loadConfig()
{
    QString filename = QFileDialog::getOpenFileName(this, "Load configuration", outputDir, "MorphogenGL configurations (*.morph)");

    if (!filename.isEmpty())
    {
        QList<QUuid> opIDs = generator->getOperationNodesIDs();
        foreach (QUuid id, opIDs)
        {
            removeParametersWidget(id);
        }

        parser->setFilename(filename);
        parser->read();

        opIDs = generator->getOperationNodesIDs();
        foreach (QUuid id, opIDs)
        {
            createParametersWidget(id);
        }

        generator->sortOperations();

        // ToDo: connect with graph widget

        generator->resetIterationNumer();

        updateIterationNumberLabel();
        updateIterationMetricsLabels(0, 0);
        updateUpdateMetricsLabels(0, 0);
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
}*/



void ControlWidget::toggleOverlay()
{
    overlayEnabled = !overlayEnabled;

    emit overlayToggled(overlayEnabled);

    if (overlayEnabled)
    {
        foreach (QUuid id, operationsWidgets.keys())
            overlayLinkParametersWidget(id);

        //foreach (QUuid id, blendFactorWidgets.keys())
            //overlayLinkBlendFactorWidget(id);
    }
    else
    {
        foreach (QUuid id, operationsWidgets.keys())
            overlayUnlinkParametersWidget(id);

        //foreach (QUuid id, blendFactorWidgets.keys())
            //overlayUnlinkBlendFactorWidget(id);
    }
}



void ControlWidget::about()
{
    QMessageBox* aboutBox = new QMessageBox(this);

    aboutBox->setTextFormat(Qt::RichText);

    aboutBox->setWindowTitle("About");

    QStringList lines;
    lines.append(QString("<h2>F&oacute;sforo %1</h2>").arg(mNodeManager->version));
    lines.append("<h4>Videofeedback simulation software.</h4>");
    lines.append("<h5>Let the pixels come alive!</h5><br>");
    lines.append("Looking for help? Please visit:<br>");
    lines.append("<a href='https://github.com/jmcastelo/MorphogenGL'>https://github.com/jmcastelo/MorphogenGL</a>");

    QString text = lines.join("");

    aboutBox->setText(text);

    aboutBox->setInformativeText("Copyright 2024 Jose Maria Castelo Ares\nLicense: GPLv3");

    aboutBox->exec();
}



void ControlWidget::updateIterationNumberLabel()
{
    iterationNumberLabel->setText(QString("Frame: %1").arg(mRenderManager->iterationNumber()));
}



void ControlWidget::updateIterationMetricsLabels(double uspf, double fps)
{
    timePerIterationLabel->setText(QString("uSPF: %1").arg(uspf));
    iterationFPSLabel->setText(QString("FPS: %1").arg(fps));
}



void ControlWidget::updateUpdateMetricsLabels(double uspf, double fps)
{
    timePerUpdateLabel->setText(QString("uSPF: %1").arg(uspf));
    updateFPSLabel->setText(QString("FPS: %1").arg(fps));
}



void ControlWidget::updateWindowSizeLineEdits(int width, int height)
{
    windowWidthLineEdit->setText(QString::number(width));
    windowHeightLineEdit->setText(QString::number(height));
}



// Nodes toolbar: single node selected
/*
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

            foreach (QString opName, generator->availableOperations)
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

    if (graphWidget->operationNodesSelected() > 0)
    {
        nodesToolBar->addAction(QIcon(QPixmap(":/icons/circle-green.png")), "Enable", graphWidget, &GraphWidget::enableSelectedOperations);
        nodesToolBar->addAction(QIcon(QPixmap(":/icons/circle-grey.png")), "Disable", graphWidget, &GraphWidget::disableSelectedOperations);
        if (graphWidget->selectedOperationNodesHaveInputs())
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
*/


// Display

void ControlWidget::constructDisplayOptionsWidget(double itsFPS, double updFPS)
{
    FocusLineEdit* itsFPSLineEdit = new FocusLineEdit;
    itsFPSLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QDoubleValidator* itsFPSDoubleValidator = new QDoubleValidator(1.0, 1000.0, 3, itsFPSLineEdit);
    itsFPSDoubleValidator->setLocale(QLocale::English);
    itsFPSLineEdit->setValidator(itsFPSDoubleValidator);
    itsFPSLineEdit->setText(QString::number(itsFPS));

    FocusLineEdit* updFPSLineEdit = new FocusLineEdit;
    updFPSLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QDoubleValidator* updFPSDoubleValidator = new QDoubleValidator(1.0, 1000.0, 3, updFPSLineEdit);
    updFPSDoubleValidator->setLocale(QLocale::English);
    updFPSLineEdit->setValidator(updFPSDoubleValidator);
    updFPSLineEdit->setText(QString::number(updFPS));

    windowWidthLineEdit = new QLineEdit;
    windowWidthLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* windowWidthIntValidator = new QIntValidator(0, 8192, windowWidthLineEdit);
    windowWidthIntValidator->setLocale(QLocale::English);
    windowWidthLineEdit->setValidator(windowWidthIntValidator);

    windowHeightLineEdit = new QLineEdit;
    windowHeightLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* windowHeightIntValidator = new QIntValidator(0, 8192, windowHeightLineEdit);
    windowHeightIntValidator->setLocale(QLocale::English);
    windowHeightLineEdit->setValidator(windowHeightIntValidator);

    texFormatComboBox = new QComboBox;
    texFormatComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    QCheckBox* updateCheckBox = new QCheckBox;
    updateCheckBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    updateCheckBox->setChecked(true);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("Its FPS:", itsFPSLineEdit);
    formLayout->addRow("Upd FPS:", updFPSLineEdit);
    formLayout->addRow("Update:", updateCheckBox);
    formLayout->addRow("Width (px):", windowWidthLineEdit);
    formLayout->addRow("Height (px):", windowHeightLineEdit);
    formLayout->addRow("Format:", texFormatComboBox);

    displayOptionsWidget = new QWidget;
    displayOptionsWidget->setWindowTitle("Display options");
    displayOptionsWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    displayOptionsWidget->setVisible(false);
    displayOptionsWidget->setLayout(formLayout);

    // Signals + Slots

    connect(itsFPSLineEdit, &FocusLineEdit::editingFinished, this, [=, this]()
    {
        double fps = itsFPSLineEdit->text().toDouble();
        emit iterationFPSChanged(fps);
    });

    connect(updFPSLineEdit, &FocusLineEdit::editingFinished, this, [=, this]()
    {
        double fps = updFPSLineEdit->text().toDouble();
        emit updateFPSChanged(fps);
    });

    connect(updateCheckBox, &QCheckBox::checkStateChanged, this, [=, this](Qt::CheckState state){
        emit updateStateChanged(state == Qt::Checked);
    });

    connect(windowWidthLineEdit, &FocusLineEdit::editingFinished, this, [=, this]()
    {
        emit imageSizeChanged(windowWidthLineEdit->text().toInt(), windowHeightLineEdit->text().toInt());
    });
    connect(windowHeightLineEdit, &FocusLineEdit::editingFinished, this, [=, this]()
    {
        emit imageSizeChanged(windowWidthLineEdit->text().toInt(), windowHeightLineEdit->text().toInt());
    });

    connect(texFormatComboBox, &QComboBox::activated, this, [&](int index)
    {
        int selectedValue = texFormatComboBox->itemData(index).toInt();
        TextureFormat selectedFormat = static_cast<TextureFormat>(selectedValue);
        mRenderManager->setTextureFormat(selectedFormat);
    });
}



QString ControlWidget::textureFormatToString(TextureFormat format) {
    switch (format)
    {
        case TextureFormat::RGBA2: return "RGBA2";
        case TextureFormat::RGBA4: return "RGBA4";
        case TextureFormat::RGBA8: return "RGBA8";
        case TextureFormat::RGBA8_SNORM: return "RGBA8_SNORM";
        case TextureFormat::RGB10_A2: return "RGB10_A2";
        //case TextureFormat::RGB10_A2UI: return "RGB10_A2";
        case TextureFormat::RGBA12: return "RGBA12";
        //case TextureFormat::SRGB8_ALPHA8: return "SRGB8_ALPHA8";
        case TextureFormat::RGBA16: return "RGBA16";
        case TextureFormat::RGBA16F: return "RGBA16F";
        case TextureFormat::RGBA32F: return "RGBA32F";
        //case TextureFormat::RGBA8I: return "RGBA8I";
        //case TextureFormat::RGBA8UI: return "RGBA8UI";
        //case TextureFormat::RGBA16I: return "RGBA16I";
        //case TextureFormat::RGBA16UI: return "RGBA16UI";
        //case TextureFormat::RGBA32I: return "RGBA32I";
        //case TextureFormat::RGBA32UI: return "RGBA32UI";
        default: return "Unknown";
    }
}



void ControlWidget::populateTexFormatComboBox(QList<TextureFormat> formats)
{
    int index = 0;
    foreach (TextureFormat format, formats)
    {
        texFormatComboBox->addItem(textureFormatToString(format), QVariant(static_cast<int>(format)));
        if (format == mRenderManager->texFormat())
            texFormatComboBox->setCurrentIndex(index);
        index++;
    }
}



void ControlWidget::constructRecordingOptionsWidget()
{
    QPushButton* videoFilenamePushButton = new QPushButton(QIcon(QPixmap(":/icons/document-open.png")), "Select output dir");
    videoFilenamePushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    fileFormatsComboBox = new QComboBox;
    fileFormatsComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    populateFileFormatsComboBox();

    videoCodecsComboBox = new QComboBox;
    videoCodecsComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    populateVideoCodecsComboBox();

    FocusLineEdit* fpsVideoLineEdit = new FocusLineEdit;
    fpsVideoLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* fpsVideoValidator = new QIntValidator(1, 1000, fpsVideoLineEdit);
    fpsVideoValidator->setLocale(QLocale::English);
    fpsVideoLineEdit->setValidator(fpsVideoValidator);
    fpsVideoLineEdit->setText(QString::number(framesPerSecond));

    videoCaptureElapsedTimeLabel = new QLabel("00:00:00.000");
    videoCaptureElapsedTimeLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    QFormLayout* videoFormLayout = new QFormLayout;
    videoFormLayout->addRow("File format:", fileFormatsComboBox);
    videoFormLayout->addRow("Codec:", videoCodecsComboBox);
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

    recordingOptionsWidget = new QWidget;
    recordingOptionsWidget->setLayout(mainLayout);
    recordingOptionsWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    recordingOptionsWidget->setVisible(false);

    // Signals + Slots

    connect(videoFilenamePushButton, &QPushButton::clicked, this, &ControlWidget::setOutputDir);
    connect(fileFormatsComboBox, &QComboBox::activated, this, [=, this](int index)
    {
        if (index >= 0)
        {
            format.setFileFormat(supportedFileFormats[index]);
            populateVideoCodecsComboBox();
        }
    });
    connect(videoCodecsComboBox, &QComboBox::activated, this, [=, this](int index)
    {
        if (index >= 0)
        {
            format.setVideoCodec(supportedVideoCodecs[index]);
            populateFileFormatsComboBox();
        }
    });
    connect(fpsVideoLineEdit, &FocusLineEdit::editingFinished, this, [=, this]()
    {
        framesPerSecond = fpsVideoLineEdit->text().toDouble();
    });
}



void ControlWidget::populateFileFormatsComboBox()
{
    QString previousFormat = fileFormatsComboBox->currentText();

    fileFormatsComboBox->clear();

    format.resolveForEncoding(QMediaFormat::RequiresVideo);

    supportedFileFormats = format.supportedFileFormats(QMediaFormat::Encode);
    foreach (QMediaFormat::FileFormat fileFormat, supportedFileFormats)
        fileFormatsComboBox->addItem(QMediaFormat::fileFormatName(fileFormat));

    if (!supportedFileFormats.isEmpty())
    {
        int index = fileFormatsComboBox->findText(previousFormat);
        if (index == -1)
        {
            fileFormatsComboBox->setCurrentIndex(0);
            format.setFileFormat(supportedFileFormats[0]);
        }
        else
        {
            fileFormatsComboBox->setCurrentIndex(index);
        }
    }
    else
    {
        fileFormatsComboBox->setCurrentIndex(-1);
        format.setFileFormat(QMediaFormat::UnspecifiedFormat);
    }
}



void ControlWidget::populateVideoCodecsComboBox()
{
    QString previousCodec = videoCodecsComboBox->currentText();

    videoCodecsComboBox->clear();

    format.resolveForEncoding(QMediaFormat::RequiresVideo);

    supportedVideoCodecs = format.supportedVideoCodecs(QMediaFormat::Encode);
    foreach (QMediaFormat::VideoCodec videoCodec, supportedVideoCodecs)
        videoCodecsComboBox->addItem(QMediaFormat::videoCodecName(videoCodec));

    if (!supportedVideoCodecs.isEmpty())
    {
        int index = videoCodecsComboBox->findText(previousCodec);
        if (index == -1)
        {
            videoCodecsComboBox->setCurrentIndex(0);
            format.setVideoCodec(supportedVideoCodecs[0]);
        }
        else
        {
            videoCodecsComboBox->setCurrentIndex(index);
        }
    }
    else
    {
        videoCodecsComboBox->setCurrentIndex(-1);
        format.setVideoCodec(QMediaFormat::VideoCodec::Unspecified);
    }
}



void ControlWidget::setOutputDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select output directory", outputDir);

    if (!dir.isEmpty())
        outputDir = dir;
}



void ControlWidget::setVideoCaptureElapsedTimeLabel(int frameNumber)
{
    int milliseconds = static_cast<int>(1000.0 * frameNumber / framesPerSecond);

    QTime start(0, 0, 0, 0);

    videoCaptureElapsedTimeLabel->setText(start.addMSecs(milliseconds).toString("hh:mm:ss.zzz"));
}



void ControlWidget::constructSortedOperationWidget()
{
    sortedOperationsTable = new QTableWidget();
    sortedOperationsTable->setColumnCount(1);
    sortedOperationsTable->setHorizontalHeaderLabels(QStringList("Operation"));
    sortedOperationsTable->horizontalHeader()->setStretchLastSection(true);
    sortedOperationsTable->resizeColumnsToContents();
    sortedOperationsTable->setSelectionMode(QAbstractItemView::MultiSelection);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignCenter);
    layout->addWidget(sortedOperationsTable);

    sortedOperationWidget = new QWidget;
    sortedOperationWidget->setLayout(layout);
    sortedOperationWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    sortedOperationWidget->setVisible(false);

    //connect(mNodeManager, &NodeManager::sortedOperationsChanged, this, &ControlWidget::populateSortedOperationsTable);
    //connect(generator, &NodeManager::sortedOperationsChanged, this, &ControlWidget::populateScrollLayout);
    //connect(sortedOperationsTable, &QTableWidget::itemSelectionChanged, this, &ControlWidget::selectNodesToMark);
}



void ControlWidget::populateSortedOperationsTable(QList<QPair<QUuid, QString>> sortedData, QList<QUuid> unsortedData)
{
    Q_UNUSED(unsortedData);

    sortedOperationsTable->clearContents();
    sortedOperationsTable->setRowCount(sortedData.size());

    for (int row = 0; row < sortedData.size(); row++)
    {
        QTableWidgetItem* item = new QTableWidgetItem(sortedData[row].second);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        sortedOperationsTable->setItem(row, 0, item);
    }

    sortedOperationsData = sortedData;
}



/*void ControlWidget::selectNodesToMark()
{
    QVector<QUuid> nodeIds;

    foreach (QTableWidgetItem* item, sortedOperationsTable->selectedItems())
        nodeIds.push_back(sortedOperationsData[item->row()].first);

    graphWidget->markNodes(nodeIds);
}*/



/*void ControlWidget::populateScrollLayout(QList<QPair<QUuid, QString>> sortedData, QList<QUuid> unsortedData)
{
    QWidget *widget = nullptr;

    for (int i = 0; i < sortedData.size(); i++)
    {
        QUuid id = sortedData[i].first;
        if (operationsWidgets.contains(id) && operationsWidgets.value(id)->isFocused())
            widget = operationsWidgets.value(id);
    }

    for (int i = 0; i < unsortedData.size(); i++)
    {
        QUuid id = unsortedData[i];
        if (operationsWidgets.contains(id) && operationsWidgets.value(id)->isFocused())
            widget = operationsWidgets.value(id);
    }

    QList<QWidget*> children = scrollWidget->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    foreach (QWidget* child, children)
        scrollLayout->removeWidget(child);

    for (int i = sortedData.size() - 1; i >= 0; i--)
    {
        QUuid id = sortedData[i].first;
        if (operationsWidgets.contains(id))
            scrollLayout->addWidget(operationsWidgets.value(id), 0, Qt::AlignLeft | Qt::AlignBottom);
    }

    for (int i = unsortedData.size() - 1; i >= 0; i--)
    {
        QUuid id = unsortedData[i];
        if (operationsWidgets.contains(id))
            scrollLayout->addWidget(operationsWidgets.value(id), 0, Qt::AlignLeft | Qt::AlignBottom);
    }

    updateScrollArea();

    if (widget)
    {
        scrollArea->ensureWidgetVisible(widget);
        //widget->setFocus();
    }
}*/



void ControlWidget::setupMidi(QString portName, bool open)
{
    if (open)
    {
        if (!midiFloatLinks.contains(portName))
            midiFloatLinks[portName] = QMap<int, Number<float>*>();

        if (!midiIntLinks.contains(portName))
            midiIntLinks[portName] = QMap<int, Number<int>*>();
    }
    else
    {
        midiFloatLinks.remove(portName);
        midiIntLinks.remove(portName);
    }

    anyMidiPortOpen = !midiFloatLinks.isEmpty() || !midiIntLinks.isEmpty();
    setUpMidiLinks(anyMidiPortOpen);
}



void ControlWidget::setUpMidiLinks(bool midiOn)
{
    for (auto [id, widget] : operationsWidgets.asKeyValueRange())
    {
        if (midiOn)
            midiLinkParametersWidget(id);
        else
            midiUnlinkParametersWidget(id);
    }

    /*for (auto [id, widget] : blendFactorWidgets.asKeyValueRange())
    {
        if (midiOn)
            midiLinkBlendFactorWidget(id);
        else
            midiUnlinkBlendFactorWidget(id);
    }*/
}



void ControlWidget::updateMidiLinks(QString portName, int key, int value)
{
    if (linkingFloat != nullptr)
    {
        if (!midiFloatLinks.contains(portName))
            midiFloatLinks[portName] = QMap<int, Number<float>*>();

        if (midiFloatLinks[portName].contains(key))
        {
            midiFloatLinks[portName][key]->setIndexMax(100'000);
            midiFloatLinks[portName][key]->setMidiLinked(false);
            midiFloatLinks[portName].remove(key);
        }

        linkingFloat->setIndexMax(127);
        linkingFloat->setMidiLinked(true);

        connect(linkingFloat, &Number<float>::deleting, this, [=, this]()
        {
            midiFloatLinks[portName].remove(key);
        });

        midiFloatLinks[portName][key] = linkingFloat;
        linkingFloat = nullptr;
    }
    else if (linkingInt != nullptr)
    {
        if (!midiIntLinks.contains(portName))
            midiIntLinks[portName] = QMap<int, Number<int>*>();

        if (midiIntLinks[portName].contains(key))
        {
            midiIntLinks[portName][key]->setIndexMax(100'000);
            midiIntLinks[portName][key]->setMidiLinked(false);
            midiIntLinks[portName].remove(key);
        }

        linkingInt->setIndexMax(127);
        linkingInt->setMidiLinked(true);

        connect(linkingInt, &Number<int>::deleting, this, [=, this]()
        {
            midiIntLinks[portName].remove(key);
        });

        midiIntLinks[portName][key] = linkingInt;
        linkingInt = nullptr;
    }
    else
    {
        if (midiFloatLinks.contains(portName) && midiFloatLinks[portName].contains(key))
        {
            midiFloatLinks[portName][key]->setValueFromIndex(value);
            midiFloatLinks[portName][key]->setIndex();
        }
        if (midiIntLinks.contains(portName) && midiIntLinks[portName].contains(key))
        {
            midiIntLinks[portName][key]->setValueFromIndex(value);
            midiIntLinks[portName][key]->setIndex();
        }
    }
}



/*void ControlWidget::createParametersWidget(QUuid id)
{
    if (generator->hasOperationParamaters(id) && !operationsWidgets.contains(id))
    {
        // Create OperationWidget and insert it in QMap

        operationsWidgets.insert(id, new OperationWidget(generator->getOperation(id), anyMidiPortOpen, scrollWidget));

        // Connections

        connect(operationsWidgets.value(id), &OperationWidget::enableButtonToggled, this, [=, this]()
        {
            if (nodesToolBar->isVisible() && graphWidget->singleOperationNodeSelected() && graphWidget->isOperationNodeSelected(id))
                constructSingleNodeToolBar(graphWidget->getNode(id));
        });

        connect(operationsWidgets.value(id), &OperationWidget::focusOut, this, &ControlWidget::removeOneParametersWidgetBorder);
        connect(operationsWidgets.value(id), &OperationWidget::focusIn, this, &ControlWidget::updateParametersWidgetsBorder);

        // MIDI

        if (anyMidiPortOpen)
            midiLinkParametersWidget(id);

        // Overlay

        if (overlayEnabled)
            overlayLinkParametersWidget(id);

        //scrollLayout->insertWidget(0, operationsWidgets.value(id));
        scrollLayout->addWidget(operationsWidgets.value(id));
        scrollArea->ensureWidgetVisible(operationsWidgets.value(id));
        operationsWidgets.value(id)->setFocus();
    }
}*/



void ControlWidget::midiLinkParametersWidget(QUuid id)
{
    operationsWidgets.value(id)->toggleMidiButton(anyMidiPortOpen);

    connect(operationsWidgets.value(id), QOverload<Number<float>*>::of(&OperationWidget::linkWait), this, [=, this](Number<float>* number)
    {
        linkingFloat = number;
    });
    connect(operationsWidgets.value(id), QOverload<Number<int>*>::of(&OperationWidget::linkWait), this, [=, this](Number<int>* number)
    {
        linkingInt = number;
    });

    connect(operationsWidgets.value(id), QOverload<Number<float>*>::of(&OperationWidget::linkBreak), this, [=, this](Number<float>* number)
    {
        for (auto [port, map] : midiFloatLinks.asKeyValueRange())
        {
            for (auto [key, n] : map.asKeyValueRange())
            {
                if (n == number)
                {
                    number->setMidiLinked(false);
                    midiFloatLinks[port].remove(key);
                    break;
                }
            }
        }
    });
    connect(operationsWidgets.value(id), QOverload<Number<int>*>::of(&OperationWidget::linkBreak), this, [=, this](Number<int>* number)
    {
        for (auto [port, map] : midiIntLinks.asKeyValueRange())
        {
            for (auto [key, n] : map.asKeyValueRange())
            {
                if (n == number)
                {
                    number->setMidiLinked(false);
                    midiFloatLinks[port].remove(key);
                    break;
                }
            }
        }
    });
}



void ControlWidget::midiUnlinkParametersWidget(QUuid id)
{
    operationsWidgets.value(id)->toggleMidiButton(anyMidiPortOpen);

    disconnect(operationsWidgets.value(id), QOverload<Number<float>*>::of(&OperationWidget::linkWait), this, nullptr);
    disconnect(operationsWidgets.value(id), QOverload<Number<int>*>::of(&OperationWidget::linkWait), this, nullptr);

    disconnect(operationsWidgets.value(id), QOverload<Number<float>*>::of(&OperationWidget::linkBreak), this, nullptr);
    disconnect(operationsWidgets.value(id), QOverload<Number<int>*>::of(&OperationWidget::linkBreak), this, nullptr);
}



void ControlWidget::overlayLinkParametersWidget(QUuid id)
{
    ImageOperation *operation = mNodeManager->getOperation(id);

    foreach (auto parameter, operation->uniformParameters<float>())
    {
        connect(parameter, QOverload<QVariant>::of(&Parameter::valueChanged), this, [=, this](QVariant value){
            emit parameterValueChanged(id, operation->name(), parameter->name(), QString::number(value.toFloat(), 'f', 6));
        });
    }

    foreach (auto parameter, operation->uniformParameters<int>())
    {
        connect(parameter, QOverload<QVariant>::of(&Parameter::valueChanged), this, [=, this](QVariant value){
            emit parameterValueChanged(id, operation->name(), parameter->name(), QString::number(value.toInt()));
        });
    }

    foreach (auto parameter, operation->uniformParameters<unsigned int>())
    {
        connect(parameter, QOverload<QVariant>::of(&Parameter::valueChanged), this, [=, this](QVariant value){
            emit parameterValueChanged(id, operation->name(), parameter->name(), QString::number(value.toUInt()));
        });
    }
}



void ControlWidget::overlayUnlinkParametersWidget(QUuid id)
{
    ImageOperation *operation = mNodeManager->getOperation(id);

    foreach (auto parameter, operation->uniformParameters<float>())
        disconnect(parameter, QOverload<QVariant>::of(&Parameter::valueChanged), this, nullptr);

    foreach (auto parameter, operation->uniformParameters<int>())
        disconnect(parameter, QOverload<QVariant>::of(&Parameter::valueChanged), this, nullptr);

    foreach (auto parameter, operation->uniformParameters<unsigned int>())
        disconnect(parameter, QOverload<QVariant>::of(&Parameter::valueChanged), this, nullptr);
}



/*void ControlWidget::setUpBlendFactorWidget(BlendFactorWidget* widget)
{
    blendFactorWidgets.insert(widget->id, widget);

    // MIDI

    if (anyMidiPortOpen)
        midiLinkBlendFactorWidget(widget->id);

    // Overlay

    if (overlayEnabled)
        overlayLinkParametersWidget(widget->id);

    widget->setParent(scrollWidget);
    updateScrollArea();
}*/



/*void ControlWidget::midiLinkBlendFactorWidget(QUuid id)
{
    BlendFactorWidget* widget = blendFactorWidgets.value(id);

    widget->toggleMidiButton(anyMidiPortOpen);

    connect(widget, &BlendFactorWidget::linkWait, this, [=, this](Number<float>* number)
    {
        linkingFloat = number;
    });

    connect(widget, &BlendFactorWidget::linkBreak, this, [=, this](Number<float>* number)
    {
        for (auto [port, map] : midiFloatLinks.asKeyValueRange())
        {
            for (auto [key, n] : map.asKeyValueRange())
            {
                if (n == number)
                {
                    number->setMidiLinked(false);
                    midiFloatLinks[port].remove(key);
                    break;
                }
            }
        }
    });
}



void ControlWidget::midiUnlinkBlendFactorWidget(QUuid id)
{
    BlendFactorWidget* widget = blendFactorWidgets.value(id);

    widget->toggleMidiButton(anyMidiPortOpen);

    disconnect(widget, &BlendFactorWidget::linkWait, this, nullptr);
    disconnect(widget, &BlendFactorWidget::linkBreak, this, nullptr);
}



void ControlWidget::overlayLinkBlendFactorWidget(QUuid id)
{
    BlendFactorWidget *widget = blendFactorWidgets.value(id);

    connect(widget->blendFactor, &Number<float>::valueChanged, this, [=, this](QVariant value)
    {
        emit parameterValueChanged(id, "Blend factor", widget->getName(), QString::number(value.toFloat(), 'f', 6));
    });
}



void ControlWidget::overlayUnlinkBlendFactorWidget(QUuid id)
{
    BlendFactorWidget *widget = blendFactorWidgets.value(id);

    disconnect(widget->blendFactor, &Number<float>::valueChanged, this, nullptr);
}
*/


/*void ControlWidget::updateParametersWidgetsBorder(QWidget* widget)
{
    QMapIterator<QUuid, OperationWidget*> it(operationsWidgets);
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
    foreach (OperationWidget* widget, operationsWidgets)
        widget->setLastFocused(false);
}



void ControlWidget::removeOneParametersWidgetBorder(QWidget* widget)
{
    QMapIterator<QUuid, OperationWidget*> it(operationsWidgets);
    while (it.hasNext())
    {
        it.next();
        if (it.value() == widget && !it.value()->isLastFocused())
        {
            graphWidget->selectNode(it.key(), false);
            break;
        }
    }
}*/


/*
void ControlWidget::showParametersWidget(QUuid id)
{
    if (generator->hasOperationParamaters(id) && operationsWidgets.contains(id))
    {
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
        //operationsWidgets.value(id)->setVisible(false);
        //updateScrollLayout(operationsWidgets.value(id));

        //scrollLayout->removeWidget(operationsWidgets.value(id));
        operationsWidgets.value(id)->deleteLater();
        operationsWidgets.remove(id);
        //updateScrollArea();
    }
}



void ControlWidget::updateParametersWidget(QUuid id)
{
    if (operationsWidgets.contains(id))
    {
        if (generator->getOperation(id)->hasParameters())
        {
            //scrollLayout->removeWidget(operationsWidgets.value(id));
            operationsWidgets.value(id)->recreate(generator->getOperation(id), anyMidiPortOpen);

            if (anyMidiPortOpen)
                midiLinkParametersWidget(id);

            if (overlayEnabled)
                overlayLinkParametersWidget(id);

            //scrollLayout->addWidget(operationsWidgets.value(id));

            //scrollArea->ensureWidgetVisible(operationsWidgets.value(id));
            //operationsWidgets.value(id)->setFocus();
            //updateScrollArea();
            //if (operationsWidgets.value(id)->isVisible())
                //QTimer::singleShot(100, this, [&]{ updateScrollArea(); });
        }
        else
            removeParametersWidget(id);

        updateScrollArea();
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
    int maxHeight = 0;
    QList<QWidget*> children = scrollWidget->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    foreach (QWidget* child, children)
    {
        //child->updateGeometry();
        //child->adjustSize();
        if (child->height() > maxHeight)
            maxHeight = child->height();
    }

    scrollLayout->update();
    //scrollWidget->setFixedHeight(maxHeight);
    scrollWidget->updateGeometry();
    scrollWidget->adjustSize();


    if (scrollArea->horizontalScrollBar()->isVisible())
    {
        scrollArea->setFixedHeight(scrollWidget->height() + scrollLayout->contentsMargins().top() + scrollArea->horizontalScrollBar()->height());
        //scrollArea->setFixedHeight(maxHeight + scrollLayout->contentsMargins().top() + scrollArea->horizontalScrollBar()->height());
    }
    else
    {
        scrollArea->setFixedHeight(scrollWidget->height() + scrollLayout->contentsMargins().top());
        //scrollArea->setFixedHeight(maxHeight + scrollLayout->contentsMargins().top());
    }
    update();
}*/
