#include "recorder.h"

#include <QUrl>
#include <QVideoFrame>

Recorder::Recorder(QString filename, qreal framesPerSecond, QMediaFormat::VideoCodec codec, QMediaFormat::FileFormat fileFormat) : fps { framesPerSecond }
{
    recorder.setOutputLocation(QUrl::fromLocalFile(filename));
    recorder.setQuality(QMediaRecorder::VeryHighQuality);
    recorder.setEncodingMode(QMediaRecorder::ConstantQualityEncoding);
    recorder.setVideoFrameRate(framesPerSecond);

    QMediaFormat format;
    format.setVideoCodec(codec);
    format.setFileFormat(fileFormat);
    recorder.setMediaFormat(format);

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
    QVideoFrame frame(image);
    frame.setStreamFrameRate(fps);
    frame.setStartTime(static_cast<qint64>(frameNumber * 1000000 / fps));
    frame.setEndTime(static_cast<qint64>((frameNumber + 1) * 1000000 / fps));
    videoInput.sendVideoFrame(frame);
    frameNumber++;
}
