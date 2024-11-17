#include "timerthread.h"



TimerThread::TimerThread(double fps, QObject *parent) : QThread(parent)
{
    timer.setTimerType(Qt::PreciseTimer);
    //timer.setSingleShot(true);
    setTimerInterval(fps);

    timer.start();
}


void TimerThread::setTimerInterval(double fps)
{
    QMutexLocker locker(&mutex);
    std::chrono::nanoseconds nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(fps > 0 ? 1.0 / fps : 0));
    timer.setInterval(nanos);
}



void TimerThread::stop()
{
    timer.stop();
    quit();
    wait();
}



void TimerThread::run()
{
    connect(&timer, &QChronoTimer::timeout, this, &TimerThread::timeout);
    exec();
}
