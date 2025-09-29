


#include "factory.h"
#include "operationwidget.h"
#include "seedwidget.h"



Factory::Factory(QObject *parent)
    : QObject{parent}
{}



QList<ImageOperation*> Factory::operations()
{
    return mOperations;
}



QList<Seed*> Factory::seeds()
{
    return mSeeds;
}



void Factory::createNewOperation()
{
    QUuid id = QUuid::createUuid();

    ImageOperation* operation = new ImageOperation();
    mOperations.append(operation);
    emit newOperationCreated(id, operation);

    OperationWidget* widget = new OperationWidget(operation, false, true);
    emit newOperationWidgetCreated(id, widget);
}



void Factory::createNewSeed()
{
    QUuid id = QUuid::createUuid();

    Seed* seed = new Seed();
    mSeeds.append(seed);
    emit newSeedCreated(id, seed);

    SeedWidget* widget = new SeedWidget(seed);
    emit newSeedWidgetCreated(id, widget);
}



void Factory::deleteOperation(ImageOperation* operation)
{
    mOperations.removeOne(operation);
    delete operation;
}



void Factory::deleteSeed(Seed* seed)
{
    mSeeds.removeOne(seed);
    delete seed;
}
