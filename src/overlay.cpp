#include "overlay.h"



Overlay::Overlay(QObject *parent)
    : QObject{parent}
{}



Overlay::~Overlay()
{
    qDeleteAll(messages);
    messages.clear();
}



void Overlay::addMessage(QUuid id, QString operationName, QString parameterName, QString value)
{
    if (messages.contains(id))
        messages.value(id)->setValue(parameterName, value);
    else
    {
        Message *message = new Message(operationName, parameterName, value);
        connect(message, &Message::expired, this, &Overlay::removeMessage);
        messages.insert(id, message);
    }

    setMessagesFrames();
}



void Overlay::removeMessage(Message *message)
{
    disconnect(message);

    QUuid id = messages.key(message);
    message->deleteLater();
    messages.remove(id);

    setMessagesFrames();
}



void Overlay::setMessagesFrames()
{
    qreal posY = margin;

    for (auto [id, message] : messages.asKeyValueRange())
    {
        message->moveFrameTo(QPointF(0.0, posY));
        posY += message->getFrame().height() + margin;
    }

    qreal totalHeight = 0.0;

    for (auto [id, message] : messages.asKeyValueRange())
        totalHeight += message->getFrame().height();

    totalHeight += (messages.size() + 1) * margin;

    qreal halfHeight = totalHeight * 0.5;

    if (totalHeight > maxHeight)
        maxHeight = totalHeight;

    qreal posY0 = maxHeight * 0.5 - halfHeight;

    windowRect.setRect(0, -posY0, maxHeight, maxHeight);
}



void Overlay::paint(QPainter *painter)
{
    painter->setWindow(windowRect);
    painter->setViewport(viewportRect);

    for (auto [id, message] : messages.asKeyValueRange())
        message->paint(painter);
}



void Overlay::setViewportRect(int w, int h)
{
    Q_UNUSED(w)
    viewportRect.setRect(0, 0, h, h);
}
