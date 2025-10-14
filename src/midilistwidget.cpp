


#include "midilistwidget.h"

#include <QListWidgetItem>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>



MidiListWidget::MidiListWidget(QWidget *parent): QWidget{parent}
{
    QPushButton* multiLinkButton = new QPushButton("Multi-assign");
    multiLinkButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    multiLinkButton->setCheckable(true);

    QPushButton* clearLinksButton = new QPushButton("Clear assignments");
    clearLinksButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    portsTable = new QListWidget();

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addWidget(multiLinkButton);
    hLayout->addWidget(clearLinksButton);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addLayout(hLayout);
    layout->addWidget(portsTable);

    setLayout(layout);

    setWindowTitle("MIDI Controller");

    connect(clearLinksButton, &QPushButton::clicked, this, &MidiListWidget::clearLinksButtonClicked);
    connect(multiLinkButton, &QPushButton::clicked, this, &MidiListWidget::multiLinkButtonChecked);
}



void MidiListWidget::populatePortsTable(QList<QString> portNames)
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
