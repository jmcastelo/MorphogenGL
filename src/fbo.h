/*
*  Copyright 2020 Jose Maria Castelo Ares
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

#include <QOpenGLExtraFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QDebug>

class MorphoWidget;

class FBO : protected QOpenGLExtraFunctions
{
public:
    static GLuint width;
    static GLuint height;

    QOpenGLShaderProgram* program;
    
    FBO(QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext);
    virtual ~FBO();
    
    GLuint getTextureID() { return textureID; }
    void setInputTextureID(GLuint id) { inputTextureID = id; }
        
    void generateFramebuffer(GLuint& framebuffer, GLuint& texture);

    void setMinMagFilter(GLenum filter);

    void makeCurrent();
    void doneCurrent();

    virtual void resize();

    void draw();

protected:
    QOpenGLContext* context;
    QOffscreenSurface* surface;
    GLuint textureID = 0;
    GLenum minMagFilter = GL_NEAREST;
    GLuint inputTextureID = 0;
    GLuint samplerID = 0;
    GLuint fbo = 0;
    QOpenGLVertexArrayObject* vao;
    QOpenGLBuffer* vbo_pos;
    QOpenGLBuffer* vbo_tex;
    GLuint widthOld;
    GLuint heightOld;
};
