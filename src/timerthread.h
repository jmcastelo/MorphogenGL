#ifndef TIMERTHREAD_H
#define TIMERTHREAD_H



#include <QThread>
#include <QMutex>
#include <QChronoTimer>



class TimerThread : public QThread
{
    Q_OBJECT

public:
    TimerThread(double fps, QObject* parent = nullptr);

    void run() override;
    void stop();

    void setTimerInterval(double fps);

signals:
    void timeout();

private:
    QChronoTimer timer;
    QMutex mutex;
};

#endif // TIMERTHREAD_H
