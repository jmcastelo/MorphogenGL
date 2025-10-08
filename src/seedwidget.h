#ifndef SEEDWIDGET_H
#define SEEDWIDGET_H



#include "seed.h"

#include <QObject>
#include <QFrame>
#include <QUuid>



class SeedWidget : public QFrame
{
    Q_OBJECT

public:
    SeedWidget(QUuid id, Seed* seed, QWidget* parent = nullptr);

    QUuid id();

signals:
    void outputChanged(QUuid id);
    void typeChanged();
    void seedDrawn();
    void connectTo();
    void remove(QUuid id);

public slots:
    void toggleOutputAction(QUuid id);

private:
    QUuid mId;
    Seed* mSeed;

    QWidget* headerWidget;

    QAction* outputAction;
    QAction* fixedAction;
    QAction* imageAction;

private slots:
    void setFixedSeed(bool fixed);
    void setSeedType();
    void loadSeedImage();
};



#endif // SEEDWIDGET_H
