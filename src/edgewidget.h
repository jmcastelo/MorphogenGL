#ifndef EDGEWIDGET_H
#define EDGEWIDGET_H



#include "parameters/number.h"
#include "midisignals.h"
#include "factory.h"

#include <QWidget>
#include <QUuid>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QSlider>
#include <QToolBar>
#include <QAction>
#include <QMenu>



class EdgeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EdgeWidget(Number<float>* blendFactor, bool srcIsOp, Factory* factory, QWidget *parent = nullptr);

    QString const name();
    void setName(QString name);

    void setBlendFactor(float factor);

    MidiSignals* midiSignals();

    void toggleTypeAction(bool predge);

    void adjustAllSizes();

signals:
    void edgeTypeChanged(bool predge);
    void typeActionToggled(bool checked);

    void operationInsert(int index);
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
    QAction* mInsertOpAction;
    QAction* mRemoveAction;

    QMenu* mAvailOpsMenu;
    QList<QAction*> mAvailOpsActions;

    Factory* mFactory;

private slots:
    void populateAvailOpsMenu();
    void insertOperation(QAction* action);
};



#endif // EDGEWIDGET_H
