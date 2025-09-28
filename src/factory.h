#ifndef FACTORY_H
#define FACTORY_H



#include "imageoperation.h"
#include "seed.h"

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
    void newWidgetCreated(QUuid id, QWidget* widget);

private:
    QList<ImageOperation*> mOperations;
    QList<Seed*> mSeeds;
};



#endif // FACTORY_H
