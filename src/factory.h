#ifndef FACTORY_H
#define FACTORY_H



#include "imageoperation.h"
#include "seed.h"
#include "operationwidget.h"
#include "seedwidget.h"

#include <QObject>
#include <QUuid>



class Factory : public QObject
{
    Q_OBJECT

public:
    explicit Factory(QObject *parent = nullptr);

    QList<ImageOperation*> operations();
    QList<Seed*> seeds();

    void createNewOperation();
    void createNewSeed();

    void deleteOperation(ImageOperation* operation);
    void deleteSeed(Seed* seed);

signals:
    void newOperationCreated(QUuid id, ImageOperation* operation);
    void newSeedCreated(QUuid id, Seed* seed);

    void newOperationWidgetCreated(QUuid id, OperationWidget* widget);
    void newSeedWidgetCreated(QUuid id, SeedWidget* widget);

private:
    QList<ImageOperation*> mOperations;
    QList<Seed*> mSeeds;
};



#endif // FACTORY_H
