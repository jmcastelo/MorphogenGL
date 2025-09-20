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



Seed::Seed(GLenum texFormat, GLuint width, GLuint height, QOpenGLContext* shareContext)
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

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Framebuffer object

    glGenFramebuffers(1, &mOutFbo);

    // Vertex array object

    glGenVertexArrays(1, &mVao);

    // Vertex buffer object: vertices positions

    glGenBuffers(1, &mVboPos);

    // Initialize shader program

    mRandomProgram = new QOpenGLShaderProgram();
    setRandomProgram();

    mContext->doneCurrent();

    // Generate textures with format and size given externally

    genTextures(texFormat, width, height);

    // Clear texture

    clearTexture(mClearTexId);

    // Setup VAO

    setVao(width, height);

    // Texture to store loaded image

    mImageTex = new QOpenGLTexture(QOpenGLTexture::Target2D);
}



Seed::Seed(GLenum texFormat, GLuint width, GLuint height, const Seed& seed) :
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

    // Framebuffer object

    glGenFramebuffers(1, &mOutFbo);

    // Initialize shader program

    mRandomProgram = new QOpenGLShaderProgram();
    setRandomProgram();

    mContext->doneCurrent();

    // Generate textures with format and size given externally

    genTextures(texFormat, width, height);

    // Clear texture

    clearTexture(mClearTexId);

    // Setup VAO

    setVao(width, height);

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



QList<GLuint*> Seed::textureIds()
{
    return QList<GLuint*> { &mRandomTexId, &mClearTexId };
}



void Seed::setVao(GLuint width, GLuint height)
{
    // Recompute vertices coordinates

    GLfloat w = static_cast<GLfloat>(width);
    GLfloat h = static_cast<GLfloat>(height);

    GLfloat vertCoords[] = {
        -0.5f * w, -0.5f * h, // Bottom-left
        0.5f * w, -0.5f * h, // Bottom-right
        -0.5f * w, 0.5f * h, // Top-left
        0.5f * w, 0.5f * h // Top-right
    };

    mContext->makeCurrent(mSurface);

    glViewport(0, 0, width, height);

    glBindVertexArray(mVao);

    glBindBuffer(GL_ARRAY_BUFFER, mVboPos);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertCoords), vertCoords, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    mContext->doneCurrent();
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
    setOutTexture(true);
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
        {
            drawRandom(mType == 1);
            mOutTexId = mRandomTexId;
        }
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

    mContext->makeCurrent(mSurface);

    foreach (GLuint* texId, textureIds())
    {
        glGenTextures(1, texId);
        glBindTexture(GL_TEXTURE_2D, *texId);
        glTexStorage2D(GL_TEXTURE_2D, 1, texFormat, width, height);
        glTexParameteri(*texId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(*texId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    mContext->doneCurrent();
}



void Seed::clearTexture(GLuint texId)
{
    mContext->makeCurrent(mSurface);

    glBindFramebuffer(GL_FRAMEBUFFER, mOutFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);

    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    mContext->doneCurrent();
}


void Seed::setRandomProgram()
{
    if (!mRandomProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/random.vert"))
        qDebug() << "Vertex shader error:\n" << mRandomProgram->log();
    if (!mRandomProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/random.frag"))
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

    glClear(GL_COLOR_BUFFER_BIT);

    mRandomProgram->bind();

    std::uniform_real_distribution<float> distribution(0, 1);
    float randomNumber = distribution(mGenerator);

    mRandomProgram->setUniformValue("randomNumber", randomNumber);
    mRandomProgram->setUniformValue("grayscale", grayscale);

    glBindVertexArray(mVao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    mRandomProgram->release();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    mContext->doneCurrent();
}
