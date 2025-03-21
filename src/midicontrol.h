#ifndef MIDICONTROL_H
#define MIDICONTROL_H


#include <QObject>
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
    void openPort(int portId, bool open);

private:
    libremidi::observer observer;
    QList<libremidi::midi_in*> midiInputs;
};



#endif // MIDICONTROL_H
