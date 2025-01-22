#ifndef MIDICONTROL_H
#define MIDICONTROL_H


#include <QObject>
#include <libremidi/libremidi.hpp>



class MidiControl : public QObject
{
    Q_OBJECT

public:
    explicit MidiControl(QObject *parent = nullptr);

    void setInputPorts();

signals:
    void inputPortsChanged(std::vector<std::string> portNames);

private:
    libremidi::observer observer;
};



#endif // MIDICONTROL_H
