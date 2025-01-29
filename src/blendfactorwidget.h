#ifndef BLENDFACTORWIDGET_H
#define BLENDFACTORWIDGET_H



#include "edge.h"
#include "node.h"
#include "parameter.h"
#include "focuswidgets.h"


#include <QWidget>
#include <QFormLayout>



class BlendFactorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BlendFactorWidget(Edge* edge, float factor, QWidget *parent = nullptr) :
        QWidget(parent)
    {
        Number<float>* blendFactor = new Number<float>(factor, 0.0, 1.0, 0.0, 1.0);

        FocusLineEdit* lineEdit = new FocusLineEdit;
        lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        QDoubleValidator* validator = new QDoubleValidator(0.0, 1.0, 6, lineEdit);
        validator->setLocale(QLocale::English);
        lineEdit->setValidator(validator);
        lineEdit->setText(QString::number(factor));

        QFormLayout* formLayout = new QFormLayout;
        formLayout->setAlignment(Qt::AlignLeft);
        formLayout->addRow("Blend factor:", lineEdit);

        QSlider* slider = new QSlider(Qt::Horizontal);
        slider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        slider->setRange(0, blendFactor->indexMax);
        slider->setValue(blendFactor->getIndex());

        connect(slider, &QAbstractSlider::sliderMoved, blendFactor, &Number<float>::setValueFromIndex);

        connect(blendFactor, &Number<float>::currentIndexChanged, slider, &QAbstractSlider::setValue);

        connect(blendFactor, QOverload<float>::of(&Number<float>::currentValueChanged), this, [=, this](float currentValue)
        {
            edge->setBlendFactor(currentValue);
            emit blendFactorChanged(edge->sourceNode()->id, edge->destNode()->id, currentValue);
        });

        connect(lineEdit, &FocusLineEdit::editingFinished, this, [=]()
        {
            blendFactor->setValue(lineEdit->text().toFloat());
            blendFactor->setIndex();
        });


        QPushButton* closeButton = new QPushButton;
        closeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        closeButton->setFixedHeight(10);
        closeButton->setStyleSheet("background-color: rgb(255, 0, 0); color: rgb(255, 255, 255)");

        connect(closeButton, &QPushButton::clicked, this, [&]()
        {
            setVisible(false);
            emit blendFactorWidgetToggled();
        });

        QVBoxLayout* layout = new QVBoxLayout;
        layout->setSizeConstraint(QLayout::SetFixedSize);
        layout->addLayout(formLayout);
        layout->addWidget(slider);
        layout->addWidget(closeButton);

        groupBox = new QGroupBox(edge->sourceNode()->name + " - " + edge->destNode()->name);
        groupBox->setLayout(layout);

        QVBoxLayout* mainLayout = new QVBoxLayout;
        mainLayout->addWidget(groupBox);

        setLayout(mainLayout);
        setVisible(false);
    }

    void setTitle(QString title)
    {
        groupBox->setTitle(title);
    }

signals:
    void blendFactorChanged(QUuid srcId, QUuid dstId, float factor);
    void blendFactorWidgetToggled();

public slots:
    void setBlendFactor();

private:
    QGroupBox* groupBox;
};



#endif // BLENDFACTORWIDGET_H
