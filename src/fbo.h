/*
*  Copyright 2021 Jose Maria Castelo Ares
*
*  Contact: <jose.maria.castelo@gmail.com>
*  Repository: <https://github.com/jmcastelo/MorphogenGL>
*
*  This file is part of MorphogenGL.
*
*  MorphogenGL is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  MorphogenGL is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with MorphogenGL.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "texformat.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QImage>
#include <QObject>

class MorphoWidget;

class FBO : public QObject, protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    static GLuint width;
    static GLuint height;

    static TextureFormat texFormat;

    QOpenGLShaderProgram* program;
    
    //FBO(QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext);
    FBO(QOpenGLContext* mainContext);
    virtual ~FBO();

    void setShadersFromSourceCode(QString vertexShader, QString fragmentShader);
    void setShadersFromSourceFile(QString vertexShader, QString fragmentShader);

    GLuint getFBO() { return fboBlit; }
    GLuint** getTextureID() { return &texID; }
    GLuint** getTextureBlit() { return &texBlit; }
    void setInputTextureID(GLuint* id) { inputTextureID = id; }

    void generateFramebuffer(GLuint& framebuffer, GLuint& texture);

    void setMinMagFilter(GLenum filter);
    void setTextureFormat();

    void makeCurrent();
    void doneCurrent();

    virtual void resize();

    void draw();
    void blit();
    void identity();
    void clear();

    void setOrthographic(QString name);
    void adjustOrtho();

    QImage outputImage();

signals:
    void sizeChanged();

protected:
    QOpenGLContext* context;
    QOffscreenSurface* surface;
    GLuint fbo;
    GLuint *texID;
    GLuint textureID = 0;
    GLenum minMagFilter = GL_NEAREST;
    GLuint* inputTextureID;
    GLuint samplerID = 0;
    GLuint fboBlit;
    GLuint *texBlit;
    GLuint textureBlit;
    QOpenGLVertexArrayObject* vao;
    QOpenGLBuffer* vboPos;
    QOpenGLBuffer* vboTex;
    QOpenGLShaderProgram* identityProgram;
    GLuint widthOld;
    GLuint heightOld;
    GLsync fence = 0;

    bool mOrthoEnabled = false;
    QString mOrthoName;

private:
    void resizeVertices();
    GLenum getFormat(GLenum format);
};
