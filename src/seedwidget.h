#ifndef SEEDWIDGET_H
#define SEEDWIDGET_H



#include "seed.h"

#include <QObject>
#include <QFrame>



class SeedWidget : public QFrame
{
    Q_OBJECT

public:
    SeedWidget(Seed* seed, QWidget* parent = nullptr);

signals:
    void outputChanged(bool checked);
    void typeChanged();
    void seedDrawn();
    void connectTo();
    void remove();

public slots:
    void toggleOutputAction(bool show);

private:
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
