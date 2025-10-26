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
    foreach (auto midiInput, midiInputs)
        delete midiInput;
}



void MidiControl::setInputPorts()
{
    midiInputs.clear();

    QList<QString> portNames;

    for (const libremidi::input_port& port : observer.get_input_ports())
    {
        portNames.push_back(QString::fromStdString(port.port_name));

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

        midiInputs.append(midiIn);
    }

    emit inputPortsChanged(portNames);
}



void MidiControl::openPort(int portId, bool open)
{
    auto inputPorts = observer.get_input_ports();

    if (open) {
        midiInputs[portId]->open_port(inputPorts[portId]);
    }
    else {
        midiInputs[portId]->close_port();
    }

    emit inputPortOpen(QString::fromStdString(inputPorts[portId].port_name), midiInputs[portId]->is_port_open());
}
