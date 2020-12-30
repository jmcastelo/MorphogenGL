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

#include "fbo.h"

GLuint FBO::width = 512;
GLuint FBO::height = 512;

FBO::FBO(QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext)
{
    context = new QOpenGLContext();
    context->setFormat(mainContext->format());
    context->setShareContext(mainContext);
    context->create();

    surface = new QOffscreenSurface();
    surface->setFormat(context->format());
    surface->create();

    context->makeCurrent(surface);

    initializeOpenGLFunctions();

    // Old size

    widthOld = width;
    heightOld = height;

    // Frame buffer object

    generateFramebuffer(fbo, textureID);

    // Initialize shader program

    program = new QOpenGLShaderProgram();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShader))
        qDebug() << "Vertex shader error:\n" << program->log();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShader))
        qDebug() << "Fragment shader error:\n" << program->log();
    if (!program->link())
        qDebug() << "Shader link error:\n" << program->log();

    program->bind();

    // Input quad vertices and texture data

    GLfloat vertices[] = {
        -1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f
    };

    GLfloat texcoords[] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };

    // Vertex array object

    vao = new QOpenGLVertexArrayObject();
    vao->create();
    vao->bind();

    // Vertex buffer object: vertices

    vbo_pos = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vbo_pos->create();
    vbo_pos->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vbo_pos->bind();
    vbo_pos->allocate(vertices, 12 * sizeof(GLfloat));

    // Map vbo data to shader attribute location

    int posLocation = program->attributeLocation("pos");
    program->setAttributeBuffer(posLocation, GL_FLOAT, 0, 2);
    program->enableAttributeArray(posLocation);

    // Vertex buffer object: texture coordinates

    vbo_tex = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vbo_tex->create();
    vbo_tex->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vbo_tex->bind();
    vbo_tex->allocate(texcoords, 12 * sizeof(GLfloat));

    // Map vbo data to shader attribute location

    int texcoordLocation = program->attributeLocation("tex");
    program->setAttributeBuffer(texcoordLocation, GL_FLOAT, 0, 2);
    program->enableAttributeArray(texcoordLocation);

    // Unbind all

    vao->release();
    vbo_pos->release();
    vbo_tex->release();
    program->release();

    // Sampler

    glGenSamplers(1, &samplerID);
    glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, minMagFilter);
    glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, minMagFilter);

    context->doneCurrent();
}

FBO::~FBO()
{
    context->makeCurrent(surface);

    vao->destroy();
    vbo_pos->destroy();
    vbo_tex->destroy();

    glDeleteFramebuffers(1, &fbo);
    delete vao;
    delete vbo_pos;
    delete vbo_tex;
    delete program;

    context->doneCurrent();

    delete context;
    delete surface;
}

void FBO::generateFramebuffer(GLuint& framebuffer, GLuint& texture)
{
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minMagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, minMagFilter);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        qDebug() << "Framebuffer is not complete.\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::setMinMagFilter(GLenum filter)
{
    minMagFilter = filter;
    
    context->makeCurrent(surface);
    glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, minMagFilter);
    glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, minMagFilter);
    context->doneCurrent();
}

void FBO::makeCurrent()
{
    context->makeCurrent(surface);
}

void FBO::doneCurrent()
{
    context->doneCurrent();
}

void FBO::resize()
{
    context->makeCurrent(surface);

    GLuint fbo2;
    GLuint textureID2;
    generateFramebuffer(fbo2, textureID2);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo2);

    glBlitFramebuffer(0, 0, widthOld, heightOld, 0, 0, width, height, GL_COLOR_BUFFER_BIT, minMagFilter);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &textureID);

    textureID = textureID2;
    fbo = fbo2;

    widthOld = width;
    heightOld = height;

    context->doneCurrent();
}

void FBO::draw()
{
    context->makeCurrent(surface);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    program->bind();
    vao->bind();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTextureID);
    glBindSampler(0, samplerID);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindSampler(0, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    vao->release();
    program->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();
}