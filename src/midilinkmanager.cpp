


#include "midilinkmanager.h"



MidiLinkManager::MidiLinkManager(QObject *parent)
    : QObject{parent}
{}



void MidiLinkManager::addMidiSignals(QUuid id, MidiSignals* midiSignals)
{
    mMidiSignalsMap.insert(id, midiSignals);
}



void MidiLinkManager::removeMidiSignals(QUuid id)
{
    mMidiSignalsMap.remove(id);
}



void MidiLinkManager::setupMidi(QString portName, bool open)
{
    if (open)
    {
        if (!floatLinks.contains(portName))
            floatLinks[portName] = QMap<int, Number<float>*>();

        if (!intLinks.contains(portName))
            intLinks[portName] = QMap<int, Number<int>*>();
    }
    else
    {
        floatLinks.remove(portName);
        intLinks.remove(portName);
    }

    anyMidiPortOpen = !floatLinks.isEmpty() || !intLinks.isEmpty();

    setUpConnections(anyMidiPortOpen);

    emit midiEnabled(anyMidiPortOpen);
}



void MidiLinkManager::updateMidiLinks(QString portName, int key, int value)
{
    if (linkingFloat != nullptr)
    {
        if (!floatLinks.contains(portName))
            floatLinks[portName] = QMap<int, Number<float>*>();

        if (floatLinks[portName].contains(key))
        {
            floatLinks[portName][key]->setIndexMax(100'000);
            floatLinks[portName][key]->setMidiLinked(false);
            floatLinks[portName].remove(key);
        }

        linkingFloat->setIndexMax(127);
        linkingFloat->setMidiLinked(true);

        connect(linkingFloat, &Number<float>::deleting, this, [=, this]() {
            floatLinks[portName].remove(key);
        });

        floatLinks[portName][key] = linkingFloat;
        linkingFloat = nullptr;
    }
    else if (linkingInt != nullptr)
    {
        if (!intLinks.contains(portName))
            intLinks[portName] = QMap<int, Number<int>*>();

        if (intLinks[portName].contains(key))
        {
            intLinks[portName][key]->setIndexMax(100'000);
            intLinks[portName][key]->setMidiLinked(false);
            intLinks[portName].remove(key);
        }

        linkingInt->setIndexMax(127);
        linkingInt->setMidiLinked(true);

        connect(linkingInt, &Number<int>::deleting, this, [=, this]() {
            intLinks[portName].remove(key);
        });

        intLinks[portName][key] = linkingInt;
        linkingInt = nullptr;
    }
    else
    {
        if (floatLinks.contains(portName) && floatLinks[portName].contains(key))
        {
            floatLinks[portName][key]->setValueFromIndex(value);
            floatLinks[portName][key]->setIndex();
        }
        if (intLinks.contains(portName) && intLinks[portName].contains(key))
        {
            intLinks[portName][key]->setValueFromIndex(value);
            intLinks[portName][key]->setIndex();
        }
    }
}



void MidiLinkManager::setUpConnections(bool midiOn)
{
    for (auto [id, widget] : mMidiSignalsMap.asKeyValueRange())
    {
        if (midiOn)
            connectMidiSignals(id);
        else
            disconnectMidiSignals(id);
    }
}



void MidiLinkManager::connectMidiSignals(QUuid id)
{
    connect(mMidiSignalsMap.value(id), QOverload<Number<float>*>::of(&MidiSignals::linkWait), this, [=, this](Number<float>* number) {
        linkingFloat = number;
    });

    connect(mMidiSignalsMap.value(id), QOverload<Number<int>*>::of(&MidiSignals::linkWait), this, [=, this](Number<int>* number) {
        linkingInt = number;
    });

    connect(mMidiSignalsMap.value(id), QOverload<Number<float>*>::of(&MidiSignals::linkBreak), this, [=, this](Number<float>* number) {
        for (auto [port, map] : floatLinks.asKeyValueRange())
        {
            for (auto [key, n] : map.asKeyValueRange())
            {
                if (n == number)
                {
                    number->setMidiLinked(false);
                    floatLinks[port].remove(key);
                    break;
                }
            }
        }
    });

    connect(mMidiSignalsMap.value(id), QOverload<Number<int>*>::of(&MidiSignals::linkBreak), this, [=, this](Number<int>* number) {
        for (auto [port, map] : intLinks.asKeyValueRange())
        {
            for (auto [key, n] : map.asKeyValueRange())
            {
                if (n == number)
                {
                    number->setMidiLinked(false);
                    intLinks[port].remove(key);
                    break;
                }
            }
        }
    });
}



void MidiLinkManager::disconnectMidiSignals(QUuid id)
{
    disconnect(mMidiSignalsMap.value(id), QOverload<Number<float>*>::of(&MidiSignals::linkWait), this, nullptr);
    disconnect(mMidiSignalsMap.value(id), QOverload<Number<int>*>::of(&MidiSignals::linkWait), this, nullptr);

    disconnect(mMidiSignalsMap.value(id), QOverload<Number<float>*>::of(&MidiSignals::linkBreak), this, nullptr);
    disconnect(mMidiSignalsMap.value(id), QOverload<Number<int>*>::of(&MidiSignals::linkBreak), this, nullptr);
}
