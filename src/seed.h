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



#ifndef SEED_H
#define SEED_H



#include <random>
#include <QOpenGLExtraFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QImage>
#include <QString>
#include <QDebug>



class Seed : protected QOpenGLExtraFunctions
{
public:
    Seed(GLenum texFormat, GLuint width, GLuint height, QOpenGLContext* context, QOffscreenSurface* surface);
    Seed(GLenum texFormat, GLuint width, GLuint height, const Seed& seed);
    ~Seed();

    GLuint outTextureId();
    QList<GLuint*> textureIds();

    void setOutTexture(bool draw);

    void setVao(GLuint width, GLuint height);

    void genTextures(GLenum texFormat, GLuint width, GLuint height);

    void loadImage(QString filename);

    int type() const;
    void setType(int type);

    bool fixed() const;
    void setFixed(bool set);

    QString imageFilename() const;

private:
    int mType = 0;
    bool mFixed = false;
    QString mImageFilename;
    bool mCleared = false;

    QOpenGLContext* mContext;
    QOffscreenSurface* mSurface;

    GLuint mOutFbo = 0;
    GLuint mVao = 0;
    GLuint mVboPos = 0;

    GLuint mOutTexId = 0;

    GLuint mRandomTexId = 0;
    QOpenGLShaderProgram* mRandomProgram;

    QOpenGLTexture* mImageTex;

    GLuint mClearTexId = 0;

    std::default_random_engine mGenerator;

    void clearTexture(GLuint texId);

    void setRandomProgram();

    void drawRandom(bool grayscale);
};



#endif // SEED_H
