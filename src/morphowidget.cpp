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

#include "morphowidget.h"

MorphoWidget::MorphoWidget()
{
    // Timer for render update

    timer = new QTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    timer->setInterval(20);

    connect(timer, &QTimer::timeout, this, QOverload<>::of(&MorphoWidget::update));

    // Construct control widget an pass this MorphoWidget to it

    controlWidget = new ControlWidget(this);

    // Set icons

    setWindowIcon(QIcon(":/icons/morphogengl.png"));
    controlWidget->setWindowIcon(QIcon(":/icons/morphogengl.png"));

    // Arrange both widgets side by side and centered on the screen

    resize(512, 512);

    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), QGuiApplication::screens().first()->geometry()));
    setGeometry(geometry().x() - controlWidget->width() / 2, geometry().y(), width(), height());
    controlWidget->setGeometry(geometry().x() + width(), geometry().y(), controlWidget->width(), controlWidget->height());
    controlWidget->show();

    // Signals + Slots

    connect(controlWidget, &ControlWidget::imageSizeChanged, [=](){ srcX0 = 0; srcY0 = 0; srcX1 = FBO::width; srcY1 = FBO::height; });
}

MorphoWidget::~MorphoWidget()
{
    makeCurrent();
    glDeleteFramebuffers(1, &fbo);
    delete controlWidget;
    doneCurrent();
}

void MorphoWidget::closeEvent(QCloseEvent* event)
{
    timer->stop();
    controlWidget->close();
    event->accept();
}

void MorphoWidget::wheelEvent(QWheelEvent* event)
{
    // scale > 0 if zoom in, scale < 0 if zoom out

    float scale = event->angleDelta().y() / 1200.0f;

    // Translation increments

    float deltaX0 = scale * (srcX1 - srcX0) * 0.5f;
    float deltaY0 = scale * (srcY1 - srcY0) * 0.5f;
    float deltaX1 = scale * (srcX0 - srcX1) * 0.5f;
    float deltaY1 = scale * (srcY0 - srcY1) * 0.5f;

    // Translated points must define rectangle greater than or equal to one texel

    if (srcX1 + deltaX1 - srcX0 - deltaX0 >= 1.0f && srcY1 + deltaY1 - srcY0 - deltaY0 >= 1.0f)
    {
        // Check boundaries

        if (srcX0 + deltaX0 >= 0.0f) srcX0 += deltaX0; else srcX0 = 0;
        if (srcY0 + deltaY0 >= 0.0f) srcY0 += deltaY0; else srcY0 = 0;

        if (srcX1 + deltaX1 <= FBO::width) srcX1 += deltaX1; else srcX1 = FBO::width;
        if (srcY1 + deltaY1 <= FBO::height) srcY1 += deltaY1; else srcY1 = FBO::height;
    }

    event->accept();
}

void MorphoWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        // Translation increment scaled from screen to texture coordinates

        deltaX +=  (event->x() - xPrev) * static_cast<float>(srcX1 - srcX0) / width();

        float deltaXFract, deltaXInt;
        deltaXFract = modf(deltaX, &deltaXInt);

        // Must be at least one texel wide

        if (abs(deltaXInt) >= 1.0f)
        {
            // Check boundaries

            if (srcX0 - deltaXInt >= 0 && srcX1 - deltaXInt <= FBO::width)
            {
                // Increment by integer number of texels

                srcX0 -= deltaXInt;
                srcX1 -= deltaXInt;
            }

            // Keep remaining fractional translation

            deltaX = deltaXFract;
        }

        xPrev = event->x();

        // Translation increment scaled from screen to texture coordinates

        deltaY += (event->y() - yPrev) * static_cast<float>(srcY1 - srcY0) / height();

        float deltaYFract, deltaYInt;
        deltaYFract = modf(deltaY, &deltaYInt);

        // Must be at least one texel wide

        if (abs(deltaYInt) >= 1.0f)
        {
            // Check boundaries

            if (srcY0 + deltaYInt >= 0 && srcY1 + deltaYInt <= FBO::height)
            {
                // Increment by integer number of texels

                srcY0 += deltaYInt;
                srcY1 += deltaYInt;
            }

            // Keep remaining fractional translation

            deltaY = deltaYFract;
        }

        yPrev = event->y();
    }

    event->accept();
}

void MorphoWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        deltaX = 0.0f;
        deltaY = 0.0f;

        xPrev = event->x();
        yPrev = event->y();
    }
}

void MorphoWidget::initializeGL()
{
    initializeOpenGLFunctions();

    //qDebug () << (const char*)context()->functions()->glGetString(GL_VERSION);

    // Setup

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glGenFramebuffers(1, &fbo);

    controlWidget->generator->init(context());

    // Start timer

    timer->start();
}

void MorphoWidget::paintGL()
{  
    // Bind fbo as read frame buffer

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Set generator's output texture as fbo's texture

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, controlWidget->generator->getOutputTextureID(), 0);

    // Render to default frame buffer (screen) from fbo

    glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, 0, 0, width(), height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

    controlWidget->generator->iterate();

    if (controlWidget->generator->isRecording())
    {
        controlWidget->generator->record();
        emit frameRecorded();
    }

    // Compute iteration time

    if (controlWidget->generator->active && controlWidget->generator->getIterationNumber() % 10 == 0)
    {
        end = std::chrono::steady_clock::now();
        auto iterationTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        start = std::chrono::steady_clock::now();

        emit iterationTimeMeasured(iterationTime);
    }

    if (controlWidget->generator->active)
        emit iterationPerformed();
}

void MorphoWidget::resizeGL(int width, int height)
{
    emit screenSizeChanged(width, height);
}

void MorphoWidget::setStartTime()
{
    start = std::chrono::steady_clock::now();
}
