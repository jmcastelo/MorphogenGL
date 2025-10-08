


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

    OperationWidget* widget = new OperationWidget(id, operation, false, true);
    emit newOperationWidgetCreated(id, widget);
}



void Factory::createNewSeed()
{
    QUuid id = QUuid::createUuid();

    Seed* seed = new Seed();
    mSeeds.append(seed);
    emit newSeedCreated(id, seed);

    SeedWidget* widget = new SeedWidget(id, seed);
    emit newSeedWidgetCreated(id, widget);
}



void Factory::addAvailableOperation(int index)
{
    QUuid id = QUuid::createUuid();

    ImageOperation* operation = new ImageOperation(*mAvailOps[index]);
    mOperations.append(operation);
    emit newOperationCreated(id, operation);

    OperationWidget* widget = new OperationWidget(id, operation, false, false);
    emit newOperationWidgetCreated(id, widget);
}



void Factory::addOperation(QUuid id, ImageOperation* operation, bool midiEnabled)
{
    mOperations.append(operation);
    emit newOperationCreated(id, operation);

    OperationWidget* widget = new OperationWidget(id, operation, midiEnabled, false);
    emit newOperationWidgetCreated(id, widget);
}



void Factory::addSeed(QUuid id, Seed* seed)
{
    mSeeds.append(seed);
    emit newSeedCreated(id, seed);

    SeedWidget* widget = new SeedWidget(id, seed);
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



template<>
Number<float>* Factory::number(QUuid id)
{
    Number<float>* number = nullptr;

    foreach (ImageOperation* operation, mOperations)
    {
        number = operation->number<float>(id);
        if (number) {
            return number;
        }
    }

    return nullptr;
}



template<>
Number<int>* Factory::number(QUuid id)
{
    Number<int>* number = nullptr;

    foreach (ImageOperation* operation, mOperations)
    {
        number = operation->number<int>(id);
        if (number) {
            return number;
        }
    }

    return nullptr;
}



template<>
Number<unsigned int>* Factory::number(QUuid id)
{
    Number<unsigned int>* number = nullptr;

    foreach (ImageOperation* operation, mOperations)
    {
        number = operation->number<unsigned int>(id);
        if (number) {
            return number;
        }
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



void Factory::clear()
{
    qDeleteAll(mOperations);
    mOperations.clear();

    qDeleteAll(mSeeds);
    mSeeds.clear();

    emit cleared();
}
