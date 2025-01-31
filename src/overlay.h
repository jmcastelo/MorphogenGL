#ifndef OVERLAY_H
#define OVERLAY_H



#include <QObject>
#include <QPainter>
#include <QDateTime>
#include <QUuid>



class Message
{
public:
    Message( QString op, QString parameter, QString value) :
        operation { op }
    {
        setValue(parameter, value);

        background = QBrush(QColor(64, 32, 64, 64));
        textPen = QPen(Qt::white);
        textFont.setPixelSize(50);

        frame = QRectF();

        setTimestamp(parameter);
    }

    void setValue(QString parameter, QString value)
    {
        if (values.contains(parameter))
            values[parameter] = value;
        else
            values.insert(parameter, value);

        setText();
        setTimestamp(parameter);
    }

    void setText()
    {
        QString list;

        for (auto it = values.keyValueBegin(); it != values.keyValueEnd(); ++it) {
            list += it->first + ": " + it->second;
        }
        text = operation + '\n' + list;
    }

    void setTimestamp(QString parameter)
    {
        timestamps[parameter] = QDateTime::currentMSecsSinceEpoch();
    }

    bool expired()
    {
        for (auto it = timestamps.begin(); it != timestamps.end();)
            if ((QDateTime::currentMSecsSinceEpoch() - it.value() > durationMSecs))
                timestamps.erase(it++);
            else
                ++it;

        return timestamps.empty();
    }

    void paint(QPainter *painter)
    {
        painter->fillRect(frame, background);
        painter->setPen(textPen);
        painter->setFont(textFont);
        painter->drawText(frame, Qt::AlignLeft| Qt::AlignHCenter, text);
    }

private:
    QString operation;
    QMap<QString, QString> values;
    QString text;

    QMap<QString, qint64> timestamps;
    int durationMSecs = 2000;

    QRectF frame;
    QBrush background;
    QFont textFont;
    QPen textPen;
};



class Overlay : public QObject
{
    Q_OBJECT

public:
    explicit Overlay(QObject *parent = nullptr);

    void paint(QPainter *painter);
    void setSize(QSize size_){ size = size_; }
    void addMessage(QUuid id, QString operation, QString parameter, QString value);

public slots:
    void updateMessages();

private:
    QSize size;
    QMap<QUuid, Message*> messages;
};



#endif // OVERLAY_H
