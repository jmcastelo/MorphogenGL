#ifndef BLENDFACTORWIDGET_H
#define BLENDFACTORWIDGET_H


/*
#include "edge.h"
#include "node.h"
#include "parameter.h"

#include <QWidget>
#include <QUuid>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QSlider>
#include <QPushButton>



class BlendFactorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BlendFactorWidget(Edge* edge, float factor, QWidget *parent = nullptr) :
        QWidget(parent)
    {
        id = QUuid::createUuid();

        blendFactor = new Number<float>(factor, 0.0, 1.0, 0.0, 1.0);

        QLineEdit* lineEdit = new QLineEdit;
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
        slider->setRange(0, blendFactor->indexMax());
        slider->setValue(blendFactor->index());

        midiLinkButton = new QPushButton();
        midiLinkButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        midiLinkButton->setCheckable(true);
        midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");
        midiLinkButton->setVisible(false);

        connect(midiLinkButton, &QPushButton::clicked, this, [=, this](bool checked)
        {
            if (checked)
            {
                midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-orange.png); }");
                emit linkWait(blendFactor);
            }
            else
            {
                midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");
                emit linkBreak(blendFactor);
            }
        });

        connect(blendFactor, &Number<float>::linked, this, [=, this](bool set)
        {
            if (set)
                midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-green.png); }");
            else
                midiLinkButton->setStyleSheet("QPushButton{ qproperty-icon: url(:/icons/circle-grey.png); }");

            slider->setRange(0, blendFactor->indexMax());
        });

        QHBoxLayout* sliderLayout = new QHBoxLayout;
        sliderLayout->addWidget(midiLinkButton);
        sliderLayout->addWidget(slider);

        connect(slider, &QAbstractSlider::sliderMoved, blendFactor, &Number<float>::setValueFromIndex);

        connect(blendFactor, &Number<float>::indexChanged, slider, &QAbstractSlider::setValue);

        connect(blendFactor, &Number<float>::valueChanged, this, [=, this](QVariant value)
        {
            lineEdit->setText(QString::number(value.toFloat()));
            blendFactor->setIndex();
            edge->setBlendFactor(value.toFloat());
        });

        connect(lineEdit, &QLineEdit::editingFinished, this, [=, this]()
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
            toggle(false);
        });

        QVBoxLayout* layout = new QVBoxLayout;
        layout->setSizeConstraint(QLayout::SetMaximumSize);
        layout->addLayout(formLayout);
        layout->addLayout(sliderLayout);
        layout->addWidget(closeButton);

        //name = edge->sourceNode()->name + " - " + edge->destNode()->name;

        groupBox = new QGroupBox(name);
        groupBox->setLayout(layout);

        QVBoxLayout* mainLayout = new QVBoxLayout;
        mainLayout->addWidget(groupBox);

        setLayout(mainLayout);
        setVisible(false);
    }

    ~BlendFactorWidget()
    {
        delete blendFactor;
    }

    void setTitle(QString title)
    {
        name = title;
        groupBox->setTitle(title);
    }

    void toggle(bool visible)
    {
        setVisible(visible);
        emit toggled(this);
    }

    void setBlendFactor(float factor)
    {
        blendFactor->setValue(factor);
    }

    void toggleMidiButton(bool show)
    {
        midiLinkButton->setVisible(show);
    }

    QUuid id;
    Number<float>* blendFactor;

    QString getName(){ return name; }

signals:
    void toggled(BlendFactorWidget* widget);
    void linkWait(Number<float>* number);
    void linkBreak(Number<float>* number);

private:
    QString name;
    QPushButton* midiLinkButton;
    QGroupBox* groupBox;
};
*/


#endif // BLENDFACTORWIDGET_H
