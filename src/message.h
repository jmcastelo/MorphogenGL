#ifndef MESSAGE_H
#define MESSAGE_H



#include <QObject>
#include <QPainter>
#include <QTimer>
#include <QMap>
#include <QUuid>



class Message : public QObject
{
    Q_OBJECT

public:
    Message(QString op, QString parameter, QString value, QObject *parent = nullptr);
    ~Message();

    void setValue(QString parameter, QString value);
    void setText();
    void setTimer(QString parameter);

    QRectF getFrame(){ return frame; }
    void moveFrameTo(QPointF point);

    void paint(QPainter *painter);

signals:
    void expired(Message* msg);

private:
    QString operation;
    QMap<QString, QString> values;
    QString text;
    QList<QString> textLines;

    QMap<QString, QTimer*> timers;
    int durationMSecs = 3000;

    QTimer fadeTimer;
    int intervalMSecs = 10;
    qreal fadeDurationMSecs = 100;
    bool fadeIn = true;

    QRectF frame;
    QFont textFont;
    qreal frameMargin = 10.0;
    qreal opacity = 0.0;

    qreal fontPointSize = 10;

    void setFrame();

private slots:
    void fade();
};


#endif // MESSAGE_H
