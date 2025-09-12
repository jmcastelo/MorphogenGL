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

#include <QApplication>
#include <QMessageBox>



GLuint FBO::width = 2048;
GLuint FBO::height = 2048;

TextureFormat FBO::texFormat = TextureFormat::RGBA8;



FBO::FBO(QOpenGLContext* shareContext)
{
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

    // Old size

    widthOld = width;
    heightOld = height;

    // Frame buffer object

    generateFramebuffer(mFbo, mTextureId);

    // Sampler

    glGenSamplers(1, &mSamplerId);
    glSamplerParameteri(mSamplerId, GL_TEXTURE_MIN_FILTER, mMinMagFilter);
    glSamplerParameteri(mSamplerId, GL_TEXTURE_MAG_FILTER, mMinMagFilter);

    mContext->doneCurrent();
}



FBO::~FBO()
{
    mContext->makeCurrent(mSurface);

    glDeleteFramebuffers(1, &mFbo);
    glDeleteTextures(1, &mTextureId);

    mContext->doneCurrent();

    delete mContext;
    delete mSurface;
}



GLuint FBO::fboId() const
{
    return mFbo;
}



GLuint FBO::textureId() const
{
    return mTextureId;
}


void FBO::generateFramebuffer(GLuint& framebuffer, GLuint& texture)
{
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLenum>(texFormat), width, height, 0, getFormat(static_cast<GLenum>(texFormat)), GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        qDebug() << glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void FBO::setMinMagFilter(GLenum filter)
{
    mMinMagFilter = filter;
    
    mContext->makeCurrent(mSurface);

    glSamplerParameteri(mSamplerId, GL_TEXTURE_MIN_FILTER, filter);
    glSamplerParameteri(mSamplerId, GL_TEXTURE_MAG_FILTER, filter);

    mContext->doneCurrent();
}



void FBO::setTextureFormat()
{
    mContext->makeCurrent(mSurface);

    // FBO

    GLuint fbo;
    GLuint textureId;
    generateFramebuffer(fbo, textureId);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &textureId);

    mTextureId = textureId;
    mFbo = fbo;

    mContext->doneCurrent();
}



GLenum FBO::getFormat(GLenum format)
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




void FBO::resize()
{
    mContext->makeCurrent(mSurface);

    // Create FBO with new size texture
    // Blit old FBO to new FBO

    GLuint fbo;
    GLuint textureId;
    generateFramebuffer(fbo, textureId);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    glBlitFramebuffer(0, 0, widthOld, heightOld, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Delete old FBO, keeping new FBO

    glDeleteFramebuffers(1, &mFbo);
    glDeleteTextures(1, &mTextureId);

    mTextureId = textureId;
    mFbo = fbo;

    mContext->doneCurrent();

    // Keep old size

    widthOld = width;
    heightOld = height;
}



/*void FBO::draw()
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

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindSampler(0, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    vao->release();
    program->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    context->doneCurrent();
}*/



void FBO::blit(GLuint fbo)
{
    mContext->makeCurrent(mSurface);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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



void FBO::clear()
{
    mContext->makeCurrent(mSurface);

    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    mContext->doneCurrent();
}



QImage FBO::outputImage()
{
    mContext->makeCurrent(mSurface);

    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    GLenum status;
    do {
        status = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000);
    } while (status == GL_TIMEOUT_EXPIRED);

    glDeleteSync(fence);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFbo);

    glViewport(0, 0, width, height);

    QImage image(width, height, QImage::Format_RGBA8888);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    mContext->doneCurrent();

    return image;
}
