#ifndef MIDILISTWIDGET_H
#define MIDILISTWIDGET_H



#include <QWidget>
#include <QListWidget>
#include <QTableWidgetItem>



class MidiListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MidiListWidget(QWidget *parent = nullptr);

signals:
    void portSelected(int port, bool state);
    void multiLinkButtonChecked(bool checked);
    void clearLinksButtonClicked();

public slots:
    void populatePortsTable(QList<QString> portNames);

private:
    QListWidget* portsTable;
};



#endif // MIDILISTWIDGET_H
