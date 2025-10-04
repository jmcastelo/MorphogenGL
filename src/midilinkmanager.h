#ifndef MIDILINKMANAGER_H
#define MIDILINKMANAGER_H



#include "parameters/number.h"
#include "midisignals.h"

#include <QObject>
#include <QMap>
#include <QUuid>
#include <QString>



class MidiLinkManager : public QObject
{
    Q_OBJECT

public:
    explicit MidiLinkManager(QObject *parent = nullptr);

signals:
    void midiEnabled(bool enabled);

public slots:
    void addMidiSignals(QUuid id, MidiSignals* midiSignals);
    void removeMidiSignals(QUuid id);

    void setupMidi(QString portName, bool open);

    void updateMidiLinks(QString portName, int key, int value);

private:
    QMap<QUuid, MidiSignals*> mMidiSignalsMap;

    QMap<QString, QMap<int, Number<float>*>> floatLinks;
    QMap<QString, QMap<int, Number<int>*>> intLinks;

    Number<float>* linkingFloat = nullptr;
    Number<int>* linkingInt = nullptr;

    bool anyMidiPortOpen = false;

    void setUpConnections(bool midiOn);

    void connectMidiSignals(QUuid id);
    void disconnectMidiSignals(QUuid id);
};



#endif // MIDILINKMANAGER_H
