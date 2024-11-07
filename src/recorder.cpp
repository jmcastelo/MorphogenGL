#include "recorder.h"

#include <QUrl>
#include <QVideoFrame>
#include <QDebug>



Recorder::Recorder(QString filename, qreal framesPerSecond, QMediaFormat format) : fps { framesPerSecond }
{
    recorder.setOutputLocation(QUrl::fromLocalFile(filename));
    recorder.setQuality(QMediaRecorder::VeryHighQuality);
    recorder.setEncodingMode(QMediaRecorder::ConstantQualityEncoding);
    recorder.setVideoFrameRate(framesPerSecond);
    recorder.setVideoResolution(QSize());
    recorder.setMediaFormat(format);

    session.setRecorder(&recorder);
    session.setVideoFrameInput(&videoInput);

    connect(&videoInput, &QVideoFrameInput::readyToSendVideoFrame, this, &Recorder::setVideoFrameInputReady);

    //recorder.record();
}



Recorder::~Recorder()
{
    //recorder.stop();
}



void Recorder::startRecording()
{
    recorder.record();
}



void Recorder::stopRecording()
{
    recorder.stop();
}



void Recorder::setVideoFrameInputReady()
{
    videoFrameInputReady = true;
    //qDebug() << "Ready";
}



void Recorder::sendVideoFrame(const QImage &image)
{
    QVideoFrame frame(image);

    frame.setStreamFrameRate(fps);
    qint64 start = static_cast<qint64>(frameNumber * 1000000 / fps);
    qint64 end = static_cast<qint64>((frameNumber + 1) * 1000000 / fps);
    frame.setStartTime(start);
    frame.setEndTime(end);

    bool sent = videoInput.sendVideoFrame(frame);
    if (sent)
    {
        frameNumber++;
        emit frameRecorded(frameNumber);
        //qDebug() << frameNumber;
    }
    else
    {
        videoFrameInputReady = false;
        //qDebug() << "Busy";
    }
}
