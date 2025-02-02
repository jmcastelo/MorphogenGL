#ifndef OVERLAY_H
#define OVERLAY_H



#include "message.h"

#include <QObject>
#include <QPainter>
#include <QUuid>



class Overlay : public QObject
{
    Q_OBJECT

public:
    explicit Overlay(QObject *parent = nullptr);
    ~Overlay();

    bool isEnabled(){ return enabled; }

    void paint(QPainter *painter);
    void setViewportRect(int w, int h);
    void addMessage(QUuid id, QString operation, QString parameter, QString value);

public slots:
    void enable(bool set){ enabled = set; }

private:
    bool enabled = false;

    QRect viewportRect;
    QRect windowRect;
    int maxHeight = 1000;
    qreal margin = 10.0;

    QMap<QUuid, Message*> messages;

    void setMessagesFrames();

private slots:
    void removeMessage(Message *message);
};



#endif // OVERLAY_H
