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
    Seed(QOpenGLContext* shareContext);
    Seed(const Seed& seed);
    ~Seed();

    GLuint outTextureId();
    QList<GLuint*> textureIds();

    void loadImage(QString filename);

    void draw();

    int type() const { return mType; }
    void setType(int type);

    bool fixed() const { return mFixed; }
    void setFixed(bool set) { mFixed = set; }

    QString imageFilename() const { return mImageFilename; }

    bool cleared() const { return mCleared; }

private:
    int mType = 0;
    bool mFixed = false;
    QString mImageFilename;
    bool mCleared = false;

    QOpenGLContext* mContext;
    QOffscreenSurface* mSurface;

    GLuint mOutTexId = 0;

    GLuint mRandomTexId = 0;
    QOpenGLShaderProgram* mRandomProgram;

    QOpenGLTexture* mImageTex;

    std::default_random_engine mGenerator;

    void setRandomProgram();

    void drawRandom(bool grayscale);
    void drawImage();
};



#endif // SEED_H
