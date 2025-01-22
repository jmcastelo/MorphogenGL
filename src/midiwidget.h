#ifndef MIDIWIDGET_H
#define MIDIWIDGET_H



#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>



class MidiWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MidiWidget(QWidget *parent = nullptr);

public slots:
    void populatePortsTable(std::vector<std::string> portNames);

private:
    QTableWidget* portsTable;
};

#endif // MIDIWIDGET_H
