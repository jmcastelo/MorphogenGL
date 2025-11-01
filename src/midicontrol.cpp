#include "midicontrol.h"
#include <QDebug>



MidiControl::MidiControl(QObject *parent) : QObject(parent)
{
    libremidi::observer_configuration config
    {
        .input_added = [&] (const libremidi::input_port& port) {
            Q_UNUSED(port)
            setInputPorts();
        },
        .input_removed = [&] (const libremidi::input_port& port) {
            emit inputPortOpen(QString::fromStdString(port.port_name), false);
            setInputPorts();
        }
    };

    observer = libremidi::observer{std::move(config)};
}



MidiControl::~MidiControl()
{
    foreach (auto midiInput, midiInputs) {
        delete midiInput;
    }
}



void MidiControl::setInputPorts()
{
    qDeleteAll(midiInputs);
    midiInputs.clear();

    midiInputPorts.clear();

    QList<QString> portNames;

    for (const libremidi::input_port& port : observer.get_input_ports())
    {
        QString portName = QString::fromStdString(port.port_name);
        portNames.push_back(portName);

        libremidi::input_configuration config {
            .on_message = [=, this](const libremidi::message& message) {
                if (message.get_message_type() == libremidi::message_type::CONTROL_CHANGE)
                {
                    // MIDI CC message bytes:
                    // message[0] = channel, ranges from 176 (channel 1) to 191 (channel 16)
                    // message[1] = controller, ranges from 0 to 127 -> knob or fader
                    // message[2] = value, ranges from 0 to 127

                    int key = message[0] * 128 + message[1];
                    emit ccInputMessageReceived(QString::fromStdString(port.port_name), key, message[2]);
                }
            }
        };

        libremidi::midi_in* midiIn = new libremidi::midi_in { std::move(config) };

        midiInputs.insert(portName, midiIn);
        midiInputPorts.insert(portName, port);
    }

    emit inputPortsChanged(portNames);
}



void MidiControl::openPort(QString portName, bool open)
{
    if (midiInputs.contains(portName))
    {
        if (open) {
            midiInputs[portName]->open_port(midiInputPorts[portName]);
        }
        else {
            midiInputs[portName]->close_port();
        }

        emit inputPortOpen(portName, midiInputs[portName]->is_port_open());
    }
}
