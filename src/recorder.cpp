#include "recorder.h"

#include <QUrl>
#include <QMediaFormat>
#include <QVideoFrame>

Recorder::Recorder(QString filename, qreal framesPerSecond) : fps { framesPerSecond }
{
    recorder.setOutputLocation(QUrl::fromLocalFile(filename));
    recorder.setQuality(QMediaRecorder::VeryHighQuality);
    recorder.setEncodingMode(QMediaRecorder::ConstantQualityEncoding);
    recorder.setMediaFormat(QMediaFormat::AVI);
    recorder.setVideoFrameRate(framesPerSecond);

    session.setRecorder(&recorder);
    session.setVideoFrameInput(&videoInput);

    recorder.record();
}



Recorder::~Recorder()
{
    recorder.stop();
}



void Recorder::sendVideoFrame(const QImage &image)
{
    QVideoFrame frame(std::move(image));
    frame.setStreamFrameRate(fps);
    frame.setStartTime(static_cast<qint64>(frameNumber * 1000000 / fps));
    frame.setEndTime(static_cast<qint64>((frameNumber + 1) * 1000000 / fps));
    videoInput.sendVideoFrame(std::move(frame));
    frameNumber++;
}
