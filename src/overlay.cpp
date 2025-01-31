#include "overlay.h"



Overlay::Overlay(QObject *parent)
    : QObject{parent}
{}



void Overlay::addMessage(QUuid id, QString operationName, QString parameterName, QString value)
{
    if (messages.contains(id))
        messages[id]->setValue(parameterName, value);
    else
        messages.insert(id, new Message(operationName, parameterName, value));
}



void Overlay::updateMessages()
{
    for (auto it = messages.begin(); it != messages.end();)
        if (it.value()->expired())
            messages.erase(it++);
        else
            ++it;
}



void Overlay::paint(QPainter *painter)
{
    for (auto [id, message] : messages.asKeyValueRange())
        message->paint(painter);
}
