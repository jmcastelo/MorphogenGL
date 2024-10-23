#ifndef RECORDER_H
#define RECORDER_H

#include <QVideoFrameInput>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>

class Recorder : protected QOpenGLExtraFunctions
{
public:
    unsigned int frameNumber = 0;

    Recorder(QString filename, int width, int height, int fps, QOpenGLContext* mainContext, GLuint id);
    ~Recorder();

    void recordFrame();
    void setTextureID(GLuint id) { textureID = id; }

private:
    QVideoFrameInput videoInput;
    QMediaCaptureSession session;
    QMediaRecorder recorder;
    int width;
    int height;
    QOpenGLContext* context;
    GLuint textureID = 0;
};

#endif // RECORDER_H
