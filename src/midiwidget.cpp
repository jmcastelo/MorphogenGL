#include "midiwidget.h"
#include <QVBoxLayout>



MidiWidget::MidiWidget(QWidget *parent): QWidget{parent}
{
    portsTable = new QTableWidget();

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(portsTable);

    setLayout(layout);
}



void MidiWidget::populatePortsTable(std::vector<std::string> portNames)
{
    portsTable->clearContents();

    portsTable->setRowCount(portNames.size());
    portsTable->setColumnCount(1);

    int row = 0;

    for (std::string name : portNames)
    {
        QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(name));
        portsTable->setItem(row++, 0, item);
    }
}
