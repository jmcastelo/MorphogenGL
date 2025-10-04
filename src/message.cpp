#include "message.h"



Message:: Message(QString op, QString parameter, QString value, QObject *parent)
    : QObject{parent},
    operation { op }
{
    textFont.setFamily("Monospace");
    textFont.setPointSizeF(fontPointSize);

    setValue(parameter, value);

    fadeTimer.setInterval(intervalMSecs);
    connect(&fadeTimer, &QTimer::timeout, this, &Message::fade);
    fadeTimer.start();
}



Message::~Message()
{
    qDeleteAll(timers);
    timers.clear();
}



void Message::fade()
{
    if (fadeIn)
    {
        opacity += intervalMSecs / fadeDurationMSecs;
        if (opacity >= 1.0)
        {
            fadeTimer.stop();
            opacity = 1.0;
        }
    }
    else
    {
        if (values.empty())
        {
            opacity -= intervalMSecs / fadeDurationMSecs;
            if (opacity <= 0.0)
            {
                fadeTimer.stop();
                opacity = 0.0;
                emit expired(this);
            }
        }
        else
        {
            fadeDurationMSecs = 100;
            fadeIn = true;
        }
    }
}



void Message::setValue(QString parameter, QString value)
{
    if (values.contains(parameter)) {
        values[parameter] = value;
    }
    else {
        values.insert(parameter, value);
    }

    setText();
    setFrame();
    setTimer(parameter);
}



void Message::setText()
{
    textLines.clear();
    textLines.append(operation);
    for (auto it = values.keyValueBegin(); it != values.keyValueEnd(); ++it) {
        textLines.append(" " + it->first + ": " + it->second);
    }

    text.clear();
    foreach (QString line, textLines)
        text += line + '\n';
}



void Message::setTimer(QString parameter)
{
    if (!timers.contains(parameter))
    {
        QTimer* timer = new QTimer(this);
        timer->setSingleShot(true);

        timers.insert(parameter, timer);

        connect(timer, &QTimer::timeout, this, [=, this]()
        {
            values.remove(parameter);
            timers.remove(parameter);

            if (values.empty())
            {
                fadeIn = false;
                fadeDurationMSecs = 1000;
                fadeTimer.start();
            }
        });
    }

    timers.value(parameter)->start(durationMSecs);
}



void Message::setFrame()
{
    QFontMetricsF fontMetrics(textFont);

    qreal frameHeight = 0.0;
    qreal frameWidth = 0.0;

    foreach (QString line, textLines)
    {
        QRectF bRect = fontMetrics.boundingRect(line);

        frameHeight += bRect.height();

        if (bRect.width() > frameWidth)
            frameWidth = bRect.width();
    }

    frame.setSize(QSizeF(frameWidth, frameHeight));
    frame = frame.marginsAdded(QMarginsF(frameMargin, frameMargin, frameMargin, frameMargin));
}



void Message::moveFrameTo(QPointF point)
{
    frame.moveTopLeft(point);
}


void Message::paint(QPainter *painter)
{
    QColor startColor = QColor(64, 32, 64);
    startColor.setAlphaF(opacity);

    QColor endColor = QColor(64, 32, 64);
    endColor.setAlphaF(0.25 * opacity);

    QLinearGradient gradient(frame.topLeft(), frame.bottomRight());
    gradient.setColorAt(0, startColor);
    gradient.setColorAt(1, endColor);

    painter->setBrush(gradient);
    painter->setPen(QPen(gradient, 0));
    painter->drawRect(frame);

    QColor textColor = QColor(255, 255, 255);
    textColor.setAlphaF(opacity);

    painter->setPen(QPen(textColor));
    painter->setFont(textFont);
    painter->drawText(frame.marginsAdded(QMarginsF(-frameMargin, -frameMargin, -frameMargin, -frameMargin)), Qt::AlignVCenter, text);
}
