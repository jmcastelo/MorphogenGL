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

#include "seed.h"
#include "morphowidget.h"

Seed::Seed(QOpenGLContext* mainContext)
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

    // Frame buffer object

    generateFramebuffer(fboRandom, texRandom);
    generateFramebuffer(fboImage, texImage);

    textureID = &texRandom;

    // Initialize shader programs

    clearProgram = new QOpenGLShaderProgram();
    if (!clearProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/random-seed.vert"))
        qDebug() << "Vertex shader error:\n" << clearProgram->log();
    if (!clearProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/clear.frag"))
        qDebug() << "Fragment shader error:\n" << clearProgram->log();
    if (!clearProgram->link())
        qDebug() << "Shader link error:\n" << clearProgram->log();

    imageProgram = new QOpenGLShaderProgram();
    if (!imageProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/screen.vert"))
        qDebug() << "Vertex shader error:\n" << imageProgram->log();
    if (!imageProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/screen.frag"))
        qDebug() << "Fragment shader error:\n" << imageProgram->log();
    if (!imageProgram->link())
        qDebug() << "Shader link error:\n" << imageProgram->log();

    randomProgram = new QOpenGLShaderProgram();
    if (!randomProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/random-seed.vert"))
        qDebug() << "Vertex shader error:\n" << randomProgram->log();
    if (!randomProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/random-seed.frag"))
        qDebug() << "Fragment shader error:\n" << randomProgram->log();
    if (!randomProgram->link())
        qDebug() << "Shader link error:\n" << randomProgram->log();

    // In quad data

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

    // Random

    // Vertex array object

    vaoRandom = new QOpenGLVertexArrayObject();
    vaoRandom->create();
    vaoRandom->bind();

    // Vertex buffer object

    vboRandom = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboRandom->create();
    vboRandom->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboRandom->bind();
    vboRandom->allocate(vertices, 12 * sizeof(GLfloat));

    // Map vbo data to shader attribute location

    randomProgram->bind();
    int posLocation = randomProgram->attributeLocation("pos");
    randomProgram->setAttributeBuffer(posLocation, GL_FLOAT, 0, 2);
    randomProgram->enableAttributeArray(posLocation);
    randomProgram->release();

    // Unbind all

    vaoRandom->release();
    vboRandom->release();

    // Image

    // Vertex array object

    vaoImage = new QOpenGLVertexArrayObject();
    vaoImage->create();
    vaoImage->bind();

    // Vertex buffer object

    vboPosImage = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboPosImage->create();
    vboPosImage->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboPosImage->bind();
    vboPosImage->allocate(vertices, 12 * sizeof(GLfloat));

    // Map vbo data to shader attribute location

    imageProgram->bind();
    posLocation = imageProgram->attributeLocation("pos");
    imageProgram->setAttributeBuffer(posLocation, GL_FLOAT, 0, 2);
    imageProgram->enableAttributeArray(posLocation);
    imageProgram->release();

    // Vertex buffer object: texture coordinates

    vboTexImage = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboTexImage->create();
    vboTexImage->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboTexImage->bind();
    vboTexImage->allocate(texcoords, 12 * sizeof(GLfloat));

    // Map vbo data to shader attribute location

    imageProgram->bind();
    int texcoordLocation = imageProgram->attributeLocation("tex");
    imageProgram->setAttributeBuffer(texcoordLocation, GL_FLOAT, 0, 2);
    imageProgram->enableAttributeArray(texcoordLocation);
    imageProgram->release();

    // Unbind all

    vaoImage->release();
    vboPosImage->release();
    vboTexImage->release();

    context->doneCurrent();

    widthOld = FBO::width;
    heightOld = FBO::height;

    image = new QOpenGLTexture(QOpenGLTexture::Target2D);
}

Seed::Seed(const Seed& seed) :
    type { seed.type },
    fixed { seed.fixed },
    imageFilename { seed.imageFilename }
{
    context = new QOpenGLContext();
    context->setFormat(seed.context->format());
    context->setShareContext(seed.context);
    context->create();

    surface = new QOffscreenSurface();
    surface->setFormat(seed.context->format());
    surface->create();

    context->makeCurrent(surface);

    initializeOpenGLFunctions();

    // Frame buffer object

    generateFramebuffer(fboRandom, texRandom);
    generateFramebuffer(fboImage, texImage);

    textureID = &texRandom;

    // Initialize shader programs

    clearProgram = new QOpenGLShaderProgram();
    if (!clearProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/random-seed.vert"))
        qDebug() << "Vertex shader error:\n" << clearProgram->log();
    if (!clearProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/clear.frag"))
        qDebug() << "Fragment shader error:\n" << clearProgram->log();
    if (!clearProgram->link())
        qDebug() << "Shader link error:\n" << clearProgram->log();

    imageProgram = new QOpenGLShaderProgram();
    if (!imageProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/screen.vert"))
        qDebug() << "Vertex shader error:\n" << imageProgram->log();
    if (!imageProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/screen.frag"))
        qDebug() << "Fragment shader error:\n" << imageProgram->log();
    if (!imageProgram->link())
        qDebug() << "Shader link error:\n" << imageProgram->log();

    randomProgram = new QOpenGLShaderProgram();
    if (!randomProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/random-seed.vert"))
        qDebug() << "Vertex shader error:\n" << randomProgram->log();
    if (!randomProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/random-seed.frag"))
        qDebug() << "Fragment shader error:\n" << randomProgram->log();
    if (!randomProgram->link())
        qDebug() << "Shader link error:\n" << randomProgram->log();

    // In quad data

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

    // Random

    // Vertex array object

    vaoRandom = new QOpenGLVertexArrayObject();
    vaoRandom->create();
    vaoRandom->bind();

    // Vertex buffer object

    vboRandom = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboRandom->create();
    vboRandom->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboRandom->bind();
    vboRandom->allocate(vertices, 12 * sizeof(GLfloat));

    // Map vbo data to shader attribute location

    randomProgram->bind();
    int posLocation = randomProgram->attributeLocation("pos");
    randomProgram->setAttributeBuffer(posLocation, GL_FLOAT, 0, 2);
    randomProgram->enableAttributeArray(posLocation);
    randomProgram->release();

    // Unbind all

    vaoRandom->release();
    vboRandom->release();

    // Image

    // Vertex array object

    vaoImage = new QOpenGLVertexArrayObject();
    vaoImage->create();
    vaoImage->bind();

    // Vertex buffer object

    vboPosImage = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboPosImage->create();
    vboPosImage->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboPosImage->bind();
    vboPosImage->allocate(vertices, 12 * sizeof(GLfloat));

    // Map vbo data to shader attribute location

    imageProgram->bind();
    posLocation = imageProgram->attributeLocation("pos");
    imageProgram->setAttributeBuffer(posLocation, GL_FLOAT, 0, 2);
    imageProgram->enableAttributeArray(posLocation);
    imageProgram->release();

    // Vertex buffer object: texture coordinates

    vboTexImage = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboTexImage->create();
    vboTexImage->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboTexImage->bind();
    vboTexImage->allocate(texcoords, 12 * sizeof(GLfloat));

    // Map vbo data to shader attribute location

    imageProgram->bind();
    int texcoordLocation = imageProgram->attributeLocation("tex");
    imageProgram->setAttributeBuffer(texcoordLocation, GL_FLOAT, 0, 2);
    imageProgram->enableAttributeArray(texcoordLocation);
    imageProgram->release();

    // Unbind all

    vaoImage->release();
    vboPosImage->release();
    vboTexImage->release();

    context->doneCurrent();

    widthOld = FBO::width;
    heightOld = FBO::height;

    image = new QOpenGLTexture(QOpenGLTexture::Target2D);

    if (!imageFilename.isEmpty())
        loadImage(imageFilename);
}

Seed::~Seed()
{
    context->makeCurrent(surface);

    vaoRandom->destroy();
    vboRandom->destroy();

    vaoImage->destroy();
    vboPosImage->destroy();
    vboTexImage->destroy();

    glDeleteFramebuffers(1, &fboRandom);
    glDeleteFramebuffers(1, &fboImage);

    delete vaoRandom;
    delete vboRandom;
    delete randomProgram;

    delete vaoImage;
    delete vboPosImage;
    delete vboTexImage;
    delete imageProgram;

    delete clearProgram;

    glDeleteTextures(1, &texRandom);
    glDeleteTextures(1, &texImage);

    image->destroy();
    delete image;

    context->doneCurrent();

    delete context;
    delete surface;
}

void Seed::loadImage(QString filename)
{
    context->makeCurrent(surface);

    image->destroy();
    image->setSize(FBO::width, FBO::height);
    if (!filename.isEmpty())
        image->setData(QImage(filename).mirrored());

    context->doneCurrent();

    imageFilename = filename;
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

void Seed::resizeFBO(GLuint &fbo, GLuint &texture)
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
    glDeleteTextures(1, &texture);

    context->doneCurrent();

    texture = textureID2;
    fbo = fbo2;
}

void Seed::resize()
{
    resizeFBO(fboRandom, texRandom);
    resizeFBO(fboImage, texImage);

    loadImage(imageFilename);

    widthOld = FBO::width;
    heightOld = FBO::height;
}

void Seed::draw()
{
    if (type == 0) drawRandom(false);
    else if (type == 1) drawRandom(true);
    else if (type == 2) drawImage();
}

void Seed::drawRandom(bool grayscale)
{
    context->makeCurrent(surface);

    glBindFramebuffer(GL_FRAMEBUFFER, fboRandom);

    glViewport(0, 0, FBO::width, FBO::height);
    glClear(GL_COLOR_BUFFER_BIT);

    randomProgram->bind();
    vaoRandom->bind();

    std::uniform_real_distribution<float> distribution(0, 1);
    float randomNumber = distribution(generator);

    randomProgram->setUniformValue("randomNumber", randomNumber);
    randomProgram->setUniformValue("grayscale", grayscale);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    vaoRandom->release();
    randomProgram->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();

    textureID = &texRandom;
}

void Seed::drawImage()
{
    context->makeCurrent(surface);

    glBindFramebuffer(GL_FRAMEBUFFER, fboImage);

    glViewport(0, 0, FBO::width, FBO::height);
    glClear(GL_COLOR_BUFFER_BIT);

    imageProgram->bind();
    vaoImage->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image->textureId());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    vaoImage->release();
    imageProgram->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();

    textureID = &texImage;
}

void Seed::clear()
{
    context->makeCurrent(surface);

    glBindFramebuffer(GL_FRAMEBUFFER, fboRandom);

    glViewport(0, 0, FBO::width, FBO::height);
    glClear(GL_COLOR_BUFFER_BIT);

    clearProgram->bind();
    vaoRandom->bind();

    glDrawArrays(GL_TRIANGLES, 0, 6);

    vaoRandom->release();
    clearProgram->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();

    textureID = &texRandom;
}
