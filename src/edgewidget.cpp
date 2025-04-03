


#include "edgewidget.h"
#include "widgets/focuswidgets.h"

#include <QToolBar>



EdgeWidget::EdgeWidget(float factor, bool srcIsOp, QWidget *parent) :
    QFrame(parent)
{
    // Blend factor

    blendFactor = new Number<float>(factor, 0.0, 1.0, 0.0, 1.0);

    // Blend factor line edit

    FocusLineEdit* lineEdit = new FocusLineEdit;
    lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QDoubleValidator* validator = new QDoubleValidator(0.0, 1.0, 6, lineEdit);
    validator->setLocale(QLocale::English);
    lineEdit->setValidator(validator);
    lineEdit->setText(QString::number(factor));

    // Header toolbar

    QToolBar* headerToolBar = new QToolBar;
    headerToolBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // Midi link

    midiLinkAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/circle-grey.png")), "Midi link");
    midiLinkAction->setCheckable(true);
    midiLinkAction->setVisible(false);

    // Set edge type

    QAction* typeAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/edit-undo.png")), "Set as predge", this, &EdgeWidget::edgeTypeChanged);
    typeAction->setCheckable(true);
    typeAction->setVisible(srcIsOp);

    // Delete

    headerToolBar->addAction(QIcon(QPixmap(":/icons/dialog-close.png")), "Delete", this, &EdgeWidget::remove);

    QSlider* slider = new QSlider(Qt::Horizontal);
    slider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    slider->setRange(0, blendFactor->indexMax());
    slider->setValue(blendFactor->index());

    QHBoxLayout* headerLayout = new QHBoxLayout;
    headerLayout->addWidget(lineEdit);
    headerLayout->addWidget(headerToolBar);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(headerLayout);
    layout->addWidget(slider);

    connect(midiLinkAction, &QAction::toggled, this, [=, this](bool checked){
        if (checked)
        {
            midiLinkAction->setIcon(QIcon(QPixmap(":/icons/circle-orange.png")));
            emit linkWait(blendFactor);
        }
        else
        {
            midiLinkAction->setIcon(QIcon(QPixmap(":/icons/circle-grey.png")));
            emit linkBreak(blendFactor);
        }
    });

    connect(blendFactor, &Number<float>::linked, this, [=, this](bool set){
        if (set)
            midiLinkAction->setIcon(QIcon(QPixmap(":/icons/circle-green.png")));
        else
            midiLinkAction->setIcon(QIcon(QPixmap(":/icons/circle-grey.png")));

        slider->setRange(0, blendFactor->indexMax());
    });

    connect(slider, &QAbstractSlider::sliderMoved, blendFactor, &Number<float>::setValueFromIndex);

    connect(blendFactor, &Number<float>::indexChanged, slider, &QAbstractSlider::setValue);

    connect(blendFactor, &Number<float>::valueChanged, this, [=, this](QVariant value){
        lineEdit->setText(QString::number(value.toFloat()));
        blendFactor->setIndex();
        emit blendFactorChanged(value.toFloat());
    });

    connect(lineEdit, &QLineEdit::editingFinished, this, [=, this](){
        blendFactor->setValue(lineEdit->text().toFloat());
        blendFactor->setIndex();
        emit blendFactorChanged(lineEdit->text().toFloat());
    });

    connect(lineEdit, &FocusLineEdit::focusOut, this, [=, this](){
        lineEdit->setText(QString::number(blendFactor->value()));
    });

    setLayout(layout);

    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Raised);
    setMidLineWidth(3);
    setLineWidth(3);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}



EdgeWidget::~EdgeWidget()
{
    delete blendFactor;
}



const QString EdgeWidget::name()
{
    return mName;
}


void EdgeWidget::setName(QString name) { mName = name; }



void EdgeWidget::setBlendFactor(float factor)
{
    blendFactor->setValue(factor);
}



void EdgeWidget::toggleMidiAction(bool show)
{
    midiLinkAction->setVisible(show);
}
