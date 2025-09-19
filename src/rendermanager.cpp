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



#include "rendermanager.h"

#include <QApplication>
#include <QMessageBox>



RenderManager::RenderManager(){}



void RenderManager::init(QOpenGLContext* shareContext)
{
    mShareContext = shareContext;

    // Create context

    mContext = new QOpenGLContext();
    mContext->setFormat(shareContext->format());
    mContext->setShareContext(shareContext);
    mContext->create();

    mSurface = new QOffscreenSurface();
    mSurface->setFormat(shareContext->format());
    mSurface->create();

    mContext->makeCurrent(mSurface);

    initializeOpenGLFunctions();

    // Framebuffer objects

    glGenFramebuffers(1, &mOutFbo);
    glGenFramebuffers(1, &mReadFbo);
    glGenFramebuffers(1, &mDrawFbo);

    // Vertex array object

    glGenVertexArrays(1, &mVao);

    // Vertex buffer object: vertices positions

    glGenBuffers(1, &mVboPos);

    // Vertex buffer object: texture coordinates

    glGenBuffers(1, &mVboTex);

    // Setup VAO

    GLfloat texCoords[] = {
        0.0f, 0.0f,  // Bottom-left
        1.0f, 0.0f,  // Bottom-right
        0.0f, 1.0f,  // Top-left
        1.0f, 1.0f   // Top-right
    };

    glBindVertexArray(mVao);

    glBindBuffer(GL_ARRAY_BUFFER, mVboTex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    resizeVertices();

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Shader program

    mBlenderProgram = new QOpenGLShaderProgram();
    mIdentityProgram = new QOpenGLShaderProgram();

    setBlenderProgram();
    setIdentityProgram();

    // Get and clamp maximum number of texture units (sampler2D)

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &mMaxTexUnits);

    if (mMaxTexUnits > mNumTexUnits)
        mMaxTexUnits = mNumTexUnits;

    mContext->doneCurrent();
}



RenderManager::~RenderManager()
{
    mContext->makeCurrent(mSurface);

    glDeleteFramebuffers(1, &mOutFbo);
    glDeleteFramebuffers(1, &mReadFbo);
    glDeleteFramebuffers(1, &mDrawFbo);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint vbos[2] = { mVboPos, mVboTex };
    glDeleteBuffers(2, vbos);

    glDeleteVertexArrays(1, &mVao);

    delete mBlenderProgram;
    delete mIdentityProgram;

    mContext->doneCurrent();

    delete mContext;
    delete mSurface;
}



Seed* RenderManager::createNewSeed()
{
    Seed* seed = new Seed(static_cast<GLenum>(mTexFormat), mTexWidth, mTexHeight, mVao, mShareContext);

    mSeeds.append(seed);

    return seed;
}



void RenderManager::deleteSeed(Seed* seed)
{
    if (mSeeds.removeOne(seed))
        delete seed;
}



ImageOperation* RenderManager::createNewOperation()
{
    ImageOperation* operation = new ImageOperation("New Operation", static_cast<GLenum>(mTexFormat), mTexWidth, mTexHeight, mShareContext);

    mOperations.append(operation);

    return operation;
}



void RenderManager::iterate()
{
    if (!mSortedOperations.isEmpty())
    {
        mContext->makeCurrent(mSurface);

        blit();
        render();

        mContext->doneCurrent();
    }

    foreach (Seed* seed, mSeeds)
        seed->setOutTexture(false);

    mIterationNumber++;
}



GLenum RenderManager::getFormat(GLenum format)
{
    GLint redType, greenType, blueType, alphaType;

    glGetInternalformativ(GL_TEXTURE_2D, format, GL_INTERNALFORMAT_RED_TYPE, 1, &redType);
    glGetInternalformativ(GL_TEXTURE_2D, format, GL_INTERNALFORMAT_GREEN_TYPE, 1, &greenType);
    glGetInternalformativ(GL_TEXTURE_2D, format, GL_INTERNALFORMAT_BLUE_TYPE, 1, &blueType);
    glGetInternalformativ(GL_TEXTURE_2D, format, GL_INTERNALFORMAT_ALPHA_TYPE, 1, &alphaType);

    if (redType == GL_INT || redType == GL_UNSIGNED_INT ||
        greenType == GL_INT || greenType == GL_UNSIGNED_INT ||
        blueType == GL_INT || blueType == GL_UNSIGNED_INT ||
        alphaType == GL_INT || alphaType == GL_UNSIGNED_INT)
    {
        return GL_RGBA_INTEGER;
    }
    else
    {
        return GL_RGBA;
    }
}




void RenderManager::resize(GLuint width, GLuint height)
{
    mOldTexWidth = mTexWidth;
    mOldTexHeight = mTexHeight;

    mTexWidth = width;
    mTexHeight = height;

    mContext->makeCurrent(mSurface);

    glViewport(0, 0, mTexWidth, mTexHeight);

    resizeVertices();
    resizeTextures();

    mContext->doneCurrent();
}



/*void FBO::identity()
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

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindSampler(0, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    vao->release();
    identityProgram->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();
}*/



QImage RenderManager::outputImage()
{
    QImage image(mTexWidth, mTexHeight, QImage::Format_RGBA8888);

    if (mOutputTexId > 0)
    {
        mContext->makeCurrent(mSurface);

        fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        GLenum status;
        do {
            status = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000);
        } while (status == GL_TIMEOUT_EXPIRED);

        glDeleteSync(fence);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, mOutFbo);

        glReadPixels(0, 0, mTexWidth, mTexHeight, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        mContext->doneCurrent();
    }
    else
    {
        image.fill(Qt::black);
    }

    return image;
}



TextureFormat RenderManager::texFormat()
{
    return mTexFormat;
}



void RenderManager::setTextureFormat(TextureFormat format)
{
    mTexFormat = format;

    QList<GLuint> oldTexIds;

    foreach (Seed* seed, mSeeds)
        oldTexIds.append(seed->textureIds());

    foreach (ImageOperation* operation, mOperations)
        oldTexIds.append(operation->textureIds());

    mContext->makeCurrent(mSurface);

    foreach (GLuint oldTexId, oldTexIds)
    {
        GLuint newTexId = 0;
        genTexture(newTexId);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, mReadFbo);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, oldTexId, 0);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawFbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newTexId, 0);

        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        glBlitFramebuffer(0, 0, mOldTexWidth, mOldTexHeight, 0, 0, mTexWidth, mTexHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glDeleteTextures(1, &oldTexId);
        oldTexId = newTexId;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    mContext->doneCurrent();

    emit texturesChanged();
}



GLuint RenderManager::texWidth()
{
    return mTexWidth;
}



GLuint RenderManager::texHeight()
{
    return mTexHeight;
}



void RenderManager::resetIterationNumer()
{
    mIterationNumber = 0;
}



int RenderManager::iterationNumber()
{
    return mIterationNumber;
}



void RenderManager::setSortedOperations(QList<ImageOperation*> operations)
{
    mSortedOperations = operations;
}



void RenderManager::setOutputTextureId(GLuint texId)
{
    mOutputTexId = texId;
}



void RenderManager::setBlenderProgram()
{
    mBlenderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/blender.vert");
    mBlenderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/blender.frag");
    mBlenderProgram->link();

    // Set blender shader attributes

    if (mBlenderProgram->isLinked())
    {
        mBlenderProgram->bind();

        // Vertices coordinates attribute (lcoation = 0)

        mBlenderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2);
        mBlenderProgram->enableAttributeArray(0);

        // Texture coordinates attribute (lcoation = 1)

        mBlenderProgram->setAttributeBuffer(1, GL_FLOAT, 0, 2);
        mBlenderProgram->enableAttributeArray(1);

        // Input texture units (uniform sampler2D inTextures[...])

        GLint locSamplers = mBlenderProgram->uniformLocation("inTextures");
        if (locSamplers >= 0)
        {
            QList<GLint> samplers;
            for (int i = 0; i < mNumTexUnits; i++)
                samplers.append(i);

            glUniform1iv(locSamplers, mNumTexUnits, samplers.constData());
        }

        mBlenderProgram->release();
    }
}



void RenderManager::setIdentityProgram()
{
    mIdentityProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/identity.vert");
    mIdentityProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/identity.frag");
    mIdentityProgram->link();

    // Set blender shader attributes

    if (mIdentityProgram->isLinked())
    {
        mIdentityProgram->bind();

        // Vertices coordinates attribute (lcoation = 0)

        mIdentityProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2);
        mIdentityProgram->enableAttributeArray(0);

        // Texture coordinates attribute (lcoation = 1)

        mIdentityProgram->setAttributeBuffer(1, GL_FLOAT, 0, 2);
        mIdentityProgram->enableAttributeArray(1);

        // Input texture unit (uniform sampler2D inTexture)

        GLint locSampler = mIdentityProgram->uniformLocation("inTexture");
        if (locSampler >= 0)
            glUniform1i(locSampler, 0);

        mIdentityProgram->release();
    }
}



void RenderManager::verticesCoords(GLfloat& left, GLfloat& right, GLfloat& bottom, GLfloat& top)
{
    // Maintain aspect ratio

    GLfloat w = static_cast<GLfloat>(mTexWidth);
    GLfloat h = static_cast<GLfloat>(mTexHeight);

    GLfloat ratio = w / h;

    if (mTexWidth > mTexHeight)
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
}



void RenderManager::resizeVertices()
{
    // Recompute vertices coordinates

    GLfloat w = static_cast<GLfloat>(mTexWidth);
    GLfloat h = static_cast<GLfloat>(mTexHeight);

    GLfloat vertCoords[] = {
        0.0f, 0.0f, // Bottom-left
        w, 0.0f, // Bottom-right
        0.0f, h, // Top-left
        w, h // Top-right
    };

    glBindBuffer(GL_ARRAY_BUFFER, mVboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertCoords), vertCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
}



void RenderManager::genTexture(GLuint texId)
{
    // Allocated on immutable storage (glTexStorage2D)
    // To be called within active OpenGL context

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexStorage2D(GL_TEXTURE_2D, 1, static_cast<GLenum>(mTexFormat), mTexWidth, mTexHeight);
    glTexParameteri(texId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(texId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}



void RenderManager::resizeTextures()
{
    QList<GLuint> oldTexIds;

    foreach (Seed* seed, mSeeds)
        oldTexIds.append(seed->textureIds());

    foreach (ImageOperation* operation, mOperations)
        oldTexIds.append(operation->textureIds());

    foreach (GLuint oldTexId, oldTexIds)
    {
        GLuint newTexId = 0;
        genTexture(newTexId);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, mReadFbo);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, oldTexId, 0);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawFbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newTexId, 0);

        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        glBlitFramebuffer(0, 0, mOldTexWidth, mOldTexHeight, 0, 0, mTexWidth, mTexHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glDeleteTextures(1, &oldTexId);
        oldTexId = newTexId;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    emit texturesChanged();
}



void RenderManager::clearTexture(GLuint texId)
{
    mContext->makeCurrent(mSurface);

    glBindFramebuffer(GL_FRAMEBUFFER, mOutFbo);
    glBindVertexArray(mVao);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);

    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    mContext->doneCurrent();
}



void RenderManager::blit()
{
    // Expects active OpenGL context

    foreach (ImageOperation* operation, mSortedOperations)
    {
        if (operation->blitEnabled())
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mReadFbo);
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, operation->outTextureId(), 0);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawFbo);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, operation->blitTextureId(), 0);

            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);

            glBlitFramebuffer(0, 0, mTexWidth, mTexHeight, 0, 0, mTexWidth, mTexHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        }
    }
}



void RenderManager::blend(ImageOperation *operation)
{
    QList<GLuint> inputTextures = operation->inputTextures();

    int nInputs = inputTextures.size();
    if (nInputs > mMaxTexUnits)
        nInputs = mMaxTexUnits;

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, operation->blendOutTextureId(), 0);

    glClear(GL_COLOR_BUFFER_BIT);

    mBlenderProgram->bind();

    // Bind input textures

    for (int i = 0; i < nInputs; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, inputTextures[i]);
    }

    // Set number of input textures

    GLint locCount = mBlenderProgram->uniformLocation("texCount");
    glUniform1i(locCount, nInputs);

    // Set blend factors

    GLint locWeights = mBlenderProgram->uniformLocation("weights");
    QList<float> blendFactors = operation->inputBlendFactors();

    QList<float> weights(mNumTexUnits, 0.0f);
    for (int i = 0; i < nInputs; ++i)
        weights[i] = blendFactors[i];

    glUniform1fv(locWeights, mNumTexUnits, weights.constData());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Clean up

    for (int i = 0; i < nInputs; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    mBlenderProgram->release();
}



void RenderManager::renderOperation(ImageOperation* operation)
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, operation->outTextureId(), 0);

    glClear(GL_COLOR_BUFFER_BIT);

    operation->program()->bind();

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, operation->inTextureId());
    glBindSampler(0, operation->samplerId());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindSampler(0, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    operation->program()->release();
}



void RenderManager::render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mOutFbo);

    glBindVertexArray(mVao);

    foreach (ImageOperation* operation, mSortedOperations)
    {
        if (operation->blendEnabled())
            blend(operation);

        if (operation->enabled())
            renderOperation(operation);
    }

    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
