#ifndef FACTORY_H
#define FACTORY_H



#include "imageoperation.h"
#include "seed.h"
#include "operationwidget.h"
#include "seedwidget.h"
#include "parameters/number.h"

#include <QObject>
#include <QUuid>
#include <QList>
#include <QString>
#include <QPointF>



class Factory : public QObject
{
    Q_OBJECT

public:
    explicit Factory(QObject *parent = nullptr);
    ~Factory();

    QList<ImageOperation*> operations();
    QList<Seed*> seeds();

    void createNewOperation();
    void createNewSeed();

    void addAvailableOperation(int index);

    void addOperation(QUuid id, ImageOperation* operation, bool midiEnabled, QPointF position);
    void addSeed(QUuid id, Seed* operation);

    ImageOperation* createReplaceOp(QUuid id, ImageOperation* oldOperation, int index);

    void deleteOperation(ImageOperation* operation);
    void deleteSeed(Seed* seed);

    QList<QString> availableOperationNames();
    ImageOperation* availableOperation(int index);

    template <class T>
    Number<T>* number(QUuid id);

    void scan();
    void clear();

signals:
    void newOperationCreated(QUuid id, ImageOperation* operation);
    void newSeedCreated(QUuid id, Seed* seed);

    void newOpWidgetCreated(OperationWidget* widget);
    void newOpWidgetCreated(QUuid id, OperationWidget* widget);
    void newOpWidgetCreated(QUuid id, OperationWidget* widget, QPointF position);
    void newSeedWidgetCreated(QUuid id, SeedWidget* widget);

    void replaceOpCreated(QUuid id, ImageOperation* operation);

    void cleared();

private:
    QList<ImageOperation*> mAvailOps;
    QList<ImageOperation*> mOperations;
    QList<Seed*> mSeeds;
};



#endif // FACTORY_H
