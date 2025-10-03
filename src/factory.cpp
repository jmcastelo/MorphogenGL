


#include "factory.h"
#include "operationparser.h"

#include <QDir>
#include <QStringList>



Factory::Factory(QObject *parent)
    : QObject{parent}
{
    //scan();
}



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



void Factory::addAvailableOperation(int index)
{
    QUuid id = QUuid::createUuid();

    ImageOperation* operation = new ImageOperation(*mAvailOps[index]);
    mOperations.append(operation);
    emit newOperationCreated(id, operation);

    OperationWidget* widget = new OperationWidget(operation, false, false);
    emit newOperationWidgetCreated(id, widget);
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



QList<QString> Factory::availableOperationNames()
{
    QList<QString> opNames;

    foreach (ImageOperation* operation, mAvailOps) {
        opNames.append(operation->name());
    }

    return opNames;
}



ImageOperation* Factory::availableOperation(int index)
{
    if (index >= 0 && index < mAvailOps.size()) {
        return mAvailOps[index];
    }

    return nullptr;
}



void Factory::scan()
{
    QDir opsDir = QDir(QDir::currentPath() + "/operations");

    if (opsDir.exists())
    {
        QStringList filters;
        filters << "*.op";

        QStringList fileNames = opsDir.entryList(filters, QDir::Files | QDir::NoSymLinks, QDir::Name);

        mAvailOps.clear();

        OperationParser opParser;

        foreach (QString path, fileNames)
        {
            QString filePath = opsDir.absoluteFilePath(path);

            ImageOperation* operation = new ImageOperation();

            if (opParser.read(operation, filePath, false)) {
                mAvailOps.append(operation);
            }
        }
    }
}
