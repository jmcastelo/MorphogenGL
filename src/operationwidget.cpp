


#include "operationwidget.h"
#include "parameters/uniformmat4parameter.h"

#include <limits>



OperationWidget::OperationWidget(ImageOperation* operation, bool midiEnabled, QWidget* parent) :
    QFrame(parent),
    mOperation { operation },
    mMidiEnabled {midiEnabled }
{
    mOpBuilder = new OperationBuilder(mOperation);
    mOpBuilder->installEventFilter(this);
    mOpBuilder->setVisible(false);

    connect(mOpBuilder, &OperationBuilder::shadersParsed, this, &OperationWidget::recreate);

    mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    // Header widget

    headerWidget = new QFrame;
    headerWidget->setFrameStyle(QFrame::Box | QFrame::Plain);
    headerWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // Enable button

    enableButton = new QPushButton;
    //enableButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    enableButton->setFixedSize(32, 32);
    enableButton->setStyleSheet("QPushButton { image: url(:/icons/circle-grey.png); background-color: transparent; border: 0; } QPushButton:checked { image: url(:/icons/circle-green.png); }");
    enableButton->setCheckable(true);

    // Edit button

    editButton = new QPushButton;
    //editButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    editButton->setFixedSize(32, 32);
    editButton->setStyleSheet("QPushButton { image: url(:/icons/applications-development.png); background-color: transparent; border: 0; }");
    editButton->setCheckable(true);
    editButton->setChecked(false);

    connect(editButton, &QPushButton::toggled, this, &OperationWidget::toggleEdit);
    connect(editButton, &QPushButton::toggled, this, &OperationWidget::setEditableWidgets);

    // Operation name label

    opNameLabel = new QLabel;
    //opNameLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    //opNameLabel->setMargin(10);
    opNameLabel->setFixedHeight(32);

    // Operation name line edit

    opNameLineEdit = new QLineEdit;
    //opNameLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    opNameLineEdit->setFixedHeight(32);
    opNameLineEdit->setVisible(false);

    connect(opNameLineEdit, &QLineEdit::textEdited, this, [=, this](QString name){
        operation->setName(name);
        opNameLineEdit->setFixedWidth(20 + opNameLineEdit->fontMetrics().horizontalAdvance(name));
        opNameLabel->setText(name);
    });

    // Toggle body button

    toggleButton = new QPushButton;
    //toggleButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    toggleButton->setFixedSize(32, 32);
    toggleButton->setStyleSheet("QPushButton{ image: url(:/icons/go-up.png); background-color: transparent; border: 0; } QPushButton:checked { image: url(:/icons/go-down.png); }");
    toggleButton->setCheckable(true);
    toggleButton->setChecked(true);

    connect(toggleButton, &QPushButton::toggled, this, &OperationWidget::toggleBody);

    QHBoxLayout* headerLayout = new QHBoxLayout;
    headerLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    headerLayout->setContentsMargins(10, 10, 10, 10);
    headerLayout->setSpacing(20);
    headerLayout->addWidget(enableButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
    headerLayout->addWidget(editButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
    headerLayout->addWidget(opNameLabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    headerLayout->addWidget(opNameLineEdit, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    headerLayout->addWidget(toggleButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    headerWidget->setLayout(headerLayout);
    mainLayout->addWidget(headerWidget, 0, Qt::AlignTop | Qt::AlignLeft);

    // Body widget

    bodyWidget = new QWidget;
    bodyWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    bodyWidget->installEventFilter(this);

    mainLayout->addWidget(bodyWidget, 1, Qt::AlignTop | Qt::AlignLeft);

    QVBoxLayout* bodyLayout = new QVBoxLayout;
    bodyLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    bodyWidget->setLayout(bodyLayout);

    // Grid widget containing parameter widgets

    gridWidget = new GridWidget;
    gridWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    connect(gridWidget, &GridWidget::itemRowColChanged, this, &OperationWidget::updateWidgetRowCol);

    bodyLayout->addWidget(gridWidget, 0, Qt::AlignTop | Qt::AlignLeft);

    // Selected parameter widgets

    paramNameLineEdit = new QLineEdit;
    paramNameLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    paramNameLineEdit->setPlaceholderText("Parameter name");
    paramNameLineEdit->setVisible(false);

    selParamDial = new QDial;
    selParamDial->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    selParamDial->setRange(0, 100'000);
    selParamDial->setWrapping(false);

    midiLinkButton = new QPushButton;
    midiLinkButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    midiLinkButton->setCheckable(true);
    midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");

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

    QHBoxLayout* defaultLayout = new QHBoxLayout;
    defaultLayout->addWidget(midiLinkButton);
    defaultLayout->addWidget(selParamDial);
    defaultLayout->addWidget(selParamMinLineEdit);
    defaultLayout->addWidget(selParamMaxLineEdit);

    QHBoxLayout* extraLayout = new QHBoxLayout;
    extraLayout->addWidget(layoutComboBox);
    extraLayout->addWidget(mat4TypeComboBox);
    extraLayout->addWidget(selParamInfLineEdit);
    extraLayout->addWidget(selParamSupLineEdit);

    QVBoxLayout* selParamLayout = new QVBoxLayout;
    selParamLayout->addWidget(paramNameLineEdit, 0, Qt::AlignLeft | Qt::AlignTop);
    selParamLayout->addLayout(defaultLayout);
    selParamLayout->addLayout(extraLayout);

    selParamGroupBox = new QGroupBox("No parameter selected");
    selParamGroupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    selParamGroupBox->setStyleSheet("QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; font-size: 18pt; margin: 7px; }");
    selParamGroupBox->setLayout(selParamLayout);
    selParamGroupBox->setVisible(false);

    bodyLayout->addWidget(selParamGroupBox, 0, Qt::AlignTop | Qt::AlignHCenter);

    setLayout(mainLayout);

    setFrameStyle(QFrame::Box | QFrame::Plain);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setup();
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

        floatParamWidgets.append(widget);
        uniformFloatParamWidgets.append(widget);

        setFocusedWidget(widget);
    }

    foreach (auto parameter, mOperation->uniformParameters<int>())
    {
        UniformParameterWidget<int>* widget = new UniformParameterWidget<int>(parameter);
        gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());

        intParamWidgets.append(widget);
        uniformIntParamWidgets.append(widget);

        setFocusedWidget(widget);
    }

    foreach (auto parameter, mOperation->uniformParameters<unsigned int>())
    {
        UniformParameterWidget<unsigned int>* widget = new UniformParameterWidget<unsigned int>(parameter);
        gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());

        uintParamWidgets.append(widget);
        uniformUintParamWidgets.append(widget);

        setFocusedWidget(widget);
    }

    foreach (auto parameter, mOperation->mat4UniformParameters())
    {
        UniformMat4ParameterWidget* widget = new UniformMat4ParameterWidget(parameter);
        gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());

        mat4ParamWidgets.append(widget);
        uniformMat4ParamWidgets.append(widget);

        setFocusedWidget(widget);
    }

    foreach (auto parameter, mOperation->optionsParameters<GLenum>())
    {
        OptionsParameterWidget<GLenum>* widget = new OptionsParameterWidget<GLenum>(parameter);
        gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());

        glenumOptionsWidgets.append(widget);
    }

    // Once widgets set on grid, optimize its layout to set it with proper row and column spans and sizes

    if (gridWidget->isVisible())
        gridWidget->optimizeLayout();

    // Selected parameter widget

    if (!floatParamWidgets.empty() || !intParamWidgets.empty() || !uintParamWidgets.empty() || !mat4ParamWidgets.empty())
    {
        selParamGroupBox->setVisible(true);

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
    else
    {
        selParamGroupBox->setVisible(false);
    }

    // Operation name controls

    opNameLabel->setText(mOperation->name());

    opNameLineEdit->setText(mOperation->name());
    opNameLineEdit->setFixedWidth(20 + opNameLineEdit->fontMetrics().horizontalAdvance(mOperation->name()));

    // Midi link button

    midiLinkButton->setVisible(mMidiEnabled);

    // Enable button

    enableButton->setChecked(mOperation->isEnabled());

    disconnect(enableButton, &QPushButton::toggled, this, nullptr);

    connect(enableButton, &QPushButton::toggled, this, [=, this](bool checked){
        mOperation->enable(checked);
    });

    // Toggle editable widgets

    setEditableWidgets(editMode);

    //bodyWidget->updateGeometry();
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
    {
        lastFocusedWidget = widget->lastFocusedWidget();
        lastFocused = true;
    }
}



template <typename T>
void OperationWidget::connectParamWidgets(QList<ParameterWidget<T>*> widgets)
{
    foreach (auto widget, widgets)
    {
        connect(widget, &ParameterWidget<T>::focusIn, this, [=, this](){
            if (editMode)
                updateSelParamEditControls<T>(widget);
            else
                updateSelParamControls<T>(widget);

            updateMidiButtons<T>(widget);
        });

        connect(widget, &ParameterWidget<T>::focusIn, this, [=, this](){
            lastFocusedParameter = widget->parameter();
            lastFocusedWidget = widget->lastFocusedWidget();
            lastFocused = true;
        });
    }
}



template <typename T>
void OperationWidget::connectUniformParamWidgets(QList<UniformParameterWidget<T>*> widgets)
{
    foreach (auto widget, widgets)
    {
        connect(widget, &UniformParameterWidget<T>::focusIn, this, [=, this](){
            if (editMode)
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
            if (editMode)
            {
                // Set up Mat4 parameter switcher combo box

                mat4TypeComboBox->disconnect();

                if (widget->isMat4Equivalent())
                {
                    mat4TypeComboBox->setVisible(true);
                    mat4TypeComboBox->setCurrentIndex(4);

                    connect(mat4TypeComboBox, &QComboBox::currentIndexChanged, this, [=, this](int index){
                        if (index < 4)
                        {
                            UniformParameter<float>* parameter = widget->parameter();

                            UniformMat4Parameter* newParam = new UniformMat4Parameter(*parameter, static_cast<UniformMat4Type>(index));
                            mOperation->addMat4UniformParameter(newParam);
                            mOperation->removeUniformParameter<float>(parameter);

                            lastFocusedParameter = newParam;

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
            if (editMode)
            {
                // Set up Mat4 parameter switcher combo box

                mat4TypeComboBox->disconnect();

                mat4TypeComboBox->setVisible(true);
                mat4TypeComboBox->setCurrentIndex(widget->typeIndex());

                connect(mat4TypeComboBox, &QComboBox::currentIndexChanged, this, [=, this](int index){
                    if (index < 4)
                    {
                        UniformMat4Parameter* parameter = widget->parameter();
                        parameter->setType(static_cast<UniformMat4Type>(index));

                        lastFocusedParameter = parameter;

                        recreate();
                    }
                });
            }
        });
    }
}



void OperationWidget::toggleEnableButton(bool checked)
{
    enableButton->setChecked(checked);
}



void OperationWidget::toggleMidiButton(bool show)
{
    midiLinkButton->setVisible(show);
}



void OperationWidget::closeEvent(QCloseEvent* event)
{
    mOpBuilder->close();
    event->accept();
}



bool OperationWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == bodyWidget && event->type() == QEvent::Resize)
    {
        QResizeEvent *resizeEvent = static_cast<QResizeEvent*>(event);
        QSize newSize = resizeEvent->size();

        resize(QSize(qMax(newSize.width(), headerWidget->width()), newSize.height() + headerWidget->height()));
        updateGeometry();
        return false;
    }
    else if (obj == mOpBuilder && event->type() == QEvent::Close)
    {
        editButton->setChecked(false);
        return false;
    }
    return QWidget::eventFilter(obj, event);
}



template <class T>
void OperationWidget::updateSelParamControls(ParameterWidget<T>* widget)
{
    Number<T>* number = widget->selectedNumber();

    // Title

    selParamGroupBox->setTitle("Selected parameter: " + widget->name());

    // Name line edit

    paramNameLineEdit->setText(widget->name());
    paramNameLineEdit->setFixedWidth(20 + paramNameLineEdit->fontMetrics().horizontalAdvance(widget->name()));

    paramNameLineEdit->disconnect();

    connect(paramNameLineEdit, &QLineEdit::textEdited, this, [=, this](QString name){
        widget->setName(name);
        selParamGroupBox->setTitle("Selected parameter: " + name);
        paramNameLineEdit->setFixedWidth(20 + paramNameLineEdit->fontMetrics().horizontalAdvance(name));
    });

    // Dial

    selParamDial->disconnect();
    selParamDial->setRange(0, number->indexMax());
    selParamDial->setValue(number->index());

    connect(selParamDial, &QAbstractSlider::sliderMoved, widget, &ParameterWidget<T>::setValueFromIndex);
    connect(number, &Number<T>::indexChanged, selParamDial, &QAbstractSlider::setValue);

    // Minimum

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

    // Focus in

    connect(selParamMinLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamMinLineEdit;
        lastFocused = true;
    });

    connect(selParamMaxLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamMaxLineEdit;
        lastFocused = true;
    });
}



template <class T>
void OperationWidget::updateSelParamEditControls(ParameterWidget<T>* widget)
{
    Number<T>* number = widget->selectedNumber();

    // Title

    selParamGroupBox->setTitle("Selected parameter: " + widget->name());

    // Name line edit

    paramNameLineEdit->setText(widget->name());
    paramNameLineEdit->setFixedWidth(20 + paramNameLineEdit->fontMetrics().horizontalAdvance(widget->name()));

    paramNameLineEdit->disconnect();

    connect(paramNameLineEdit, &QLineEdit::textEdited, this, [=, this](QString name){
        widget->setName(name);
        selParamGroupBox->setTitle("Selected parameter: " + name);
        paramNameLineEdit->setFixedWidth(20 + paramNameLineEdit->fontMetrics().horizontalAdvance(name));
    });

    // Dial

    selParamDial->disconnect();
    selParamDial->setRange(0, number->indexMax());
    selParamDial->setValue(number->index());

    connect(selParamDial, &QAbstractSlider::sliderMoved, widget, &ParameterWidget<T>::setValueFromIndex);
    connect(number, &Number<T>::indexChanged, selParamDial, &QAbstractSlider::setValue);

    // Inf (lowest)

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

    // Focus in

    connect(selParamInfLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamInfLineEdit;
        lastFocused = true;
    });

    connect(selParamMinLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamMinLineEdit;
        lastFocused = true;
    });

    connect(selParamMaxLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamMaxLineEdit;
        lastFocused = true;
    });

    connect(selParamSupLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamSupLineEdit;
        lastFocused = true;
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
void OperationWidget::updateMidiButtons(ParameterWidget<T>* widget)
{
    Number<T>* number = widget->selectedNumber();

    // MIDI Link button

    midiLinkButton->disconnect();

    midiLinkButton->setChecked(number->midiLinked());

    if (number->midiLinked())
        midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-green.png); }");
    else
        midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");

    connect(midiLinkButton, &QPushButton::clicked, this, [=, this](bool checked){
        if (checked)
        {
            midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-orange.png); }");
            emit linkWait(number);
        }
        else
        {
            midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");
            emit linkBreak(number);
        }
    });

    connect(number, &Number<T>::linked, this, [=, this](bool set){
        if (set)
            midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-green.png); }");
        else
            midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");
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
    bodyWidget->setVisible(visible);

    if (!visible)
    {
        setFixedSize(headerWidget->size());
        updateGeometry();
    }
}



void OperationWidget::toggleEdit(bool state)
{
    mOpBuilder->setVisible(state);

    opNameLabel->setVisible(!state);
    opNameLineEdit->setVisible(state);

    gridWidget->setAcceptDrops(state);

    paramNameLineEdit->setVisible(state);

    selParamInfLineEdit->setVisible(state);
    selParamSupLineEdit->setVisible(state);

    editMode = state;
}



void OperationWidget::setEditableWidgets(bool state)
{
    foreach (auto paramWidget, floatParamWidgets)
    {
        paramWidget->setCheckable(state);

        if (state)
            paramWidget->toggleVisibility(true);
        else
            paramWidget->setDefaultVisibility();
    }

    foreach (auto paramWidget, intParamWidgets)
    {
        paramWidget->setCheckable(state);

        if (state)
            paramWidget->toggleVisibility(true);
        else
            paramWidget->setDefaultVisibility();
    }

    foreach (auto paramWidget, uintParamWidgets)
    {
        paramWidget->setCheckable(state);

        if (state)
            paramWidget->toggleVisibility(true);
        else
            paramWidget->setDefaultVisibility();
    }

    foreach (auto paramWidget, mat4ParamWidgets)
    {
        paramWidget->setCheckable(state);

        if (state)
            paramWidget->toggleVisibility(true);
        else
            paramWidget->setDefaultVisibility();
    }

    foreach (auto paramWidget, glenumOptionsWidgets)
    {
        paramWidget->setCheckable(state);

        if (state)
            paramWidget->toggleVisibility(true);
        else
            paramWidget->setDefaultVisibility();
    }
}
