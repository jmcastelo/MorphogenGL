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
#include "imageoperation.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QImage>



class FBO : protected QOpenGLExtraFunctions
{
public:
    FBO(QOpenGLContext* shareContext);
    ~FBO();

    //GLuint** getTextureID() { return &texID; }
    //void setInputTextureID(GLuint* id) { inputTextureID = id; }

    ImageOperation* createNewOperation();

    void render();
    void blend();

    void resize();

    void blit(GLuint fbo);
    void clear();

    QImage outputImage();

private:
    QOpenGLContext* mShareContext;
    QOpenGLContext* mContext;
    QOffscreenSurface* mSurface;

    GLuint mOutFbo;

    QList<ImageOperation*> mOperations;
    QList<ImageOperation*> mSortedOperations;

    GLuint mTexWidth = 2048;
    GLuint mTexHeight = 2048;

    GLuint mOldTexWidth = 2048;
    GLuint mOldTexHeight = 2048;

    TextureFormat mTexFormat = TextureFormat::RGBA8;

    GLint mMaxTexUnits;
    GLint mNumTexUnits = 32;

    QOpenGLShaderProgram* mBlenderProgram;
    GLuint mVao;
    GLuint mVboPos;
    GLuint mVboTex;

    GLsync fence = 0;

    GLenum getFormat(GLenum format);

    void setShaderPrograms();

    void adjustOrtho();

    void resizeVertices();

    void genTexture(GLuint& texId);
    void genTextures(ImageOperation* operation);
    void resizeTextures();
};



#endif // FBO_H
