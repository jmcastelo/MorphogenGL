#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QVideoFrameInput>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QMediaFormat>
#include <QImage>

class Recorder : public QObject
{
    Q_OBJECT

public:
    unsigned int frameNumber = 0;
    qreal fps;
    QVideoFrameInput videoInput;

    Recorder(QString filename, qreal framesPerSecond, QMediaFormat format);
    ~Recorder();

    void startRecording();
    void stopRecording();
    bool isRecording(){ return recorder.recorderState() == QMediaRecorder::RecordingState; }
    void addImage(const QImage &image);

signals:
    void frameRecorded();

private:
    QMediaCaptureSession session;
    QMediaRecorder recorder;
    QList<QImage> images;
    bool videoFrameInputReady = false;
    void sendVideoFrames();

private slots:
    void setVideoFrameInputReady();
};

#endif // RECORDER_H
