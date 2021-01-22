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
#include "controlwidget.h"

ControlWidget::ControlWidget(Heart* theHeart, QWidget *parent) : QWidget(parent), heart { theHeart }
{
    // Generator

    generator = new GeneratorGL();

    // Actions toolbar

    QToolBar* toolbar = new QToolBar;
    iterateAction = toolbar->addAction(QIcon(QPixmap(":/icons/media-playback-start.png")), "Start/pause feedback loop");
    iterateAction->setCheckable(true);
    recordAction = toolbar->addAction(QIcon(QPixmap(":/icons/media-record.png")), "Record video");
    recordAction->setCheckable(true);
    screenshotAction = toolbar->addAction(QIcon(QPixmap(":/icons/digikam.png")), "Take screenshot");
    toolbar->addSeparator();
    displayOptionsAction = toolbar->addAction(QIcon(QPixmap(":/icons/video-display.png")), "Display options");
    recordingOptionsAction = toolbar->addAction(QIcon(QPixmap(":/icons/folder-video.png")), "Recording options");
    toolbar->addSeparator();
    rgbAction = toolbar->addAction(QIcon(QPixmap(":/icons/office-chart-scatter.png")), "Show RGB graph");
    rgbAction->setCheckable(true);
    toolbar->addSeparator();
    loadConfigAction = toolbar->addAction(QIcon(QPixmap(":/icons/document-open.png")), "Load configuration");
    saveConfigAction = toolbar->addAction(QIcon(QPixmap(":/icons/document-save.png")), "Save configuration");
    toolbar->addSeparator();
    QAction* aboutAction = toolbar->addAction(QIcon(QPixmap(":/icons/help-about.png")), "About");

    connect(iterateAction, &QAction::triggered, this, &ControlWidget::iterate);
    connect(recordAction, &QAction::triggered, this, &ControlWidget::record);
    connect(screenshotAction, &QAction::triggered, this, &ControlWidget::takeScreenshot);
    connect(displayOptionsAction, &QAction::triggered, this, &ControlWidget::toggleDisplayOptionsWidget);
    connect(recordingOptionsAction, &QAction::triggered, this, &ControlWidget::toggleRecordingOptionsWidget);
    connect(rgbAction, &QAction::triggered, this, &ControlWidget::toggleRGBGraph);
    connect(loadConfigAction, &QAction::triggered, this, &ControlWidget::loadConfig);
    connect(saveConfigAction, &QAction::triggered, this, &ControlWidget::saveConfig);
    connect(aboutAction, &QAction::triggered, this, &ControlWidget::about);

    // Contruct controls

    constructDisplayOptionsWidget();
    constructRecordingOptionsWidget();

    // Graph widget

    graphWidget = new GraphWidget(generator);

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

    QVBoxLayout* mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addWidget(toolbar);
    mainVBoxLayout->addWidget(graphWidget);
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
        emit imageSizeChanged();
    });
}

ControlWidget::~ControlWidget()
{
    delete displayOptionsWidget;
    delete recordingOptionsWidget;
    delete parser;
    delete graphWidget;
    delete generator;
    qDeleteAll(operationsWidgets);
}

void ControlWidget::closeEvent(QCloseEvent* event)
{
    displayOptionsWidget->close();
    recordingOptionsWidget->close();

    foreach(OperationsWidget* opWidget, operationsWidgets)
        opWidget->close();

    emit closing();
    event->accept();
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

void ControlWidget::toggleRGBGraph()
{
    heart->setRGBWidgetVisibility(rgbAction->isChecked());
}

void ControlWidget::uncheckRGBGraphAction()
{
    rgbAction->setChecked(false);
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

void ControlWidget::constructDisplayOptionsWidget()
{
    // Display

    FocusLineEdit* fpsLineEdit = new FocusLineEdit;
    fpsLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* fpsIntValidator = new QIntValidator(1, 1000, fpsLineEdit);
    fpsIntValidator->setLocale(QLocale::English);
    fpsLineEdit->setValidator(fpsIntValidator);
    fpsLineEdit->setText(QString::number(static_cast<int>(1000.0 / heart->getTimerInterval())));

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
    windowWidthLineEdit->setText(QString::number(heart->getMorphoWidgetWidth()));

    windowHeightLineEdit = new FocusLineEdit;
    windowHeightLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QIntValidator* windowHeightIntValidator = new QIntValidator(0, 8192, windowHeightLineEdit);
    windowHeightIntValidator->setLocale(QLocale::English);
    windowHeightLineEdit->setValidator(windowHeightIntValidator);
    windowHeightLineEdit->setText(QString::number(heart->getMorphoWidgetHeight()));

    QFormLayout* geometryFormLayout = new QFormLayout;
    geometryFormLayout->addRow("Image width (px):", imageWidthLineEdit);
    geometryFormLayout->addRow("Image height (px):", imageHeightLineEdit);
    geometryFormLayout->addRow("Window width (px):", windowWidthLineEdit);
    geometryFormLayout->addRow("Window height (px):", windowHeightLineEdit);

    QVBoxLayout* displayControlsVBoxLayout = new QVBoxLayout;
    displayControlsVBoxLayout->addLayout(fpsFormLayout);
    displayControlsVBoxLayout->addLayout(geometryFormLayout);

    displayOptionsWidget = new QWidget;
    displayOptionsWidget->setLayout(displayControlsVBoxLayout);

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
        emit imageSizeChanged();
    });
    connect(imageWidthLineEdit, &FocusLineEdit::focusOut, [&generator = this->generator, &imageWidthLineEdit = this->imageWidthLineEdit]()
    {
        imageWidthLineEdit->setText(QString::number(generator->getWidth()));
    });
    connect(imageHeightLineEdit, &FocusLineEdit::returnPressed, [=]()
    {
        generator->resize(generator->getWidth(), imageHeightLineEdit->text().toInt());
        emit imageSizeChanged();
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
        operationsWidgets.value(id)->recreate(generator->getOperation(id));
    }
}
