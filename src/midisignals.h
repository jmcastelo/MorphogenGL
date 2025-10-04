#ifndef MIDISIGNALS_H
#define MIDISIGNALS_H



#include "parameters/number.h"

#include <QObject>



class MidiSignals : public QObject
{
    Q_OBJECT

public:
    explicit MidiSignals(QObject *parent = nullptr) : QObject(parent) {}

signals:
    void linkWait(Number<float>* number);
    void linkWait(Number<int>* number);
    void linkWait(Number<unsigned int>* number);

    void linkBreak(Number<float>* number);
    void linkBreak(Number<int>* number);
    void linkBreak(Number<unsigned int>* number);
};



#endif // MIDISIGNALS_H
