#ifndef EDGEWIDGET_H
#define EDGEWIDGET_H



#include "parameters/number.h"
#include "midisignals.h"

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
    explicit EdgeWidget(float factor, bool srcIsOp, QWidget *parent = nullptr);
    ~EdgeWidget();

    QString const name();
    void setName(QString name);

    void setBlendFactor(float factor);

    MidiSignals* midiSignals();

    void toggleTypeAction(bool predge);

signals:
    void blendFactorChanged(float factor);

    void edgeTypeChanged(bool predge);
    void typeActionToggled(bool checked);

    void remove();

public slots:
    void toggleMidiAction(bool show);

private:
    QString mName;

    Number<float>* blendFactor;

    MidiSignals* mMidiSignals;

    QAction* midiLinkAction;
    QAction* mTypeAction;
    QAction* mRemoveAction;
};



#endif // EDGEWIDGET_H
