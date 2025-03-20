


#include "operationwidget.h"
#include "parameters/uniformmat4parameter.h"



OperationWidget::OperationWidget(ImageOperation* operation, bool midiEnabled, bool editMode, QWidget* parent) :
    QFrame(parent),
    mOperation { operation },
    mMidiEnabled { midiEnabled }
{
    mOpBuilder = new OperationBuilder(mOperation);
    mOpBuilder->installEventFilter(this);
    mOpBuilder->setVisible(false);

    connect(mOpBuilder, &OperationBuilder::shadersParsed, this, &OperationWidget::recreate);

    mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header widget

    headerWidget = new QWidget;
    headerWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    headerWidget->setStyleSheet("QWidget { background-color: rgb(16, 64, 128); }");

    // Header toolbar

    headerToolBar = new QToolBar;
    headerToolBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    // Enable action

    enableAction = headerToolBar->addAction(mOperation->isEnabled() ? QIcon(QPixmap(":/icons/circle-green.png")) : QIcon(QPixmap(":/icons/circle-grey.png")), mOperation->isEnabled() ? "Enabled" : "Disabled", this, &OperationWidget::enableOperation);
    enableAction->setCheckable(true);
    enableAction->setChecked(mOperation->isEnabled());

    // Output action

    outputAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/eye.png")), "Set as output", this, &OperationWidget::outputChanged);
    outputAction->setCheckable(true);

    // Edit action

    editAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/applications-development.png")), "Edit", this, &OperationWidget::toggleEditMode);
    editAction->setCheckable(true);
    editAction->setChecked(editMode);

    // Toggle body action

    toggleBodyAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/go-down.png")), "Hide", this, &OperationWidget::toggleBody);
    toggleBodyAction->setCheckable(true);
    toggleBodyAction->setChecked(true);

    // Operation name label

    opNameLabel = new QLabel;
    opNameLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    opNameLabel->setStyleSheet("QLabel { font-size: 16pt; }");

    // Operation name line edit

    opNameLineEdit = new QLineEdit;
    opNameLineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    opNameLineEdit->setStyleSheet("QLineEdit { font-size: 16pt; }");
    opNameLineEdit->setVisible(false);

    connect(opNameLineEdit, &QLineEdit::textEdited, this, [=, this](QString name){
        operation->setName(name);
        opNameLabel->setText(name);

        opNameLineEdit->setMinimumWidth(20 + opNameLineEdit->fontMetrics().horizontalAdvance(name));

        adjustSize();
    });

    QHBoxLayout* headerLayout = new QHBoxLayout;
    headerLayout->addWidget(headerToolBar, 0);
    headerLayout->addWidget(opNameLabel, 1, Qt::AlignLeft);
    headerLayout->addWidget(opNameLineEdit, 1, Qt::AlignLeft);

    headerWidget->setLayout(headerLayout);

    mainLayout->addWidget(headerWidget, 0);

    // Separator

    separator[0] = new QFrame;
    separator[0]->setFrameShape(QFrame::HLine);

    mainLayout->addWidget(separator[0], 0, Qt::AlignTop);

    // Grid widget containing parameter widgets

    gridWidget = new GridWidget;
    gridWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    connect(gridWidget, &GridWidget::itemRowColChanged, this, &OperationWidget::updateWidgetRowCol);

    mainLayout->addWidget(gridWidget, 1, Qt::AlignTop | Qt::AlignLeft);

    // Separator

    separator[1] = new QFrame;
    separator[1]->setFrameShape(QFrame::HLine);

    mainLayout->addWidget(separator[1], 0, Qt::AlignTop);

    // Selected parameter widgets

    paramNameLabel = new QLabel;
    paramNameLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    paramNameLineEdit = new QLineEdit;
    paramNameLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    paramNameLineEdit->setPlaceholderText("Parameter name");
    paramNameLineEdit->setVisible(false);

    selParamSlider = new QSlider(Qt::Horizontal);
    selParamSlider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    selParamSlider->setRange(0, 100'000);

    midiLinkButton = new QPushButton;
    midiLinkButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    midiLinkButton->setCheckable(true);
    midiLinkButton->setStyleSheet("QPushButton { image: url(:/icons/circle-grey.png); background-color: transparent; border: 0; }");

    selParamMinLineEdit = new FocusLineEdit;
    selParamMinLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    selParamMinLineEdit->setPlaceholderText("Minimum");

    selParamMaxLineEdit = new FocusLineEdit;
    selParamMaxLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    selParamMaxLineEdit->setPlaceholderText("Maximum");

    selParamInfLineEdit = new FocusLineEdit;
    selParamInfLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    selParamInfLineEdit->setPlaceholderText("Lowest");
    selParamInfLineEdit->setVisible(false);

    selParamSupLineEdit = new FocusLineEdit;
    selParamSupLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    selParamSupLineEdit->setPlaceholderText("Highest");
    selParamSupLineEdit->setVisible(false);

    layoutComboBox = new QComboBox;
    layoutComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    layoutComboBox->setVisible(false);

    mat4TypeComboBox = new QComboBox;
    mat4TypeComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mat4TypeComboBox->setVisible(false);
    mat4TypeComboBox->addItems(QList<QString>{ "Translation", "Rotation", "Scaling", "Orthographic", "Mat4" });

    // Presets widgets

    removePresetButton = new QPushButton;
    removePresetButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    removePresetButton->setFixedSize(32, 32);
    removePresetButton->setStyleSheet("QPushButton { image: url(:/icons/list-remove.png); background-color: transparent; border: 0; }");

    addPresetButton = new QPushButton;
    addPresetButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    addPresetButton->setFixedSize(32, 32);
    addPresetButton->setStyleSheet("QPushButton { image: url(:/icons/list-add.png); background-color: transparent; border: 0; }");

    presetNameLineEdit = new QLineEdit;
    presetNameLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    presetNameLineEdit->setMaximumWidth(100);
    presetNameLineEdit->setPlaceholderText("Preset name");

    QHBoxLayout* presetsLayout = new QHBoxLayout;
    presetsLayout->addWidget(removePresetButton);
    presetsLayout->addWidget(addPresetButton);
    presetsLayout->addWidget(presetNameLineEdit);

    presetsGroupBox = new QGroupBox("Presets");
    presetsGroupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    presetsGroupBox->setStyleSheet("QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; font-size: 18pt; margin: 7px; }");
    presetsGroupBox->setLayout(presetsLayout);
    presetsGroupBox->setVisible(false);

    // Selected parameter layout

    QVBoxLayout* selParamLayout = new QVBoxLayout;

    selParamLayout->addWidget(paramNameLabel);
    selParamLayout->addWidget(paramNameLineEdit);

    QHBoxLayout* sliderLayout = new QHBoxLayout;
    sliderLayout->addWidget(midiLinkButton, 0);
    sliderLayout->addWidget(selParamSlider, 1);

    selParamLayout->addLayout(sliderLayout);

    QHBoxLayout* minMaxLayout = new QHBoxLayout;
    minMaxLayout->addWidget(selParamMinLineEdit);
    minMaxLayout->addWidget(selParamMaxLineEdit);

    selParamLayout->addLayout(minMaxLayout);

    QHBoxLayout* infSupLayout = new QHBoxLayout;
    infSupLayout->addWidget(selParamInfLineEdit);
    infSupLayout->addWidget(selParamSupLineEdit);

    selParamLayout->addLayout(infSupLayout);

    QHBoxLayout* combosLayout = new QHBoxLayout;
    combosLayout->addWidget(layoutComboBox);
    combosLayout->addWidget(mat4TypeComboBox);

    selParamLayout->addLayout(combosLayout);

    selParamLayout->addWidget(presetsGroupBox);

    // Selected parameter widget

    selParamWidget = new QWidget;
    selParamWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    selParamWidget->setLayout(selParamLayout);

    mainLayout->addWidget(selParamWidget, 0, Qt::AlignTop);

    setLayout(mainLayout);

    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Raised);
    setMidLineWidth(3);
    setLineWidth(3);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    toggleEditMode(editMode);
}



OperationWidget::~OperationWidget()
{
    delete mOpBuilder;
}



void OperationWidget::setup()
{
    lastFocusedWidget = nullptr;

    // Parameter widgets

    foreach (auto parameter, mOperation->uniformParameters<float>())
    {
        UniformParameterWidget<float>* widget = new UniformParameterWidget<float>(parameter);
        gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());

        widget->setCheckable(mEditMode);

        if (!mEditMode && !parameter->editable())
            widget->toggleVisibility(false);

        floatParamWidgets.append(widget);
        uniformFloatParamWidgets.append(widget);

        setFocusedWidget(widget);
    }

    foreach (auto parameter, mOperation->uniformParameters<int>())
    {
        UniformParameterWidget<int>* widget = new UniformParameterWidget<int>(parameter);
        gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());

        widget->setCheckable(mEditMode);

        if (!mEditMode && !parameter->editable())
            widget->toggleVisibility(false);

        intParamWidgets.append(widget);
        uniformIntParamWidgets.append(widget);

        setFocusedWidget(widget);
    }

    foreach (auto parameter, mOperation->uniformParameters<unsigned int>())
    {
        UniformParameterWidget<unsigned int>* widget = new UniformParameterWidget<unsigned int>(parameter);
        gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());

        widget->setCheckable(mEditMode);

        if (!mEditMode && !parameter->editable())
            widget->toggleVisibility(false);

        uintParamWidgets.append(widget);
        uniformUintParamWidgets.append(widget);

        setFocusedWidget(widget);
    }

    foreach (auto parameter, mOperation->mat4UniformParameters())
    {
        UniformMat4ParameterWidget* widget = new UniformMat4ParameterWidget(parameter);
        gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());

        widget->setCheckable(mEditMode);

        if (!mEditMode && !parameter->editable())
            widget->toggleVisibility(false);

        mat4ParamWidgets.append(widget);
        uniformMat4ParamWidgets.append(widget);

        setFocusedWidget(widget);
    }

    foreach (auto parameter, mOperation->optionsParameters<GLenum>())
    {
        OptionsParameterWidget<GLenum>* widget = new OptionsParameterWidget<GLenum>(parameter);
        gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());

        widget->setCheckable(mEditMode);

        if (!mEditMode && !parameter->editable())
            widget->toggleVisibility(false);

        glenumOptionsWidgets.append(widget);
    }

    // Selected parameter widget

    if (!floatParamWidgets.empty() || !intParamWidgets.empty() || !uintParamWidgets.empty() || !mat4ParamWidgets.empty())
    {
        connectParamWidgets<float>(floatParamWidgets);
        connectParamWidgets<int>(intParamWidgets);
        connectParamWidgets<unsigned int>(uintParamWidgets);
        connectParamWidgets<float>(mat4ParamWidgets);

        connectUniformParamWidgets<float>(uniformFloatParamWidgets);
        connectUniformParamWidgets<int>(uniformIntParamWidgets);
        connectUniformParamWidgets<unsigned int>(uniformUintParamWidgets);

        connectUniformFloatParamWidgets();

        connectUniformMat4ParamWidgets();

        if (lastFocusedWidget)
            lastFocusedWidget->setFocus();
    }

    // Operation name controls

    opNameLabel->setText(mOperation->name());

    opNameLineEdit->setText(mOperation->name());
    opNameLineEdit->setFixedWidth(20 + opNameLineEdit->fontMetrics().horizontalAdvance(mOperation->name()));

    // Midi link button

    midiLinkButton->setVisible(mMidiEnabled);

    // Once widgets set on grid, optimize its layout to set it with proper row and column spans and sizes

    if (gridWidget->isVisible())
        gridWidget->optimizeLayout();

    adjustSize();
}



void OperationWidget::recreate()
{
    minValidator = nullptr;
    maxValidator = nullptr;

    infValidator = nullptr;
    supValidator = nullptr;

    gridWidget->clear();

    qDeleteAll(floatParamWidgets);
    floatParamWidgets.clear();
    uniformFloatParamWidgets.clear();

    qDeleteAll(intParamWidgets);
    intParamWidgets.clear();
    uniformIntParamWidgets.clear();

    qDeleteAll(uintParamWidgets);
    uintParamWidgets.clear();
    uniformUintParamWidgets.clear();

    qDeleteAll(mat4ParamWidgets);
    mat4ParamWidgets.clear();
    uniformMat4ParamWidgets.clear();

    qDeleteAll(glenumOptionsWidgets);
    glenumOptionsWidgets.clear();

    setup();
}



template <typename T>
void OperationWidget::setFocusedWidget(ParameterWidget<T>* widget)
{
    if (widget->parameter() == lastFocusedParameter)
        lastFocusedWidget = widget->lastFocusedWidget();
}



template <typename T>
void OperationWidget::connectParamWidgets(QList<ParameterWidget<T>*> widgets)
{
    foreach (auto widget, widgets)
    {
        connect(widget, &ParameterWidget<T>::focusIn, this, [=, this](){
            lastFocusedParameter = widget->parameter();
            lastFocusedWidget = widget->lastFocusedWidget();
        });
    }
}



template <typename T>
void OperationWidget::connectUniformParamWidgets(QList<UniformParameterWidget<T>*> widgets)
{
    foreach (auto widget, widgets)
    {
        connect(widget, &UniformParameterWidget<T>::focusIn, this, [=, this](){
            if (mEditMode)
                updateSelParamEditControls<T>(widget);
            else
                updateSelParamControls<T>(widget);

            updateMidiButton<T>(widget);

            setSelParamNameWidgets<T>(widget);
        });

        connect(widget, &UniformParameterWidget<T>::focusIn, this, [=, this](){
            if (mEditMode)
            {
                // Set up layout controller combo box

                layoutComboBox->disconnect();
                layoutComboBox->clear();

                if (widget->availableLayoutFormats().size() == 0)
                    layoutComboBox->setVisible(false);
                else
                {
                    layoutComboBox->setVisible(true);

                    layoutComboBox->addItems(widget->availableLayoutFormats());
                    layoutComboBox->setCurrentIndex(widget->layoutFormatIndex());

                    connect(layoutComboBox, &QComboBox::currentIndexChanged, this, [=, this](int index){
                        widget->setLayoutFormatIndex(index);
                        gridWidget->optimizeLayout();
                        adjustSize();
                    });
                }
            }
        });
    }
}



void OperationWidget::connectUniformFloatParamWidgets()
{
    foreach (auto widget, uniformFloatParamWidgets)
    {
        connect(widget, &UniformParameterWidget<float>::focusIn, this, [=, this](){
            if (mEditMode)
            {
                // Set up Mat4 parameter switcher combo box

                mat4TypeComboBox->disconnect();

                if (widget->isMat4Equivalent())
                {
                    mat4TypeComboBox->setVisible(true);
                    mat4TypeComboBox->setCurrentIndex(4);

                    connect(mat4TypeComboBox, &QComboBox::currentIndexChanged, this, [=, this](int index){
                        if (index >= 0 && index < 4)
                        {
                            UniformParameter<float>* parameter = widget->parameter();

                            UniformMat4Parameter* newParam = new UniformMat4Parameter(*parameter, static_cast<UniformMat4Type>(index));
                            mOpBuilder->addMat4UniformParameter(newParam);
                            mOpBuilder->removeUniformFloatParameter(parameter);

                            lastFocusedParameter = newParam;

                            if (index < 3)
                                addInterpolation();

                            recreate();
                        }
                    });
                }
                else
                    mat4TypeComboBox->setVisible(false);
            }
        });
    }
}



void OperationWidget::connectUniformMat4ParamWidgets()
{
    foreach (auto widget, uniformMat4ParamWidgets)
    {
        connect(widget, &UniformMat4ParameterWidget::focusIn, this, [=, this](){
            if (!widget->parameter()->empty())
            {
                if (mEditMode)
                    updateSelParamEditControls<float>(widget);
                else
                    updateSelParamControls<float>(widget);

                updateMidiButton<float>(widget);
            }
            else
                toggleSelParamWidgets(false);

            setSelParamNameWidgets<float>(widget);
        });

        connect(widget, &UniformMat4ParameterWidget::focusIn, this, [=, this](){
            if (mEditMode)
            {
                // Set up Mat4 parameter switcher combo box

                mat4TypeComboBox->disconnect();

                mat4TypeComboBox->setVisible(true);
                mat4TypeComboBox->setCurrentIndex(widget->typeIndex());

                connect(mat4TypeComboBox, &QComboBox::currentIndexChanged, this, [=, this](int index){
                    if (index >= 0 && index < 4)
                    {
                        UniformMat4Parameter* parameter = widget->parameter();
                        parameter->setType(static_cast<UniformMat4Type>(index));

                        lastFocusedParameter = parameter;

                        if (index < 3)
                            addInterpolation();
                        else if (index == 3)
                            removeInterpolation();

                        recreate();
                    }
                    else if (index == 4)
                    {
                        UniformMat4Parameter* parameter = widget->parameter();

                        UniformParameter<float>* newParam = new UniformParameter<float>(*parameter);
                        mOpBuilder->addUniformFloatParameter(newParam);
                        mOpBuilder->removeMat4UniformParameter(parameter);

                        lastFocusedParameter = newParam;

                        removeInterpolation();

                        recreate();
                    }
                });
            }
        });
    }
}



void OperationWidget::addInterpolation()
{
    bool interpolationExists = false;

    foreach (auto optionsWidget, glenumOptionsWidgets)
        if (optionsWidget->name() == "Interpolation")
            interpolationExists = true;

    if (!interpolationExists)
    {
        OptionsParameter<GLenum>* newParam = new OptionsParameter<GLenum>("Interpolation", true, QList<QString>({ "Nearest neighbor", "Linear" }), QList<GLenum>({ GL_NEAREST, GL_LINEAR }), GL_NEAREST, mOperation);
        mOperation->addOptionsParameter<GLenum>(newParam);
    }
}



void OperationWidget::removeInterpolation()
{
    foreach (auto optionsWidget, glenumOptionsWidgets)
    {
        if (optionsWidget->name() == "Interpolation")
            mOperation->removeOptionsParameter<GLenum>(optionsWidget->parameter());
    }
}



void OperationWidget::toggleOutputAction(OperationWidget* widget)
{
    if (widget != this)
    {
        outputAction->setChecked(false);
        headerWidget->setStyleSheet("QWidget { background-color: rgb(16, 64, 128); }");
    }
    else
        headerWidget->setStyleSheet("QWidget { background-color: rgb(128, 64, 16); }");
}



void OperationWidget::toggleMidiButton(bool show)
{
    midiLinkButton->setVisible(show);
}



void OperationWidget::toggleSelParamWidgets(bool visible)
{
    selParamSlider->setVisible(visible);
    selParamMinLineEdit->setVisible(visible);
    selParamMaxLineEdit->setVisible(visible);
    selParamInfLineEdit->setVisible(visible);
    selParamSupLineEdit->setVisible(visible);
    layoutComboBox->setVisible(visible);
    presetsGroupBox->setVisible(visible);
}



void OperationWidget::enableOperation(bool checked)
{
    mOperation->enable(checked);

    enableAction->setIcon(checked ? QIcon(QPixmap(":/icons/circle-green.png")) : QIcon(QPixmap(":/icons/circle-grey.png")));
    enableAction->setText(checked ? "Enabled" : "Disabled");
}



void OperationWidget::closeEvent(QCloseEvent* event)
{
    mOpBuilder->close();
    event->accept();
}



bool OperationWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == mOpBuilder && event->type() == QEvent::Close)
    {
        editAction->setChecked(false);
        toggleEditMode(false);
        return false;
    }

    return QWidget::eventFilter(obj, event);
}



template <class T>
void OperationWidget::setSelParamNameWidgets(ParameterWidget<T>* widget)
{
    // Name line edit

    paramNameLabel->setText(widget->name());

    paramNameLineEdit->setText(widget->name());
    paramNameLineEdit->setFixedWidth(20 + paramNameLineEdit->fontMetrics().horizontalAdvance(widget->name()));

    paramNameLineEdit->disconnect();

    connect(paramNameLineEdit, &QLineEdit::textEdited, this, [=, this](QString name){
        widget->setName(name);

        paramNameLabel->setText(name);

        paramNameLineEdit->setFixedWidth(20 + paramNameLineEdit->fontMetrics().horizontalAdvance(name));

        gridWidget->optimizeLayout();
    });
}



template <class T>
void OperationWidget::updateSelParamControls(ParameterWidget<T>* widget)
{
    Number<T>* number = widget->selectedNumber();

    // Slider

    selParamSlider->setVisible(true);

    selParamSlider->disconnect();
    selParamSlider->setRange(0, number->indexMax());
    selParamSlider->setValue(number->index());

    connect(selParamSlider, &QAbstractSlider::sliderMoved, widget, &ParameterWidget<T>::setValueFromIndex);
    connect(number, &Number<T>::indexChanged, selParamSlider, &QAbstractSlider::setValue);

    // Minimum

    selParamMinLineEdit->setVisible(true);

    selParamMinLineEdit->disconnect();

    setValidator<T>(minValidator, selParamMinLineEdit, number->inf(), number->max());

    selParamMinLineEdit->setText(QString::number(number->min()));

    connect(number, &Number<T>::minChanged, this, [=, this](){
        selParamMinLineEdit->setText(QString::number(number->min()));
    });

    connect(selParamMinLineEdit, &FocusLineEdit::editingFinished, this, [=, this](){
        if (std::is_same<T, float>::value)
            number->setMin(selParamMinLineEdit->text().toFloat());
        else if (std::is_same<T, int>::value)
            number->setMin(selParamMinLineEdit->text().toInt());
        else if (std::is_same<T, unsigned int>::value)
            number->setMin(selParamMinLineEdit->text().toUInt());
    });

    connect(selParamMinLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
        selParamMinLineEdit->setText(QString::number(number->min()));
    });

    // Maximum

    selParamMaxLineEdit->setVisible(true);

    selParamMaxLineEdit->disconnect();

    setValidator<T>(maxValidator, selParamMaxLineEdit, number->min(), number->sup());

    selParamMaxLineEdit->setText(QString::number(number->max()));

    connect(number, &Number<T>::maxChanged, this, [=, this](){
        selParamMaxLineEdit->setText(QString::number(number->max()));
    });

    connect(selParamMaxLineEdit, &FocusLineEdit::editingFinished, this, [=, this](){
        if (std::is_same<T, float>::value)
            number->setMax(selParamMaxLineEdit->text().toFloat());
        else if (std::is_same<T, int>::value)
            number->setMax(selParamMaxLineEdit->text().toInt());
        else if (std::is_same<T, unsigned int>::value)
            number->setMax(selParamMaxLineEdit->text().toUInt());
    });

    connect(selParamMaxLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
        selParamMaxLineEdit->setText(QString::number(number->max()));
    });

    // Focus-in connections

    connect(selParamMinLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamMinLineEdit;
    });

    connect(selParamMaxLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamMaxLineEdit;
    });
}



template <class T>
void OperationWidget::updateSelParamEditControls(ParameterWidget<T>* widget)
{
    Number<T>* number = widget->selectedNumber();

    // Slider

    selParamSlider->setVisible(true);

    selParamSlider->disconnect();
    selParamSlider->setRange(0, number->indexMax());
    selParamSlider->setValue(number->index());

    connect(selParamSlider, &QAbstractSlider::sliderMoved, widget, &ParameterWidget<T>::setValueFromIndex);
    connect(number, &Number<T>::indexChanged, selParamSlider, &QAbstractSlider::setValue);

    // Inf (lowest)

    selParamInfLineEdit->setVisible(true);

    selParamInfLineEdit->disconnect();

    setValidator<T>(infValidator, selParamInfLineEdit, std::numeric_limits<T>::lowest(), number->sup());

    selParamInfLineEdit->setText(QString::number(number->inf()));

    connect(selParamInfLineEdit, &FocusLineEdit::editingFinished, this, [=, this](){
        T inf = number->inf();

        if (std::is_same<T, float>::value)
            inf = selParamInfLineEdit->text().toFloat();
        else if (std::is_same<T, int>::value)
            inf = selParamInfLineEdit->text().toInt();
        else if (std::is_same<T, unsigned int>::value)
            inf = selParamInfLineEdit->text().toUInt();

        widget->setInf(inf);

        setValidator<T>(minValidator, selParamMinLineEdit, inf, number->max());
        setValidator<T>(supValidator, selParamSupLineEdit, inf, std::numeric_limits<T>::max());
    });

    connect(selParamInfLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
        selParamInfLineEdit->setText(QString::number(number->inf()));
    });

    // Minimum

    selParamMinLineEdit->setVisible(true);

    selParamMinLineEdit->disconnect();

    setValidator<T>(minValidator, selParamMinLineEdit, number->inf(), number->max());

    selParamMinLineEdit->setText(QString::number(number->min()));

    connect(number, &Number<T>::minChanged, this, [=, this](){
        selParamMinLineEdit->setText(QString::number(number->min()));
        widget->setMin(number->min());
    });

    connect(selParamMinLineEdit, &FocusLineEdit::editingFinished, this, [=, this](){
        if (std::is_same<T, float>::value)
            widget->setMin(selParamMinLineEdit->text().toFloat());
        else if (std::is_same<T, int>::value)
            widget->setMin(selParamMinLineEdit->text().toInt());
        else if (std::is_same<T, unsigned int>::value)
            widget->setMin(selParamMinLineEdit->text().toUInt());
    });

    connect(selParamMinLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
        selParamMinLineEdit->setText(QString::number(number->min()));
    });

    // Sup (highest)

    selParamSupLineEdit->setVisible(true);

    selParamSupLineEdit->disconnect();

    setValidator<T>(supValidator, selParamSupLineEdit, number->inf(), std::numeric_limits<T>::max());

    selParamSupLineEdit->setText(QString::number(number->sup()));

    connect(selParamSupLineEdit, &FocusLineEdit::editingFinished, this, [=, this](){
        T sup = number->sup();

        if (std::is_same<T, float>::value)
            sup = selParamSupLineEdit->text().toFloat();
        else if (std::is_same<T, int>::value)
            sup = selParamSupLineEdit->text().toInt();
        else if (std::is_same<T, unsigned int>::value)
            sup = selParamSupLineEdit->text().toUInt();

        widget->setSup(sup);
        setValidator<T>(maxValidator, selParamMaxLineEdit, number->min(), sup);
        setValidator<T>(infValidator, selParamInfLineEdit, std::numeric_limits<T>::lowest(), sup);
    });

    connect(selParamSupLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
        selParamSupLineEdit->setText(QString::number(number->sup()));
    });

    // Maximum

    selParamMaxLineEdit->setVisible(true);

    selParamMaxLineEdit->disconnect();

    setValidator<T>(maxValidator, selParamMaxLineEdit, number->min(), number->sup());

    selParamMaxLineEdit->setText(QString::number(number->max()));

    connect(number, &Number<T>::maxChanged, this, [=, this](){
        selParamMaxLineEdit->setText(QString::number(number->max()));
        widget->setMax(number->max());
    });

    connect(selParamMaxLineEdit, &FocusLineEdit::editingFinished, this, [=, this](){
        if (std::is_same<T, float>::value)
            widget->setMax(selParamMaxLineEdit->text().toFloat());
        else if (std::is_same<T, int>::value)
            widget->setMax(selParamMaxLineEdit->text().toInt());
        else if (std::is_same<T, unsigned int>::value)
            widget->setMax(selParamMaxLineEdit->text().toUInt());
    });

    connect(selParamMaxLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
        selParamMaxLineEdit->setText(QString::number(number->max()));
    });

    // Presets

    presetsGroupBox->setVisible(true);

    removePresetButton->disconnect();

    connect(removePresetButton, &QPushButton::clicked, this, [=, this](){
        widget->removeCurrentPreset();

        gridWidget->optimizeLayout();
    });

    addPresetButton->disconnect();

    connect(addPresetButton, &QPushButton::clicked, this, [=, this](){
        QString name = presetNameLineEdit->text();
        widget->addPreset(name);

        gridWidget->optimizeLayout();
    });

    // Focus-in connections

    connect(selParamInfLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamInfLineEdit;
    });

    connect(selParamMinLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamMinLineEdit;
    });

    connect(selParamMaxLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamMaxLineEdit;
    });

    connect(selParamSupLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamSupLineEdit;
    });
}



template <class T>
void OperationWidget::setValidator(QValidator* validator, QLineEdit* lineEdit, T bottom, T top)
{
    if (validator)
    {
        delete validator;
        validator = nullptr;
    }

    if (std::is_same<T, float>::value)
        validator = new QDoubleValidator(bottom, top, 5, lineEdit);
    else if (std::is_same<T, int>::value || std::is_same<T, unsigned int>::value)
        validator = new QIntValidator(bottom, top, lineEdit);

    if (validator)
        lineEdit->setValidator(validator);
}



template <typename T>
void OperationWidget::updateMidiButton(ParameterWidget<T>* widget)
{
    Number<T>* number = widget->selectedNumber();

    // MIDI Link button

    midiLinkButton->disconnect();

    midiLinkButton->setChecked(number->midiLinked());

    if (number->midiLinked())
        midiLinkButton->setStyleSheet("QPushButton { image: url(:/icons/circle-green.png); background-color: transparent; border: 0; }");
    else
        midiLinkButton->setStyleSheet("QPushButton { image: url(:/icons/circle-grey.png); background-color: transparent; border: 0; }");

    connect(midiLinkButton, &QPushButton::clicked, this, [=, this](bool checked){
        if (checked)
        {
            midiLinkButton->setStyleSheet("QPushButton { image: url(:/icons/circle-orange.png); background-color: transparent; border: 0; }");
            emit linkWait(number);
        }
        else
        {
            midiLinkButton->setStyleSheet("QPushButton { image: url(:/icons/circle-grey.png); background-color: transparent; border: 0; }");
            emit linkBreak(number);
        }
    });

    connect(number, &Number<T>::linked, this, [=, this](bool set){
        if (set)
            midiLinkButton->setStyleSheet("QPushButton { image: url(:/icons/circle-green.png); background-color: transparent; border: 0; }");
        else
            midiLinkButton->setStyleSheet("QPushButton { image: url(:/icons/circle-grey.png); background-color: transparent; border: 0; }");
    });
}



void OperationWidget::focusInEvent(QFocusEvent *event)
{
    if (lastFocusedWidget)
        lastFocusedWidget->setFocus(Qt::MouseFocusReason);
    event->accept();
}



void OperationWidget::updateWidgetRowCol(QWidget* widget, int row, int col)
{
    adjustSize();

    foreach (auto paramWidget, floatParamWidgets)
    {
        if (paramWidget->widget() == widget)
        {
            paramWidget->setRow(row);
            paramWidget->setCol(col);
            return;
        }
    }
    foreach (auto paramWidget, intParamWidgets)
    {
        if (paramWidget->widget() == widget)
        {
            paramWidget->setRow(row);
            paramWidget->setCol(col);
            return;
        }
    }
    foreach (auto paramWidget, uintParamWidgets)
    {
        if (paramWidget->widget() == widget)
        {
            paramWidget->setRow(row);
            paramWidget->setCol(col);
            return;
        }
    }
    foreach (auto paramWidget, mat4ParamWidgets)
    {
        if (paramWidget->widget() == widget)
        {
            paramWidget->setRow(row);
            paramWidget->setCol(col);
            return;
        }
    }
    foreach (auto paramWidget, glenumOptionsWidgets)
    {
        if (paramWidget->widget() == widget)
        {
            paramWidget->setRow(row);
            paramWidget->setCol(col);
            return;
        }
    }
}



void OperationWidget::toggleBody(bool visible)
{
    if (visible)
    {
        toggleBodyAction->setIcon(QIcon(QPixmap(":/icons/go-down.png")));
        toggleBodyAction->setText("Hide");
    }
    else
    {
        toggleBodyAction->setIcon(QIcon(QPixmap(":/icons/go-up.png")));
        toggleBodyAction->setText("Show");
    }

    selParamWidget->setVisible(visible);
    gridWidget->setVisible(visible);

    if (visible)
        mainLayout->setStretchFactor(gridWidget, 1);
    else
        mainLayout->setStretchFactor(gridWidget, 0);

    separator[0]->setVisible(visible);
    separator[1]->setVisible(visible);

    adjustSize();
}



void OperationWidget::toggleEditMode(bool mode)
{
    mEditMode = mode;

    mOpBuilder->setVisible(mEditMode);

    opNameLabel->setVisible(!mEditMode);
    opNameLineEdit->setVisible(mEditMode);

    gridWidget->setAcceptDrops(mEditMode);

    paramNameLabel->setVisible(!mEditMode);
    paramNameLineEdit->setVisible(mEditMode);

    selParamInfLineEdit->setVisible(mEditMode);
    selParamSupLineEdit->setVisible(mEditMode);

    if (!mEditMode && mat4TypeComboBox->isVisible())
        mat4TypeComboBox->setVisible(false);

    if (!mEditMode && layoutComboBox->isVisible())
        layoutComboBox->setVisible(false);

    presetsGroupBox->setVisible(mEditMode);

    recreate();
}

