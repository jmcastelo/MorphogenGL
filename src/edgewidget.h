#ifndef EDGEWIDGET_H
#define EDGEWIDGET_H



#include "parameters/number.h"

#include <QWidget>
#include <QUuid>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QSlider>
#include <QAction>



class EdgeWidget : public QFrame
{
    Q_OBJECT

public:
    explicit EdgeWidget(float factor, QWidget *parent = nullptr);
    ~EdgeWidget();

    QString const name();
    void setName(QString name);

    void setBlendFactor(float factor);

    void toggleMidiAction(bool show);

signals:
    void blendFactorChanged(float factor);

    void linkWait(Number<float>* number);
    void linkBreak(Number<float>* number);

    void remove();

private:
    QString mName;
    Number<float>* blendFactor;
    QAction* midiLinkAction;
};



#endif // EDGEWIDGET_H
