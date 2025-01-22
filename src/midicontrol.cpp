#include "midicontrol.h"



MidiControl::MidiControl(QObject *parent) : QObject(parent)
{
    libremidi::observer_configuration config
    {
        .input_added = [&] (const libremidi::input_port& port)
        {
            Q_UNUSED(port)
            setInputPorts();
        },
        .input_removed = [&] (const libremidi::input_port& port)
        {
            Q_UNUSED(port)
            setInputPorts();
        },
        .output_added = [&] (const libremidi::output_port& port)
        {
            Q_UNUSED(port)
        },
        .output_removed = [&] (const libremidi::output_port& port)
        {
            Q_UNUSED(port)
        }
    };

    observer = libremidi::observer{std::move(config)};
}



void MidiControl::setInputPorts()
{
    std::vector<std::string> portNames;

    for (const libremidi::input_port& port : observer.get_input_ports())
        portNames.push_back(port.port_name);

    emit inputPortsChanged(portNames);
}
