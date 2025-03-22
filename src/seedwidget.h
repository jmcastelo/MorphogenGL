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

public slots:
    void toggleOutputAction(QWidget* widget);

private:
    Seed* mSeed;

    QWidget* headerWidget;
    QAction* outputAction;

private slots:
    void drawSeed();
    void setFixedSeed(bool checked);
    void setSeedType();
    void loadSeedImage();
};



#endif // SEEDWIDGET_H
