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
    Seed(QString vertexShader, QString fragmentShader, QOpenGLContext* theContext);
    ~Seed();

    GLuint getTextureID() { return textureID; }

    void loadImage(QString filename);

    void generateFramebuffer(GLuint& framebuffer, GLuint& texture);

    void resize();

    void drawRandom(bool grayscale);

    void drawImage();

private:
    QOpenGLContext* context;
    QOffscreenSurface* surface;
    GLuint textureID;
    GLuint randomTex;
    GLuint fbo;
    QOpenGLVertexArrayObject* vao;
    QOpenGLBuffer* vbo;
    QOpenGLShaderProgram* program;
    std::default_random_engine generator;
    FBO* fboTex;
    QOpenGLTexture* seedTex;
    GLuint widthOld, heightOld;
    QString textureFilename;
};