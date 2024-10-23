#ifndef RECORDER_H
#define RECORDER_H

#include <QVideoFrameInput>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <QImage>

class Recorder : protected QOpenGLExtraFunctions
{
public:
    unsigned int frameNumber = 0;
    qreal fps;
    QVideoFrameInput videoInput;

    Recorder(QString filename, qreal framesPerSecond);
    ~Recorder();

    void sendVideoFrame(const QImage &image);

private:
    QMediaCaptureSession session;
    QMediaRecorder recorder;
};

#endif // RECORDER_H
