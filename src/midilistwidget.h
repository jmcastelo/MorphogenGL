#ifndef MIDILISTWIDGET_H
#define MIDILISTWIDGET_H



#include <QWidget>
#include <QListWidget>
#include <QTableWidgetItem>
#include <QPushButton>



class MidiListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MidiListWidget(QWidget *parent = nullptr);

signals:
    void portSelected(QString portName, bool state);
    void multiLinkButtonChecked(bool checked);
    void clearLinksButtonClicked();

public slots:
    void populatePortsTable(QList<QString> portNames);
    void portChecked(QListWidgetItem* item);
    void checkPort(QString portName);
    void toggleMultiLinkButton(bool checked);

private:
    QListWidget* portsTable;
    QPushButton* multiLinkButton;
};



#endif // MIDILISTWIDGET_H
