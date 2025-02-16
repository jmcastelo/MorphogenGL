#include "midiwidget.h"
#include <QListWidgetItem>
#include <QCheckBox>
#include <QVBoxLayout>



MidiWidget::MidiWidget(QWidget *parent): QWidget{parent}
{
    portsTable = new QListWidget();

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(portsTable);

    setLayout(layout);

    setWindowTitle("MIDI Controller");
}



void MidiWidget::populatePortsTable(QList<QString> portNames)
{
    portsTable->clear();

    int portId = 0;

    foreach (QString name, portNames)
    {
        QListWidgetItem* item = new QListWidgetItem("", portsTable);
        QCheckBox* checkBox = new QCheckBox(name);
        portsTable->setItemWidget(item, checkBox);
        connect(checkBox, &QCheckBox::checkStateChanged, this, [=, this](Qt::CheckState state){
            emit portSelected(portId, state == Qt::Checked);
        });
        portId++;
    }
}
