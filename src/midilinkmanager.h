#ifndef MIDILINKMANAGER_H
#define MIDILINKMANAGER_H



#include "parameters/number.h"
#include "midisignals.h"

#include <QObject>
#include <QMap>
#include <QMultiMap>
#include <QUuid>
#include <QString>



class MidiLinkManager : public QObject
{
    Q_OBJECT

public:
    explicit MidiLinkManager(QObject *parent = nullptr);

    bool enabled();

    QMap<QString, QMultiMap<int, Number<float>*>> floatLinks();
    QMap<QString, QMultiMap<int, Number<int>*>> intLinks();
    QMap<QString, QMultiMap<int, Number<unsigned int>*>> uintLinks();

signals:
    void midiEnabled(bool enabled);
    void multiLinkSet(bool set);

public slots:
    void addMidiSignals(QUuid id, MidiSignals* midiSignals);
    void removeMidiSignals(QUuid id);

    void setupMidi(QString portName, bool open);

    void updateMidiLinks(QString portName, int key, int value);

    void setupMidiLink(QString portName, int key, Number<float>* number);
    void setupMidiLink(QString portName, int key, Number<int>* number);
    void setupMidiLink(QString portName, int key, Number<unsigned int>* number);

    void clearLinks();

    bool multiLink();
    void setMultiLink(bool enabled);

private:
    QMap<QUuid, MidiSignals*> mMidiSignalsMap;

    QMap<QString, QMultiMap<int, Number<float>*>> mFloatLinks;
    QMap<QString, QMultiMap<int, Number<int>*>> mIntLinks;
    QMap<QString, QMultiMap<int, Number<unsigned int>*>> mUintLinks;

    Number<float>* mLinkingFloat = nullptr;
    Number<int>* mLinkingInt = nullptr;
    Number<unsigned int>* mLinkingUint = nullptr;

    QMap<QString, bool> mPortOpen;

    bool mMultiLink = false;

    void removeKey(int key);

    void setUpConnections(bool midiOn);

    void connectMidiSignals(QUuid id);
    void disconnectMidiSignals(QUuid id);
};



#endif // MIDILINKMANAGER_H
