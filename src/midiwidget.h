#ifndef MIDIWIDGET_H
#define MIDIWIDGET_H



#include <QWidget>
#include <QListWidget>
#include <QTableWidgetItem>



class MidiWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MidiWidget(QWidget *parent = nullptr);

signals:
    void portSelected(int port, bool state);

public slots:
    void populatePortsTable(QList<QString> portNames);

private:
    QListWidget* portsTable;
};

#endif // MIDIWIDGET_H
