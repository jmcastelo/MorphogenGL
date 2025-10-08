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
#include <QToolBar>
#include <QAction>



class EdgeWidget : public QFrame
{
    Q_OBJECT

public:
    explicit EdgeWidget(Number<float>* blendFactor, bool srcIsOp, QWidget *parent = nullptr);

    QString const name();
    void setName(QString name);

    void setBlendFactor(float factor);

    MidiSignals* midiSignals();

    void toggleTypeAction(bool predge);

    void adjustAllSizes();

signals:
    // void blendFactorChanged(float factor);

    void edgeTypeChanged(bool predge);
    void typeActionToggled(bool checked);

    void remove();

public slots:
    void toggleMidiAction(bool show);

private:
    QString mName;

    Number<float>* mBlendFactor;

    MidiSignals* mMidiSignals;

    QToolBar* headerToolBar;
    QAction* midiLinkAction;
    QAction* mTypeAction;
    QAction* mRemoveAction;
};



#endif // EDGEWIDGET_H
