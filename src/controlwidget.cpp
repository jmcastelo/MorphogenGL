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

#include "controlwidget.h"
#include "morphowidget.h"

ControlWidget::ControlWidget(GeneratorGL *theGenerator, MorphoWidget* theMorphoWidget, QWidget *parent) :
    QWidget(parent),
    generator{ theGenerator },
    morphoWidget{ theMorphoWidget }
{
    // Configuration parser

    parser = new ConfigurationParser(generator, morphoWidget);

    // Actions toolbar

    QToolBar* toolbar = new QToolBar;
    iterateAction = toolbar->addAction(QIcon(QPixmap(":/icons/media-playback-start.png")), "Start/pause feedback loop");
    iterateAction->setCheckable(true);
    recordAction = toolbar->addAction(QIcon(QPixmap(":/icons/media-record.png")), "Record video");
    recordAction->setCheckable(true);
    screenshotAction = toolbar->addAction(QIcon(QPixmap(":/icons/digikam.png")), "Take screenshot");
    toolbar->addSeparator();
    loadConfigAction = toolbar->addAction(QIcon(QPixmap(":/icons/document-open.png")), "Load configuration");
    saveConfigAction = toolbar->addAction(QIcon(QPixmap(":/icons/document-save.png")), "Save configuration");
    toolbar->addSeparator();
    QAction* aboutAction = toolbar->addAction(QIcon(QPixmap(":/icons/help-about.png")), "About");

    connect(iterateAction, &QAction::triggered, this, &ControlWidget::iterate);
    connect(recordAction, &QAction::triggered, this, &ControlWidget::record);
    connect(screenshotAction, &QAction::triggered, this, &ControlWidget::takeScreenshot);
    connect(loadConfigAction, &QAction::triggered, this, &ControlWidget::loadConfig);
    connect(saveConfigAction, &QAction::triggered, this, &ControlWidget::saveConfig);
    connect(aboutAction, &QAction::triggered, this, &ControlWidget::about);

    // Contruct controls

    constructGeneralControls();
    constructPipelineControls();

    // Main tabs

    mainTabWidget = new QTabWidget;
    mainTabWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mainTabWidget->addTab(generalControlsWidget, "General");
    mainTabWidget->addTab(pipelineControlsWidget, "Pipelines");

    connect(mainTabWidget, &QTabWidget::currentChanged, this, &ControlWidget::resizeMainTabs);

    // Status bar

    statusBar = new QStatusBar;
    statusBar->setFont(QFont("sans", 8));
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

    QVBoxLayout* mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addWidget(toolbar);
    mainVBoxLayout->addWidget(mainTabWidget);
    mainVBoxLayout->addWidget(statusBar);

    setLayout(mainVBoxLayout);

    resizeMainTabs(0);

    // Signals + Slots

    connect(morphoWidget, &MorphoWidget::iterationPerformed, this, &ControlWidget::updateIterationNumberLabel);
    connect(morphoWidget, &MorphoWidget::iterationTimeMeasured, this, &ControlWidget::updateMetricsLabels);
    connect(parser, &ConfigurationParser::updateImageSize, [=](int width, int height) { generator->resize(width, height); imageWidthLineEdit->setText(QString::number(width)); imageHeightLineEdit->setText(QString::number(height)); });
    connect(parser, &ConfigurationParser::updateMask, [=](bool applyMask) { generator->setMask(applyMask); applyCircularMaskCheckBox->setChecked(applyMask); });
}

ControlWidget::~ControlWidget()
{
    if (operationsWidget)
        delete operationsWidget;
    delete parser;
}

void ControlWidget::closeEvent(QCloseEvent* event)
{
    morphoWidget->close();
    event->accept();
}

void ControlWidget::iterate()
{
    if (iterateAction->isChecked())
    {
        iterateAction->setIcon(QIcon(QPixmap(":/icons/media-playback-pause.png")));
        morphoWidget->setStartTime();
        generator->active = true;
    }
    else
    {
        iterateAction->setIcon(QIcon(QPixmap(":/icons/media-playback-start.png")));
        generator->active = false;
    }
}

void ControlWidget::record()
{
    if (recordAction->isChecked())
    {
        if (generator->recordFilename.isEmpty())
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
            generator->startRecording(morphoWidget->width(), morphoWidget->height());
        }
    }
    else
    {
        recordAction->setIcon(QIcon(QPixmap(":/icons/media-record.png")));
        generator->stopRecording();
        generator->recordFilename.clear();
    }
}

void ControlWidget::takeScreenshot()
{   
    QRect rectangle = QRect(QPoint(0, 0), QSize(-1, -1));
    QPixmap screenshot = morphoWidget->grab(rectangle);
    
    QString filename = QFileDialog::getSaveFileName(this, "Save image", "", "Images (*.bmp *.cur *.gif *.icns *.ico *.jpeg *.jpg *.pbm *.pgm *.png *.ppm *.tga *.tif *.tiff *.wbmp *.webp *.xbm *.xpm)");
    if (!filename.isEmpty())
    {
        screenshot.save(filename);
    }
}

void ControlWidget::loadConfig()
{
    QString filename = QFileDialog::getOpenFileName(this, "Load configuration", "", "MorphogenCV configurations (*.morph)");

    if (!filename.isEmpty())
    {
        parser->setFilename(filename);
        parser->read();

        for (int i = 0; i < generator->getPipelinesSize(); i++)
            currentImageOperationIndex[i] = 0;

        initPipelineControls(0);

        generator->resetIterationNumer();

        updateIterationNumberLabel();
        updateMetricsLabels(0);
    }
}

void ControlWidget::saveConfig()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save configuration", "", "MorphogenCV configurations (*.morph)");

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
    lines.append("<h2>MorphogenGL 1.0</h2>");
    lines.append("<h4>Videofeedback simulation software.</h4>");
    lines.append("<h5>Let the pixels come alive!</h5><br>");
    lines.append("Looking for help? Please visit:<br>");
    lines.append("<a href='https://github.com/jmcastelo/MorphogenGL'>https://github.com/jmcastelo/MorphogenGL</a>");

    QString text = lines.join("");

    aboutBox->setText(text);

    aboutBox->setInformativeText("Copyright 2020 Jose Maria Castelo Ares\njose.maria.castelo@gmail.com\nLicense: GPLv3");

    aboutBox->exec();
}

void ControlWidget::resizeMainTabs(int index)
{
    for (int i = 0; i < mainTabWidget->count(); i++)
        if (i != index)
            mainTabWidget->widget(i)->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    mainTabWidget->widget(index)->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QApplication::processEvents();
    mainTabWidget->widget(index)->resize(mainTabWidget->widget(index)->minimumSizeHint());
    
    QApplication::processEvents();
    mainTabWidget->resize(mainTabWidget->minimumSizeHint());
    
    QApplication::processEvents();
    resize(minimumSizeHint());
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

void ControlWidget::constructGeneralControls()
{
    // Display

    FocusLineEdit* fpsLineEdit = new FocusLineEdit;
    fpsLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* fpsIntValidator = new QIntValidator(1, 1000, fpsLineEdit);
    fpsIntValidator->setLocale(QLocale::English);
    fpsLineEdit->setValidator(fpsIntValidator);
    fpsLineEdit->setText(QString::number(static_cast<int>(1000.0 / morphoWidget->timer->interval())));

    QFormLayout* fpsFormLayout = new QFormLayout;
    fpsFormLayout->addRow("FPS:", fpsLineEdit);

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
    windowWidthLineEdit->setText(QString::number(morphoWidget->width()));

    windowHeightLineEdit = new FocusLineEdit;
    windowHeightLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* windowHeightIntValidator = new QIntValidator(0, 8192, windowHeightLineEdit);
    windowHeightIntValidator->setLocale(QLocale::English);
    windowHeightLineEdit->setValidator(windowHeightIntValidator);
    windowHeightLineEdit->setText(QString::number(morphoWidget->height()));

    QFormLayout* geometryFormLayout = new QFormLayout;
    geometryFormLayout->addRow("Image width (px):", imageWidthLineEdit);
    geometryFormLayout->addRow("Image height (px):", imageHeightLineEdit);
    geometryFormLayout->addRow("Window width (px):", windowWidthLineEdit);
    geometryFormLayout->addRow("Window height (px):", windowHeightLineEdit);

    applyCircularMaskCheckBox = new QCheckBox("Apply circular mask");
    applyCircularMaskCheckBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    applyCircularMaskCheckBox->setChecked(generator->applyMask);

    QVBoxLayout* displayControlsVBoxLayout = new QVBoxLayout;
    displayControlsVBoxLayout->addLayout(fpsFormLayout);
    displayControlsVBoxLayout->addLayout(geometryFormLayout);
    displayControlsVBoxLayout->addWidget(applyCircularMaskCheckBox);

    QGroupBox* displayControlsGroupBox = new QGroupBox("Display");
    displayControlsGroupBox->setLayout(displayControlsVBoxLayout);

    // Video capture

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
    crfVideoLineEdit->setText(QString::number(generator->crf));

    FocusLineEdit* fpsVideoLineEdit = new FocusLineEdit;
    fpsVideoLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* fpsVideoValidator = new QIntValidator(1, 1000, fpsVideoLineEdit);
    fpsVideoValidator->setLocale(QLocale::English);
    fpsVideoLineEdit->setValidator(fpsVideoValidator);
    fpsVideoLineEdit->setText(QString::number(generator->framesPerSecond));

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

    QGroupBox* videoGroupBox = new QGroupBox("Video recording");
    videoGroupBox->setLayout(videoVBoxLayout);

    // Main layouts

    QHBoxLayout* generalControlsHBoxLayout = new QHBoxLayout;
    generalControlsHBoxLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    generalControlsHBoxLayout->addWidget(displayControlsGroupBox);
    generalControlsHBoxLayout->addWidget(videoGroupBox);

    // Widget to put in tab

    generalControlsWidget = new QWidget;
    generalControlsWidget->setLayout(generalControlsHBoxLayout);

    // Signals + Slots
    
    connect(fpsLineEdit, &FocusLineEdit::returnPressed, [=]() { morphoWidget->timer->setInterval(static_cast<int>(1000.0 / fpsLineEdit->text().toInt())); });
    connect(fpsLineEdit, &FocusLineEdit::focusOut, [=]() { fpsLineEdit->setText(QString::number(static_cast<int>(1000.0 / morphoWidget->timer->interval()))); });

    connect(imageWidthLineEdit, &FocusLineEdit::returnPressed, [=]() { generator->resize(imageWidthLineEdit->text().toInt(), generator->getHeight()); });
    connect(imageWidthLineEdit, &FocusLineEdit::focusOut, [=]() { imageWidthLineEdit->setText(QString::number(generator->getWidth())); });
    connect(imageHeightLineEdit, &FocusLineEdit::returnPressed, [=]() { generator->resize(generator->getWidth(), imageHeightLineEdit->text().toInt()); });
    connect(imageHeightLineEdit, &FocusLineEdit::focusOut, [=]() { imageHeightLineEdit->setText(QString::number(generator->getHeight())); });

    connect(windowWidthLineEdit, &FocusLineEdit::returnPressed, [=]() { morphoWidget->resize(windowWidthLineEdit->text().toInt(), windowHeightLineEdit->text().toInt()); });
    connect(windowWidthLineEdit, &FocusLineEdit::focusOut, [=]() { windowWidthLineEdit->setText(QString::number(morphoWidget->width())); });
    connect(windowHeightLineEdit, &FocusLineEdit::returnPressed, [=]() { morphoWidget->resize(windowWidthLineEdit->text().toInt(), windowHeightLineEdit->text().toInt()); });
    connect(windowHeightLineEdit, &FocusLineEdit::focusOut, [=]() { windowHeightLineEdit->setText(QString::number(morphoWidget->height())); });

    connect(morphoWidget, &MorphoWidget::screenSizeChanged, [=](int width, int height)
        {
            windowWidthLineEdit->setText(QString::number(width));
            windowHeightLineEdit->setText(QString::number(height));
        });

    connect(applyCircularMaskCheckBox, &QCheckBox::clicked, [=](bool checked) { generator->setMask(checked); });
    
    connect(videoFilenamePushButton, &QPushButton::clicked, this, &ControlWidget::setVideoFilename);
    connect(presetsVideoComboBox, &QComboBox::currentTextChanged, [=](QString preset) { generator->preset = preset; });
    connect(crfVideoLineEdit, &FocusLineEdit::returnPressed, [=]() { generator->crf = crfVideoLineEdit->text().toInt(); });
    connect(crfVideoLineEdit, &FocusLineEdit::focusOut, [=]() { crfVideoLineEdit->setText(QString::number(generator->crf)); });
    connect(fpsVideoLineEdit, &FocusLineEdit::returnPressed, [=]() { generator->framesPerSecond = fpsVideoLineEdit->text().toInt(); });
    connect(fpsVideoLineEdit, &FocusLineEdit::focusOut, [=]() { fpsVideoLineEdit->setText(QString::number(generator->framesPerSecond)); });
    connect(morphoWidget, &MorphoWidget::frameRecorded, this, &ControlWidget::setVideoCaptureElapsedTimeLabel);
}

void ControlWidget::setVideoFilename()
{
    QString videoPath = QFileDialog::getSaveFileName(this, "Output video file", "", "Videos (*.mp4 *.mkv *.mov)");

    if (!videoPath.isEmpty())
    {
        generator->recordFilename = videoPath;
        videoCaptureFilenameLabel->setText(videoPath.section('/', -1));
    }
}

void ControlWidget::setVideoCaptureElapsedTimeLabel()
{
    int milliseconds = static_cast<int>(1000.0 * generator->getFrameCount() / generator->framesPerSecond);

    QTime start(0, 0, 0, 0);

    videoCaptureElapsedTimeLabel->setText(start.addMSecs(milliseconds).toString("hh:mm:ss.zzz"));
}

void ControlWidget::constructPipelineControls()
{
    // Seed

    QPushButton* drawRandomSeedPushButton = new QPushButton("Draw random");
    drawRandomSeedPushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    QCheckBox* coloredSeedCheckBox = new QCheckBox("Color");
    coloredSeedCheckBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    coloredSeedCheckBox->setChecked(true);

    QCheckBox* bwSeedCheckBox = new QCheckBox("Grays");
    bwSeedCheckBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    bwSeedCheckBox->setChecked(false);

    QHBoxLayout* randomSeedHBoxLayout = new QHBoxLayout;
    randomSeedHBoxLayout->setAlignment(Qt::AlignLeft);
    randomSeedHBoxLayout->addWidget(drawRandomSeedPushButton);
    randomSeedHBoxLayout->addWidget(coloredSeedCheckBox);
    randomSeedHBoxLayout->addWidget(bwSeedCheckBox);

    QPushButton* drawSeedImagePushButton = new QPushButton("Draw image");
    drawSeedImagePushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    drawSeedImagePushButton->setEnabled(false);

    QPushButton* loadSeedImagePushButton = new QPushButton("Load image");
    loadSeedImagePushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    QHBoxLayout* seedImageHBoxLayout = new QHBoxLayout;
    seedImageHBoxLayout->setAlignment(Qt::AlignLeft);
    seedImageHBoxLayout->addWidget(drawSeedImagePushButton);
    seedImageHBoxLayout->addWidget(loadSeedImagePushButton);

    QVBoxLayout* seedVBoxLayout = new QVBoxLayout;
    seedVBoxLayout->addLayout(randomSeedHBoxLayout);
    seedVBoxLayout->addLayout(seedImageHBoxLayout);

    QGroupBox* seedGroupBox = new QGroupBox("Seed");
    seedGroupBox->setLayout(seedVBoxLayout);

    QButtonGroup* seedButtonGroup = new QButtonGroup(this);
    seedButtonGroup->setExclusive(true);
    seedButtonGroup->addButton(coloredSeedCheckBox, 0);
    seedButtonGroup->addButton(bwSeedCheckBox, 1);

    // Pipelines

    QPushButton* addPipelinePushButton = new QPushButton("Add new");
    addPipelinePushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QPushButton* removePipelinePushButton = new QPushButton("Remove selected");
    removePipelinePushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QHBoxLayout* pipelineButtonsHBoxLayout = new QHBoxLayout;
    pipelineButtonsHBoxLayout->addWidget(addPipelinePushButton);
    pipelineButtonsHBoxLayout->addWidget(removePipelinePushButton);

    pipelinesLabel = new QLabel("Pipelines");
    pipelinesLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    blendFactorsLabel = new QLabel("Blend factors");
    blendFactorsLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    blendFactorsLabel->hide();

    equalizeBlendFactorsPushButton = new QPushButton("Equalize factors");
    equalizeBlendFactorsPushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    equalizeBlendFactorsPushButton->hide();

    outputPipelinePushButton = new QPushButton("Out");
    outputPipelinePushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    outputPipelinePushButton->setFixedWidth(50);
    outputPipelinePushButton->setObjectName("pipelineButton");
    outputPipelinePushButton->setCheckable(true);
    outputPipelinePushButton->setChecked(true);

    pipelinesGridLayout = new QGridLayout;
    pipelinesGridLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    pipelinesGridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    pipelinesGridLayout->addWidget(pipelinesLabel, 0, 0);
    pipelinesGridLayout->addWidget(blendFactorsLabel, 0, 1);
    pipelinesGridLayout->addWidget(outputPipelinePushButton, 1, 0);
    pipelinesGridLayout->addWidget(equalizeBlendFactorsPushButton, 1, 1);

    pipelinesButtonGroup = new QButtonGroup(this);
    pipelinesButtonGroup->setExclusive(true);
    pipelinesButtonGroup->addButton(outputPipelinePushButton);
    pipelinesButtonGroup->setId(outputPipelinePushButton, -2);

    QWidget* pipelinesWidget = new QWidget;
    pipelinesWidget->setLayout(pipelinesGridLayout);

    QScrollArea* pipelineScrollArea = new QScrollArea;
    pipelineScrollArea->setWidget(pipelinesWidget);

    QVBoxLayout* pipelineVBoxLayout = new QVBoxLayout;
    pipelineVBoxLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    pipelineVBoxLayout->addLayout(pipelineButtonsHBoxLayout);
    pipelineVBoxLayout->addWidget(pipelineScrollArea);

    QGroupBox* pipelineGroupBox = new QGroupBox("Pipelines");
    pipelineGroupBox->setLayout(pipelineVBoxLayout);

    // Image operations

    newImageOperationComboBox = new QComboBox;
    newImageOperationComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    QFormLayout* newImageOperationFormLayout = new QFormLayout;
    newImageOperationFormLayout->addRow("New operation:", newImageOperationComboBox);

    insertImageOperationPushButton = new QPushButton("Insert");
    insertImageOperationPushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    removeImageOperationPushButton = new QPushButton("Remove");
    removeImageOperationPushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    QHBoxLayout* insertRemoveHBoxLayout = new QHBoxLayout;
    insertRemoveHBoxLayout->addWidget(insertImageOperationPushButton);
    insertRemoveHBoxLayout->addWidget(removeImageOperationPushButton);

    imageOperationsListWidget = new QListWidget;
    imageOperationsListWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    imageOperationsListWidget->setFixedHeight(100);
    imageOperationsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    imageOperationsListWidget->setDragDropMode(QAbstractItemView::InternalMove);

    // Parameters

    parametersLayout = new QVBoxLayout;

    QGroupBox* parametersGroupBox = new QGroupBox("Parameters");
    parametersGroupBox->setLayout(parametersLayout);

    // Selected real parameter

    selectedParameterSlider = new QSlider(Qt::Horizontal);
    selectedParameterSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    selectedParameterSlider->setRange(0, 10000);

    selectedParameterMinLineEdit = new FocusLineEdit;
    selectedParameterMinLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    selectedParameterMinLineEdit->setPlaceholderText("Minimum");

    selectedParameterMinValidator = new QDoubleValidator(selectedParameterMinLineEdit);
    selectedParameterMinValidator->setDecimals(10);
    selectedParameterMinLineEdit->setValidator(selectedParameterMinValidator);

    selectedParameterMaxLineEdit = new FocusLineEdit;
    selectedParameterMaxLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    selectedParameterMaxLineEdit->setPlaceholderText("Maximum");

    selectedParameterMaxValidator = new QDoubleValidator(selectedParameterMaxLineEdit);
    selectedParameterMaxValidator->setDecimals(10);
    selectedParameterMaxLineEdit->setValidator(selectedParameterMaxValidator);

    QHBoxLayout* minMaxHBoxLayout = new QHBoxLayout;
    minMaxHBoxLayout->setAlignment(Qt::AlignJustify);
    minMaxHBoxLayout->addWidget(selectedParameterMinLineEdit);
    minMaxHBoxLayout->addWidget(selectedParameterMaxLineEdit);

    QVBoxLayout* selectedParameterVBoxLayout = new QVBoxLayout;
    selectedParameterVBoxLayout->addWidget(selectedParameterSlider);
    selectedParameterVBoxLayout->addLayout(minMaxHBoxLayout);

    selectedParameterGroupBox = new QGroupBox("No parameter selected");
    selectedParameterGroupBox->setLayout(selectedParameterVBoxLayout);

    // Image operations layout

    QVBoxLayout* imageOperationsVBoxLayout = new QVBoxLayout;
    imageOperationsVBoxLayout->setAlignment(Qt::AlignTop);
    imageOperationsVBoxLayout->addLayout(newImageOperationFormLayout);
    imageOperationsVBoxLayout->addLayout(insertRemoveHBoxLayout);
    imageOperationsVBoxLayout->addWidget(imageOperationsListWidget);
    imageOperationsVBoxLayout->addWidget(parametersGroupBox);
    imageOperationsVBoxLayout->addWidget(selectedParameterGroupBox);

    QGroupBox* imageOperationsGroupBox = new QGroupBox("Pipeline operations");
    imageOperationsGroupBox->setLayout(imageOperationsVBoxLayout);

    // Main layouts

    QVBoxLayout* vBoxLayout1 = new QVBoxLayout;
    vBoxLayout1->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    vBoxLayout1->addWidget(seedGroupBox);
    vBoxLayout1->addWidget(pipelineGroupBox);

    QVBoxLayout* vBoxLayout2 = new QVBoxLayout;
    vBoxLayout2->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    vBoxLayout2->addWidget(imageOperationsGroupBox);

    QHBoxLayout* imageManipulationHBoxLayout = new QHBoxLayout;
    imageManipulationHBoxLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    imageManipulationHBoxLayout->addLayout(vBoxLayout1);
    imageManipulationHBoxLayout->addLayout(vBoxLayout2);

    pipelineControlsWidget = new QWidget;
    pipelineControlsWidget->setLayout(imageManipulationHBoxLayout);

    // Signals + Slots

    connect(drawRandomSeedPushButton, &QPushButton::clicked, [=]() { generator->drawRandomSeed(bwSeedCheckBox->isChecked()); });
    connect(drawSeedImagePushButton, &QPushButton::clicked, [=]() { generator->drawSeedImage(); });
    connect(loadSeedImagePushButton, &QPushButton::clicked, [=]()
        {
            QString filename = QFileDialog::getOpenFileName(this, "Load image", "", "Images (*.bmp *.cur *.gif *.icns *.ico *.jpeg *.jpg *.pbm *.pgm *.png *.ppm *.tga *.tif *.tiff *.wbmp *.webp *.xbm *.xpm)");
            if (!filename.isEmpty())
            {
                generator->loadSeedImage(filename);
                drawSeedImagePushButton->setEnabled(true);
            }                
        });
    connect(outputPipelinePushButton, &QPushButton::clicked, [=](bool checked) { if (checked) initImageOperationsListWidget(pipelinesButtonGroup->checkedId()); });
    connect(addPipelinePushButton, &QPushButton::clicked, [=]()
        {
            generator->addPipeline();
            initPipelineControls(generator->getPipelinesSize() - 1);
        });
    connect(removePipelinePushButton, &QPushButton::clicked, [=]()
        {
            generator->removePipeline(pipelinesButtonGroup->checkedId());
            initPipelineControls(pipelinesButtonGroup->checkedId());
        });
    connect(equalizeBlendFactorsPushButton, &QPushButton::clicked, [=]()
        {
            generator->equalizePipelineBlendFactors();
            for (int i = 0; i < generator->getPipelinesSize(); i++)
                pipelineBlendFactorLineEdit[i]->setText(QString::number(generator->getPipelineBlendFactor(i)));
        });
    connect(imageOperationsListWidget, &QListWidget::currentRowChanged, this, &ControlWidget::onImageOperationsListWidgetCurrentRowChanged);
    connect(imageOperationsListWidget->model(), &QAbstractItemModel::rowsMoved, this, &ControlWidget::onRowsMoved);
    connect(insertImageOperationPushButton, &QPushButton::clicked, this, &ControlWidget::insertImageOperation);
    connect(removeImageOperationPushButton, &QPushButton::clicked, this, &ControlWidget::removeImageOperation);

    // Init

    initNewImageOperationComboBox();
}

void ControlWidget::initPipelineControls(int selectedPipelineIndex)
{
    // Remove all pipeline controls

    for (int pipelineIndex = pipelinePushButton.size() - 1; pipelineIndex >= 0; pipelineIndex--)
    {
        pipelinesButtonGroup->removeButton(pipelinePushButton[pipelineIndex]);
        pipelinesGridLayout->removeWidget(pipelinePushButton[pipelineIndex]);
        pipelinesGridLayout->removeWidget(pipelineBlendFactorLineEdit[pipelineIndex]);

        delete pipelinePushButton[pipelineIndex];
        delete pipelineBlendFactorLineEdit[pipelineIndex];

        pipelinePushButton.erase(pipelinePushButton.begin() + pipelineIndex);
        pipelineBlendFactorLineEdit.erase(pipelineBlendFactorLineEdit.begin() + pipelineIndex);
    }

    // Keep index within range

    if (selectedPipelineIndex < -2)
        selectedPipelineIndex = -2;

    if (selectedPipelineIndex > generator->getPipelinesSize() - 1)
        selectedPipelineIndex = generator->getPipelinesSize() - 1;

    // Add all pipeline controls

    for (int pipelineIndex = 0; pipelineIndex < generator->getPipelinesSize(); pipelineIndex++)
    {
        // New pipeline select button

        QPushButton* pushButton = new QPushButton(QString("%1").arg(pipelineIndex + 1));
        pushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        pushButton->setFixedWidth(50);
        pushButton->setObjectName("pipelineButton");
        pushButton->setCheckable(true);
        pushButton->setChecked(pipelineIndex == selectedPipelineIndex);

        pipelinesButtonGroup->addButton(pushButton);
        pipelinesButtonGroup->setId(pushButton, pipelineIndex);

        connect(pushButton, &QPushButton::clicked, [=](bool checked)
            {
                if (checked)
                    initImageOperationsListWidget(pipelinesButtonGroup->checkedId());
            });

        pipelinePushButton.push_back(pushButton);

        // New pipeline blend factor line edit

        FocusLineEdit* blendFactorLineEdit = new FocusLineEdit;
        blendFactorLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        QDoubleValidator* blendFactorValidator = new QDoubleValidator(0.0, 1.0, 10, blendFactorLineEdit);
        blendFactorValidator->setLocale(QLocale::English);
        blendFactorLineEdit->setValidator(blendFactorValidator);
        blendFactorLineEdit->setText(QString::number(generator->getPipelineBlendFactor(pipelineIndex)));

        connect(blendFactorLineEdit, &FocusLineEdit::returnPressed, [=]() { setPipelineBlendFactorLineEditText(pipelineIndex); });
        connect(blendFactorLineEdit, &FocusLineEdit::focusOut, [=]() { blendFactorLineEdit->setText(QString::number(generator->getPipelineBlendFactor(pipelineIndex))); });

        pipelineBlendFactorLineEdit.push_back(blendFactorLineEdit);

        // Add controls to pipelines layout

        pipelinesGridLayout->addWidget(pushButton, pipelineIndex + 1, 0);
        pipelinesGridLayout->addWidget(blendFactorLineEdit, pipelineIndex + 1, 1);
    }

    pipelinesGridLayout->removeWidget(equalizeBlendFactorsPushButton);
    pipelinesGridLayout->removeWidget(outputPipelinePushButton);
    pipelinesGridLayout->addWidget(outputPipelinePushButton, generator->getPipelinesSize() + 1, 0);
    pipelinesGridLayout->addWidget(equalizeBlendFactorsPushButton, generator->getPipelinesSize() + 1, 1);

    // Set visibility of labels and equalize blend factors button

    blendFactorsLabel->setVisible(generator->getPipelinesSize() > 0);
    equalizeBlendFactorsPushButton->setVisible(generator->getPipelinesSize() > 1);

    // Check output pipeline button

    if (selectedPipelineIndex == -2 || generator->getPipelinesSize() == 0)
        outputPipelinePushButton->setChecked(true);
    else
        outputPipelinePushButton->setChecked(false);

    // Init operations

    if (generator->getPipelinesSize() > 0)
        initImageOperationsListWidget(selectedPipelineIndex);
    else
        initImageOperationsListWidget(0);

    resizeMainTabs(mainTabWidget->currentIndex());
}

void ControlWidget::setPipelineBlendFactorLineEditText(int pipelineIndex)
{
    generator->setPipelineBlendFactor(pipelineIndex, pipelineBlendFactorLineEdit[pipelineIndex]->text().toDouble());

    for (int i = 0; i < generator->getPipelinesSize(); i++)
        pipelineBlendFactorLineEdit[i]->setText(QString::number(generator->getPipelineBlendFactor(i)));

}

void ControlWidget::initNewImageOperationComboBox()
{
    for (auto operationName : generator->availableImageOperations)
        newImageOperationComboBox->addItem(operationName);
}

void ControlWidget::initImageOperationsListWidget(int pipelineIndex)
{
    imageOperationsListWidget->clear();

    for (int i = 0; i < generator->getImageOperationsSize(pipelineIndex); i++)
    {
        QListWidgetItem* newOperation = new QListWidgetItem;
        newOperation->setText(generator->getImageOperationName(pipelineIndex, i));
        imageOperationsListWidget->insertItem(i, newOperation);
    }

    if (generator->getImageOperationsSize(pipelineIndex) == 0)
    {
        currentImageOperationIndex[pipelineIndex] = -1;

        if (!parametersLayout->isEmpty())
        {
            QWidget* widget = parametersLayout->itemAt(0)->widget();
            parametersLayout->removeWidget(widget);
            widget->hide();
        }

        if (operationsWidget)
        {
            delete operationsWidget;
            operationsWidget = nullptr;
        }
    }
    else
    {
        QListWidgetItem* operation = imageOperationsListWidget->item(currentImageOperationIndex[pipelineIndex]);
        imageOperationsListWidget->setCurrentItem(operation);

        int size = imageOperationsListWidget->sizeHintForRow(0);
        if (size > 0) rowSize = size;

        imageOperationsListWidget->setFixedHeight(rowSize * 5 + 2 * imageOperationsListWidget->frameWidth());
    }

    resizeMainTabs(mainTabWidget->currentIndex());
}

void ControlWidget::onImageOperationsListWidgetCurrentRowChanged(int currentRow)
{
    if (!parametersLayout->isEmpty())
    {
        QWidget* widget = parametersLayout->itemAt(0)->widget();
        parametersLayout->removeWidget(widget);
        widget->hide();
    }

    selectedParameterSlider->disconnect();
    selectedParameterSlider->setValue(0);
    selectedParameterGroupBox->setTitle("No parameter selected");

    selectedParameterMinLineEdit->disconnect();
    selectedParameterMinLineEdit->clear();

    selectedParameterMaxLineEdit->disconnect();
    selectedParameterMaxLineEdit->clear();

    if (currentRow >= 0) // currentRow = -1 if QListWidget empty
    {
        int pipelineIndex = pipelinesButtonGroup->checkedId();

        if (operationsWidget)
        {
            delete operationsWidget;
            operationsWidget = nullptr;
        }

        operationsWidget = new OperationsWidget(pipelineIndex >= 0 ? generator->pipelines[pipelineIndex]->getImageOperation(currentRow) : generator->outputPipeline->getImageOperation(currentRow));
        parametersLayout->addWidget(operationsWidget);
        operationsWidget->show();

        for (auto widget : operationsWidget->floatParameterWidget)
            connect(widget, &FloatParameterWidget::focusIn, [=]() { onFloatParameterWidgetFocusIn(widget); });

        currentImageOperationIndex[pipelineIndex] = currentRow;
    }

    resizeMainTabs(mainTabWidget->currentIndex());
}

void ControlWidget::onFloatParameterWidgetFocusIn(FloatParameterWidget* widget)
{
    // Slider

    selectedParameterSlider->disconnect();
    selectedParameterSlider->setRange(0, widget->indexMax);
    selectedParameterSlider->setValue(widget->getIndex());

    connect(selectedParameterSlider, &QAbstractSlider::valueChanged, widget, &FloatParameterWidget::setValue);

    connect(widget, &FloatParameterWidget::currentIndexChanged, [=](int currentIndex)
        {
            disconnect(selectedParameterSlider, &QAbstractSlider::valueChanged, nullptr, nullptr);
            selectedParameterSlider->setValue(currentIndex);
            connect(selectedParameterSlider, &QAbstractSlider::valueChanged, widget, &FloatParameterWidget::setValue);
        });

    // Value changed: check if within min/max range and adjust controls

    connect(widget, &FloatParameterWidget::currentValueChanged, [=](double currentValue)
        {
            if (currentValue < widget->getMin())
            {
                widget->setMin(currentValue);

                selectedParameterMinLineEdit->setText(QString::number(currentValue));

                disconnect(selectedParameterSlider, &QAbstractSlider::valueChanged, nullptr, nullptr);
                selectedParameterSlider->setValue(widget->getIndex());
                connect(selectedParameterSlider, &QAbstractSlider::valueChanged, widget, &FloatParameterWidget::setValue);
            }
            else if (currentValue > widget->getMax())
            {
                widget->setMax(currentValue);

                selectedParameterMaxLineEdit->setText(QString::number(currentValue));

                disconnect(selectedParameterSlider, &QAbstractSlider::valueChanged, nullptr, nullptr);
                selectedParameterSlider->setValue(widget->getIndex());
                connect(selectedParameterSlider, &QAbstractSlider::valueChanged, widget, &FloatParameterWidget::setValue);
            }
        });

    // Minimum

    selectedParameterMinLineEdit->disconnect();
    selectedParameterMinLineEdit->setText(QString::number(widget->getMin()));

    connect(selectedParameterMinLineEdit, &FocusLineEdit::returnPressed, [=]()
        {
            widget->setMin(selectedParameterMinLineEdit->text().toDouble());
            widget->setIndex();
        });
    connect(selectedParameterMinLineEdit, &FocusLineEdit::focusOut, [=]() { selectedParameterMinLineEdit->setText(QString::number(widget->getMin())); });

    // Maximum

    selectedParameterMaxLineEdit->disconnect();
    selectedParameterMaxLineEdit->setText(QString::number(widget->getMax()));

    connect(selectedParameterMaxLineEdit, &FocusLineEdit::returnPressed, [=]()
        {
            widget->setMax(selectedParameterMaxLineEdit->text().toDouble());
            widget->setIndex();
        });
    connect(selectedParameterMaxLineEdit, &FocusLineEdit::focusOut, [=]() { selectedParameterMaxLineEdit->setText(QString::number(widget->getMax())); });

    // Validators

    selectedParameterMinValidator->setBottom(widget->getInf());
    selectedParameterMinValidator->setTop(widget->getMax());

    selectedParameterMaxValidator->setBottom(widget->getMin());
    selectedParameterMaxValidator->setTop(widget->getSup());

    // Title

    selectedParameterGroupBox->setTitle("Selected parameter: " + widget->getName());
}

void ControlWidget::onRowsMoved(QModelIndex parent, int start, int end, QModelIndex destination, int row)
{
    Q_UNUSED(parent)
    Q_UNUSED(destination)
    Q_UNUSED(end)
    
    int pipelineIndex = pipelinesButtonGroup->checkedId();

    if (start < row) row--;

    generator->swapImageOperations(pipelineIndex, start, row);

    currentImageOperationIndex[pipelineIndex] = row;
}

void ControlWidget::insertImageOperation()
{
    int pipelineIndex = pipelinesButtonGroup->checkedId();
    int newOperationIndex = newImageOperationComboBox->currentIndex();
    int currentOperationIndex = imageOperationsListWidget->currentRow();

    generator->insertImageOperation(pipelineIndex, newOperationIndex, currentOperationIndex + 1);

    QListWidgetItem* newOperation = new QListWidgetItem;
    newOperation->setText(generator->getImageOperationName(pipelineIndex, currentOperationIndex + 1));
    imageOperationsListWidget->insertItem(currentOperationIndex + 1, newOperation);

    int size = imageOperationsListWidget->sizeHintForRow(0);
    if (size > 0) rowSize = size;

    imageOperationsListWidget->setFixedHeight(rowSize * 5 + 2 * imageOperationsListWidget->frameWidth());

    imageOperationsListWidget->setCurrentItem(newOperation);
}

void ControlWidget::removeImageOperation()
{
    int pipelineIndex = pipelinesButtonGroup->checkedId();
    int operationIndex = imageOperationsListWidget->currentRow();

    imageOperationsListWidget->takeItem(operationIndex);

    int size = imageOperationsListWidget->sizeHintForRow(0);
    if (size > 0) rowSize = size;

    imageOperationsListWidget->setFixedHeight(rowSize * 5 + 2 * imageOperationsListWidget->frameWidth());

    generator->removeImageOperation(pipelineIndex, operationIndex);

    currentImageOperationIndex[pipelineIndex] = imageOperationsListWidget->currentRow();

    resizeMainTabs(mainTabWidget->currentIndex());
}
