#ifndef MIDICONTROL_H
#define MIDICONTROL_H


#include <QObject>
#include <QMap>
#include <libremidi/libremidi.hpp>



class MidiControl : public QObject
{
    Q_OBJECT

public:
    explicit MidiControl(QObject *parent = nullptr);
    ~MidiControl();

    void setInputPorts();

signals:
    void inputPortsChanged(QList<QString> portNames);
    void inputPortOpen(QString portName, bool open);
    void ccInputMessageReceived(QString portName, int key, int value);

public slots:
    void openPort(QString portName, bool open);

private:
    libremidi::observer observer;
    QMap<QString, libremidi::midi_in*> midiInputs;
    QMap<QString, libremidi::input_port> midiInputPorts;
};



#endif // MIDICONTROL_H
