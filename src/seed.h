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

#include "fbo.h"
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
    Seed(QOpenGLContext* theContext);
    Seed(const Seed& seed);
    ~Seed();

    GLuint** getTextureID() { return &textureID; }

    void loadImage(QString filename);

    void generateFramebuffer(GLuint& framebuffer, GLuint& texture);

    void resize();

    void draw();
    void drawRandom(bool grayscale);
    void drawImage();
    void clear();

    int getType() { return type; }
    void setType(int set) { type = set; }

    bool isFixed() { return fixed; }
    void setFixed(bool set) { fixed = set; }

    QString getImageFilename() { return imageFilename; }

private:
    int type = 0;
    bool fixed = false;
    QString imageFilename;

    QOpenGLContext* context;
    QOffscreenSurface* surface;

    GLuint widthOld, heightOld;

    GLuint* textureID;

    GLuint fboRandom = 0;
    GLuint texRandom = 0;
    QOpenGLVertexArrayObject* vaoRandom;
    QOpenGLBuffer* vboRandom;
    QOpenGLShaderProgram* randomProgram;

    GLuint fboImage = 0;
    GLuint texImage = 0;
    QOpenGLTexture* image;
    QOpenGLVertexArrayObject* vaoImage;
    QOpenGLBuffer* vboPosImage;
    QOpenGLBuffer* vboTexImage;
    QOpenGLShaderProgram* imageProgram;

    QOpenGLShaderProgram* clearProgram;

    std::default_random_engine generator;

    void resizeFBO(GLuint &fbo, GLuint &texture);
};
