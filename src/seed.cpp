/*
*  Copyright 2020 José María Castelo Ares
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

#include "seed.h"
#include "morphowidget.h"

Seed::Seed(QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext)
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

    // Frame buffer object

    generateFramebuffer(fbo, randomTex);

    widthOld = FBO::width;
    heightOld = FBO::height;

    // Initialize shader program

    program = new QOpenGLShaderProgram();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShader))
        qDebug() << "Vertex shader error:\n" << program->log();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShader))
        qDebug() << "Fragment shader error:\n" << program->log();
    if (!program->link())
        qDebug() << "Shader link error:\n" << program->log();

    program->bind();

    // In quad vertices data

    GLfloat vertices[] = {
        1.0f, 1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, -1.0f,
        -1.0f, -1.0f,
        -1.0f, 1.0f
    };

    // Vertex array object

    vao = new QOpenGLVertexArrayObject();
    vao->create();
    vao->bind();

    // Vertex buffer object

    vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vbo->create();
    vbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vbo->bind();
    vbo->allocate(vertices, 12 * sizeof(GLfloat));

    // Map vbo data to shader attribute location

    int posLocation = program->attributeLocation("pos");
    program->setAttributeBuffer(posLocation, GL_FLOAT, 0, 2);
    program->enableAttributeArray(posLocation);

    // Unbind all

    vao->release();
    vbo->release();
    program->release();

    context->doneCurrent();

    // FBO to draw seed texture image

    fboTex = new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", mainContext);
    seedTex = new QOpenGLTexture(QOpenGLTexture::Target2D);
}

Seed::~Seed()
{
    context->makeCurrent(surface);

    vao->destroy();
    vbo->destroy();

    glDeleteFramebuffers(1, &fbo);
    delete vao;
    delete vbo;
    delete program;

    glDeleteTextures(1, &randomTex);

    seedTex->destroy();
    delete seedTex;

    context->doneCurrent();

    delete context;
    delete surface;

    delete fboTex;
}

void Seed::loadImage(QString filename)
{
    seedTex->destroy();
    seedTex->setSize(FBO::width, FBO::height);
    if (!filename.isEmpty())
        seedTex->setData(QImage(filename).mirrored());

    fboTex->setInputTextureID(seedTex->textureId());

    textureFilename = filename;
}

void Seed::generateFramebuffer(GLuint& framebuffer, GLuint& texture)
{
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, FBO::width, FBO::height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        qDebug() << "Framebuffer is not complete.\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Seed::resize()
{
    context->makeCurrent(surface);

    GLuint fbo2;
    GLuint textureID2;
    generateFramebuffer(fbo2, textureID2);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo2);

    glBlitFramebuffer(0, 0, widthOld, heightOld, 0, 0, FBO::width, FBO::height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &randomTex);

    widthOld = FBO::width;
    heightOld = FBO::height;

    randomTex = textureID2;
    fbo = fbo2;

    // Resize seed texture by loading image again

    loadImage(textureFilename);

    // Keep texture fbo out of current context, context will be made current when resizing it

    context->doneCurrent();

    // Resize textured fbo

    fboTex->resize();
}

void Seed::drawRandom(bool grayscale)
{
    context->makeCurrent(surface);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glViewport(0, 0, FBO::width, FBO::height);
    glClear(GL_COLOR_BUFFER_BIT);

    program->bind();
    vao->bind();

    std::uniform_real_distribution<float> distribution(0, 1);
    float randomNumber = distribution(generator);

    program->setUniformValue("randomNumber", randomNumber);
    program->setUniformValue("grayscale", grayscale);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    vao->release();
    program->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();

    textureID = randomTex;
}

void Seed::drawImage()
{
    fboTex->draw();
    textureID = fboTex->getTextureID();
}