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

#include "fbo.h"
#include <QDebug>

GLuint FBO::width = 1024;
GLuint FBO::height = 1024;

TextureFormat FBO::texFormat = TextureFormat::RGBA8;

FBO::FBO(QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext)
{
    context = new QOpenGLContext();
    context->setFormat(mainContext->format());
    context->setShareContext(mainContext);
    context->create();

    surface = new QOffscreenSurface();
    surface->setFormat(mainContext->format());
    surface->create();

    context->makeCurrent(surface);

    initializeOpenGLFunctions();

    // Old size

    widthOld = width;
    heightOld = height;

    // Frame buffer objects

    generateFramebuffer(fbo, textureID);
    generateFramebuffer(fboBlit, textureBlit);

    // Set texture pointers

    texID = &textureID;
    texBlit = &textureBlit;

    // Initialize shader program

    program = new QOpenGLShaderProgram();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShader))
        qDebug() << "Vertex shader error:\n" << program->log();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShader))
        qDebug() << "Fragment shader error:\n" << program->log();
    if (!program->link())
        qDebug() << "Shader link error:\n" << program->log();

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

    vboPos = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboPos->create();
    vboPos->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboPos->bind();
    vboPos->allocate(vertices, 12 * sizeof(GLfloat));

    // Map vbo data to shader attribute location

    program->bind();
    int posLocation = program->attributeLocation("pos");
    program->setAttributeBuffer(posLocation, GL_FLOAT, 0, 2);
    program->enableAttributeArray(posLocation);
    program->release();

    // Vertex buffer object: texture coordinates

    vboTex = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboTex->create();
    vboTex->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboTex->bind();
    vboTex->allocate(texcoords, 12 * sizeof(GLfloat));

    // Map vbo data to shader attribute location

    program->bind();
    int texcoordLocation = program->attributeLocation("tex");
    program->setAttributeBuffer(texcoordLocation, GL_FLOAT, 0, 2);
    program->enableAttributeArray(texcoordLocation);
    program->release();

    // Unbind all

    vao->release();
    vboPos->release();
    vboTex->release();

    // Sampler

    glGenSamplers(1, &samplerID);
    glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, minMagFilter);
    glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, minMagFilter);

    // Initialize identity shader program

    identityProgram = new QOpenGLShaderProgram();
    if (!identityProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/screen.vert"))
        qDebug() << "Vertex shader error:\n" << identityProgram->log();
    if (!identityProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/screen.frag"))
        qDebug() << "Fragment shader error:\n" << identityProgram->log();
    if (!identityProgram->link())
        qDebug() << "Shader link error:\n" << identityProgram->log();

    context->doneCurrent();

    // Set ortho projection

    resizeVertices();
    transformationMatrix.setToIdentity();
    adjustTransform();
}

FBO::~FBO()
{
    context->makeCurrent(surface);

    vao->destroy();
    vboPos->destroy();
    vboTex->destroy();

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &textureID);

    glDeleteFramebuffers(1, &fboBlit);
    glDeleteTextures(1, &textureBlit);

    delete vao;
    delete vboPos;
    delete vboTex;
    delete program;
    delete identityProgram;

    context->doneCurrent();

    delete context;
    delete surface;
}

void FBO::setMinMagFilter(GLenum filter)
{
    minMagFilter = filter;
    
    context->makeCurrent(surface);
    glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, minMagFilter);
    glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, minMagFilter);
    context->doneCurrent();
}

void FBO::setTextureFormat()
{
    context->makeCurrent(surface);

    // FBO

    GLuint fbo2;
    GLuint textureID2;
    generateFramebuffer(fbo2, textureID2);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo2);

    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &textureID);

    textureID = textureID2;
    fbo = fbo2;

    // Blit FBO

    GLuint fboBlit2;
    GLuint textureBlit2;
    generateFramebuffer(fboBlit2, textureBlit2);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboBlit);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlit2);

    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &fboBlit);
    glDeleteTextures(1, &textureBlit);

    textureBlit = textureBlit2;
    fboBlit = fboBlit2;

    context->doneCurrent();
}

void FBO::generateFramebuffer(GLuint& framebuffer, GLuint& texture)
{
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(FBO::texFormat), width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        qDebug() << "Framebuffer is not complete.\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::makeCurrent()
{
    context->makeCurrent(surface);
}

void FBO::doneCurrent()
{
    context->doneCurrent();
}

void FBO::resizeVertices()
{
    // Recompute vertices and texture coordinates

    GLfloat left, right, bottom, top;
    GLfloat ratio = static_cast<GLfloat>(width) / static_cast<GLfloat>(height);

    if (width > height)
    {
        top = 1.0f;
        bottom = -top;
        right = top * ratio;
        left = -right;
    }
    else
    {
        right = 1.0f;
        left = -right;
        top = right / ratio;
        bottom = -top;
    }

    GLfloat vertices[] = {
        left, top,
        left, bottom,
        right, bottom,
        left, top,
        right, bottom,
        right, top
    };

    context->makeCurrent(surface);

    vao->bind();
    vboPos->bind();
    vboPos->allocate(vertices, 12 * sizeof(GLfloat));
    vao->release();
    vboPos->release();

    context->doneCurrent();
}

void FBO::adjustTransform()
{
    GLfloat left, right, bottom, top;
    GLfloat ratio = static_cast<GLfloat>(width) / static_cast<GLfloat>(height);

    if (width > height)
    {
        top = 1.0f;
        bottom = -top;
        right = top * ratio;
        left = -right;
    }
    else
    {
        right = 1.0f;
        left = -right;
        top = right / ratio;
        bottom = -top;
    }

    // Maintain aspect ratio

    QMatrix4x4 transform;
    transform.setToIdentity();
    transform.ortho(left, right, bottom, top, -1.0, 1.0);

    context->makeCurrent(surface);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    program->bind();
    program->setUniformValue("transform", transform * transformationMatrix);
    program->release();
    identityProgram->bind();
    identityProgram->setUniformValue("transform", transform);
    identityProgram->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();
}

void FBO::resize()
{
    resizeVertices();

    adjustTransform();

    context->makeCurrent(surface);

    // FBO

    GLuint fbo2;
    GLuint textureID2;
    generateFramebuffer(fbo2, textureID2);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo2);

    glBlitFramebuffer(0, 0, widthOld, heightOld, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &textureID);

    textureID = textureID2;
    fbo = fbo2;

    // Blit FBO

    GLuint fboBlit2;
    GLuint textureBlit2;
    generateFramebuffer(fboBlit2, textureBlit2);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboBlit);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlit2);

    glBlitFramebuffer(0, 0, widthOld, heightOld, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &fboBlit);
    glDeleteTextures(1, &textureBlit);

    textureBlit = textureBlit2;
    fboBlit = fboBlit2;

    context->doneCurrent();

    // Keep size

    widthOld = width;
    heightOld = height;

    emit sizeChanged();
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
    glBindTexture(GL_TEXTURE_2D, *inputTextureID);
    glBindSampler(0, samplerID);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindSampler(0, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    vao->release();
    program->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();
}

void FBO::blit()
{
    context->makeCurrent(surface);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlit);

    glBlitFramebuffer(0, 0, FBO::width, FBO::height, 0, 0, FBO::width, FBO::height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();
}

void FBO::identity()
{
    context->makeCurrent(surface);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    identityProgram->bind();
    vao->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *inputTextureID);
    glBindSampler(0, samplerID);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindSampler(0, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    vao->release();
    identityProgram->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();
}

void FBO::clear()
{
    context->makeCurrent(surface);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glViewport(0, 0, width, height);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fboBlit);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();
}

QImage FBO::outputImage()
{
    context->makeCurrent(surface);

    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    //glFlush();
    //glFinish();
    //glClientWaitSync(fence, 0, GL_TIMEOUT_IGNORED);
    //glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    GLenum status;
    do {
        status = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000);
    } while (status == GL_TIMEOUT_EXPIRED);

    glDeleteSync(fence);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboBlit);

    glViewport(0, 0, width, height);

    QImage image(width, height, QImage::Format_RGBA8888);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();

    return image;
}
