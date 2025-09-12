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



#ifndef FBO_H
#define FBO_H



#include "texformat.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QImage>



class FBO : protected QOpenGLExtraFunctions
{
public:
    static GLuint width;
    static GLuint height;

    static TextureFormat texFormat;

    FBO(QOpenGLContext* shareContext);
    ~FBO();

    GLuint fboId() const;
    GLuint textureId() const;

    //GLuint** getTextureID() { return &texID; }
    //void setInputTextureID(GLuint* id) { inputTextureID = id; }

    void generateFramebuffer(GLuint& framebuffer, GLuint& texture);

    void setMinMagFilter(GLenum filter);
    void setTextureFormat();

    void resize();

    void blit(GLuint fbo);
    void clear();

    QImage outputImage();

private:
    QOpenGLContext* mContext;
    QOffscreenSurface* mSurface;

    GLuint mFbo;
    GLuint mTextureId = 0;

    //GLuint *texID;

    GLenum mMinMagFilter = GL_NEAREST;
    //GLuint* inputTextureID;

    GLuint mSamplerId = 0;

    GLuint widthOld;
    GLuint heightOld;

    GLsync fence = 0;

    GLenum getFormat(GLenum format);
};



#endif // FBO_H
