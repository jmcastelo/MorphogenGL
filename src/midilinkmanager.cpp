


#include "midilinkmanager.h"



MidiLinkManager::MidiLinkManager(QObject *parent)
    : QObject{parent}
{}



bool MidiLinkManager::enabled()
{
    bool anyPortOpen = false;

    foreach (bool open, mPortOpen) {
        anyPortOpen |= open;
    }

    return anyPortOpen;
}



QMap<QString, QMap<int, Number<float>*>> MidiLinkManager::floatLinks()
{
    return mFloatLinks;
}



QMap<QString, QMap<int, Number<int>*>> MidiLinkManager::intLinks()
{
    return mIntLinks;
}



QMap<QString, QMap<int, Number<unsigned int>*>> MidiLinkManager::uintLinks()
{
    return mUintLinks;
}



void MidiLinkManager::clearLinks()
{
    mFloatLinks.clear();
    mIntLinks.clear();
}



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
        if (!mFloatLinks.contains(portName))
            mFloatLinks[portName] = QMap<int, Number<float>*>();

        if (!mIntLinks.contains(portName))
            mIntLinks[portName] = QMap<int, Number<int>*>();
    }
    /*else
    {
        mFloatLinks.remove(portName);
        mIntLinks.remove(portName);
    }*/

    mPortOpen[portName] = open;

    setUpConnections(enabled());

    emit midiEnabled(enabled());
}



void MidiLinkManager::updateMidiLinks(QString portName, int key, int value)
{
    if (mLinkingFloat != nullptr)
    {
        // Linking a float number

        setupMidiLink(portName, key, mLinkingFloat);
        mLinkingFloat = nullptr;
    }
    else if (mLinkingInt != nullptr)
    {
        // Linking an int or uint number

        setupMidiLink(portName, key, mLinkingInt);
        mLinkingInt = nullptr;
    }
    else if (mLinkingUint != nullptr)
    {
        // Linking an int or uint number

        setupMidiLink(portName, key, mLinkingUint);
        mLinkingUint = nullptr;
    }
    else
    {
        // No number being linked: set value of already linked number

        if (mFloatLinks.contains(portName) && mFloatLinks[portName].contains(key))
        {
            mFloatLinks[portName][key]->setValueFromIndex(value);
            mFloatLinks[portName][key]->setIndex();
        }
        if (mIntLinks.contains(portName) && mIntLinks[portName].contains(key))
        {
            mIntLinks[portName][key]->setValueFromIndex(value);
            mIntLinks[portName][key]->setIndex();
        }
    }
}



void MidiLinkManager::setupMidiLink(QString portName, int key, Number<float>* number)
{
    // Init if no links map exists for this port

    if (!mFloatLinks.contains(portName)) {
        mFloatLinks[portName] = QMap<int, Number<float>*>();
    }

    // Remove previously assigned key

    if (mFloatLinks[portName].contains(key))
    {
        mFloatLinks[portName][key]->setIndexMax(100'000);
        mFloatLinks[portName][key]->setMidiLinked(false);
        mFloatLinks[portName].remove(key);
    }

    // Adjust to midi range

    number->setIndexMax(127);
    number->setMidiLinked(true);

    // Ensure removable on number deletion

    connect(number, &Number<float>::deleting, this, [=, this]() {
        mFloatLinks[portName].remove(key);
    });

    // Store link

    mFloatLinks[portName][key] = number;
}



void MidiLinkManager::setupMidiLink(QString portName, int key, Number<int>* number)
{
    // Init if no links map exists for this port

    if (!mIntLinks.contains(portName)) {
        mIntLinks[portName] = QMap<int, Number<int>*>();
    }

    // Remove previously assigned key

    if (mIntLinks[portName].contains(key))
    {
        mIntLinks[portName][key]->setIndexMax(100'000);
        mIntLinks[portName][key]->setMidiLinked(false);
        mIntLinks[portName].remove(key);
    }

    // Adjust to midi range

    number->setIndexMax(127);
    number->setMidiLinked(true);

    // Ensure removable on number deletion

    connect(number, &Number<int>::deleting, this, [=, this]() {
        mIntLinks[portName].remove(key);
    });

    // Store link

    mIntLinks[portName][key] = number;
}



void MidiLinkManager::setupMidiLink(QString portName, int key, Number<unsigned int>* number)
{
    // Init if no links map exists for this port

    if (!mUintLinks.contains(portName)) {
        mUintLinks[portName] = QMap<int, Number<unsigned int>*>();
    }

    // Remove previously assigned key

    if (mUintLinks[portName].contains(key))
    {
        mUintLinks[portName][key]->setIndexMax(100'000);
        mUintLinks[portName][key]->setMidiLinked(false);
        mUintLinks[portName].remove(key);
    }

    // Adjust to midi range

    number->setIndexMax(127);
    number->setMidiLinked(true);

    // Ensure removable on number deletion

    connect(number, &Number<unsigned int>::deleting, this, [=, this]() {
        mUintLinks[portName].remove(key);
    });

    // Store link

    mUintLinks[portName][key] = number;
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
        mLinkingFloat = number;
    });

    connect(mMidiSignalsMap.value(id), QOverload<Number<int>*>::of(&MidiSignals::linkWait), this, [=, this](Number<int>* number) {
        mLinkingInt = number;
    });

    connect(mMidiSignalsMap.value(id), QOverload<Number<unsigned int>*>::of(&MidiSignals::linkWait), this, [=, this](Number<unsigned int>* number) {
        mLinkingUint = number;
    });

    connect(mMidiSignalsMap.value(id), QOverload<Number<float>*>::of(&MidiSignals::linkBreak), this, [=, this](Number<float>* number) {
        for (auto [port, map] : mFloatLinks.asKeyValueRange())
        {
            for (auto [key, n] : map.asKeyValueRange())
            {
                if (n == number)
                {
                    number->setMidiLinked(false);
                    mFloatLinks[port].remove(key);
                    break;
                }
            }
        }
    });

    connect(mMidiSignalsMap.value(id), QOverload<Number<int>*>::of(&MidiSignals::linkBreak), this, [=, this](Number<int>* number) {
        for (auto [port, map] : mIntLinks.asKeyValueRange())
        {
            for (auto [key, n] : map.asKeyValueRange())
            {
                if (n == number)
                {
                    number->setMidiLinked(false);
                    mIntLinks[port].remove(key);
                    break;
                }
            }
        }
    });

    connect(mMidiSignalsMap.value(id), QOverload<Number<unsigned int>*>::of(&MidiSignals::linkBreak), this, [=, this](Number<unsigned int>* number) {
        for (auto [port, map] : mUintLinks.asKeyValueRange())
        {
            for (auto [key, n] : map.asKeyValueRange())
            {
                if (n == number)
                {
                    number->setMidiLinked(false);
                    mUintLinks[port].remove(key);
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
    disconnect(mMidiSignalsMap.value(id), QOverload<Number<unsigned int>*>::of(&MidiSignals::linkWait), this, nullptr);

    disconnect(mMidiSignalsMap.value(id), QOverload<Number<float>*>::of(&MidiSignals::linkBreak), this, nullptr);
    disconnect(mMidiSignalsMap.value(id), QOverload<Number<int>*>::of(&MidiSignals::linkBreak), this, nullptr);
    disconnect(mMidiSignalsMap.value(id), QOverload<Number<unsigned int>*>::of(&MidiSignals::linkBreak), this, nullptr);
}
