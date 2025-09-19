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



Seed::Seed(GLenum texFormat, GLuint width, GLuint height, GLuint vao, QOpenGLContext* shareContext) :
    mVao { vao }
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

    // Framebuffer object

    glGenFramebuffers(1, &mOutFbo);

    // Generate textures with format and size given externally

    genTextures(texFormat, width, height);

    // Clear texture

    clearTexture(mClearTexId);

    // Initialize shader program

    mRandomProgram = new QOpenGLShaderProgram();
    setRandomProgram();

    mContext->doneCurrent();

    // Texture to store loaded image

    mImageTex = new QOpenGLTexture(QOpenGLTexture::Target2D);
}



Seed::Seed(GLenum texFormat, GLuint width, GLuint height, const Seed& seed) :
    mType { seed.mType },
    mFixed { seed.mFixed },
    mImageFilename { seed.mImageFilename },
    mVao { seed.mVao }
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

    // Framebuffer object

    glGenFramebuffers(1, &mOutFbo);

    // Generate textures with format and size given externally

    genTextures(texFormat, width, height);

    // Clear texture

    clearTexture(mClearTexId);

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

    glDeleteFramebuffers(1, &mOutFbo);

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



QList<GLuint> Seed::textureIds()
{
    return QList<GLuint> { mRandomTexId, mClearTexId };
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



int Seed::type() const
{
    return mType;
}


void Seed::setType(int type)
{
    mType = type;
}



bool Seed::fixed() const
{
    return mFixed;
}



void Seed::setFixed(bool set)
{
    mFixed = set;
}



QString Seed::imageFilename() const
{
    return mImageFilename;
}



void Seed::setOutTexture(bool draw)
{
    if (draw)
    {
        if (mType == 0 || mType == 1)
            mOutTexId = mRandomTexId;
        else if (mType == 2)
            mOutTexId = mImageTex->textureId();

        mCleared = false;
    }
    else if (!mCleared && !mFixed)
    {
        mOutTexId = mClearTexId;
        mCleared = true;
    }
}



void Seed::genTextures(GLenum texFormat, GLuint width, GLuint height)
{
    // Allocated on immutable storage (glTexStorage2D)
    // To be called within active OpenGL context

    foreach (GLuint texId, textureIds())
    {
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexStorage2D(GL_TEXTURE_2D, 1, texFormat, width, height);
        glTexParameteri(texId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(texId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}



void Seed::clearTexture(GLuint texId)
{
    // To be called within active OpenGL context

    glBindFramebuffer(GL_FRAMEBUFFER, mOutFbo);
    glBindVertexArray(mVao);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);

    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    mContext->makeCurrent(mSurface);

    glBindFramebuffer(GL_FRAMEBUFFER, mOutFbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRandomTexId, 0);

    glBindVertexArray(mVao);

    glClear(GL_COLOR_BUFFER_BIT);

    mRandomProgram->bind();

    std::uniform_real_distribution<float> distribution(0, 1);
    float randomNumber = distribution(mGenerator);

    mRandomProgram->setUniformValue("randomNumber", randomNumber);
    mRandomProgram->setUniformValue("grayscale", grayscale);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    mRandomProgram->release();

    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    mContext->doneCurrent();
}
