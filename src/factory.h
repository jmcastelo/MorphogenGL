#ifndef FACTORY_H
#define FACTORY_H



#include "imageoperation.h"

#include <QObject>
#include <QOpenGLContext>
#include <QOffscreenSurface>


class Factory : public QObject
{
    Q_OBJECT

public:
    explicit Factory(QObject *parent = nullptr);

    /*void init();
    void setContext(QOpenGLContext* context);
    void setSurface(QOffscreenSurface* surface);

    void setTextureFormat(GLenum format);

signals:
    void newOperationCreated(ImageOperation* operation);

public slots:
    void resize(GLuint width, GLuint height);

    void createNewOperation();

private:
    QOpenGLContext* mContext;
    QOffscreenSurface* mSurface;

    GLuint mTexWidth;
    GLuint mTexHeight;

    TextureFormat mTexFormat;*/
};



#endif // FACTORY_H
