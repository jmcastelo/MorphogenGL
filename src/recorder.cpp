#include "recorder.h"

#include <QUrl>

Recorder::Recorder(QString filename, int theWidth, int theHeight, int fps, QOpenGLContext* mainContext, GLuint id) :
    width { theWidth },
    height { theHeight },
    context { mainContext },
    textureID { id }
{
    context->makeCurrent(context->surface());
    initializeOpenGLFunctions();
    context->doneCurrent();

    recorder.setOutputLocation(QUrl::fromLocalFile(filename));
    recorder.setQuality(QMediaRecorder::VeryHighQuality);

    session.setRecorder(&recorder);
    session.setVideoFrameInput(&videoInput);
}
