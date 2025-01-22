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

public slots:
    void openPort(int portId, bool open);

private:
    libremidi::observer observer;
    std::vector<libremidi::midi_in> midiInputs;
};



#endif // MIDICONTROL_H
