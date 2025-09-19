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



Seed::Seed(QOpenGLContext* shareContext)
{
    mContext = new QOpenGLContext();
    mContext->setFormat(shareContext->format());
    mContext->setShareContext(shareContext);
    mContext->create();

    mSurface = new QOffscreenSurface();
    mSurface->setFormat(shareContext->format());
    mSurface->create();

    mContext->makeCurrent(mSurface);

    initializeOpenGLFunctions();

    // Initialize shader program

    mRandomProgram = new QOpenGLShaderProgram();
    setRandomProgram();

    mContext->doneCurrent();

    // Set image

    mImageTex = new QOpenGLTexture(QOpenGLTexture::Target2D);
}



Seed::Seed(const Seed& seed) :
    mType { seed.mType },
    mFixed { seed.mFixed },
    mImageFilename { seed.mImageFilename }
{
    mContext = new QOpenGLContext();
    mContext->setFormat(seed.mContext->format());
    mContext->setShareContext(seed.mContext);
    mContext->create();

    mSurface = new QOffscreenSurface();
    mSurface->setFormat(seed.mContext->format());
    mSurface->create();

    mContext->makeCurrent(mSurface);

    initializeOpenGLFunctions();

    // Initialize shader program

    mRandomProgram = new QOpenGLShaderProgram();
    setRandomProgram();

    mContext->doneCurrent();

    // Set image

    mImageTex = new QOpenGLTexture(QOpenGLTexture::Target2D);

    if (!mImageFilename.isEmpty())
        loadImage(mImageFilename);
}



Seed::~Seed()
{
    mContext->makeCurrent(mSurface);

    delete mRandomProgram;

    glDeleteTextures(1, &mRandomTexId);

    mImageTex->destroy();
    delete mImageTex;

    mContext->doneCurrent();

    delete mContext;
    delete mSurface;
}



GLuint Seed::outTextureId()
{
    return mOutTexId;
}



QList<GLuint*> Seed::textureIds()
{
    return QList<GLuint*> { &mRandomTexId };
}



void Seed::loadImage(QString filename)
{
    mContext->makeCurrent(mSurface);

    mImageTex->destroy();
    if (!filename.isEmpty())
        //mImageTex->setData(QImage(filename).mirrored());
        mImageTex->setData(QImage(filename));

    mContext->doneCurrent();

    mImageFilename = filename;
}



void Seed::setType(int type)
{
    mType = type;

    if (mType == 0 || mType == 1)
        mOutTexId = mRandomTexId;
    else if (mType == 2)
        mOutTexId = mImageTex->textureId();

    //draw();
}



void Seed::draw()
{
    if (mType == 0)
        drawRandom(false);
    else if (mType == 1)
        drawRandom(true);
    else if (mType == 2)
        drawImage();

    mCleared = false;
}



void Seed::setRandomProgram()
{
    if (!mRandomProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/position.vert"))
        qDebug() << "Vertex shader error:\n" << mRandomProgram->log();
    if (!mRandomProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/random-seed.frag"))
        qDebug() << "Fragment shader error:\n" << mRandomProgram->log();
    if (!mRandomProgram->link())
        qDebug() << "Shader link error:\n" << mRandomProgram->log();

    // Set in attribute: vertex coordinates

    mRandomProgram->bind();

    mRandomProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2);
    mRandomProgram->enableAttributeArray(0);

    mRandomProgram->release();
}


void Seed::drawRandom(bool grayscale)
{
    /*context->makeCurrent(surface);

    glBindFramebuffer(GL_FRAMEBUFFER, fboRandom);

    glViewport(0, 0, FBO::width, FBO::height);
    glClear(GL_COLOR_BUFFER_BIT);*/

    mRandomProgram->bind();

    std::uniform_real_distribution<float> distribution(0, 1);
    float randomNumber = distribution(mGenerator);

    mRandomProgram->setUniformValue("randomNumber", randomNumber);
    mRandomProgram->setUniformValue("grayscale", grayscale);

    /*glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    vaoRandom->release();
    randomProgram->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();*/
}



void Seed::drawImage()
{
    /*context->makeCurrent(surface);

    glBindFramebuffer(GL_FRAMEBUFFER, fboImage);

    glViewport(0, 0, FBO::width, FBO::height);
    glClear(GL_COLOR_BUFFER_BIT);

    imageProgram->bind();
    vaoImage->bind();*/

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mImageTex->textureId());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    vaoImage->release();
    imageProgram->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();
}
