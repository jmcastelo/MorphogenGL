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

#include "morphowidget.h"

MorphoWidget::MorphoWidget()
{
    // Widget setup

    resize(512, 512);

    // Timer for render update

    timer = new QTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    timer->setInterval(20);

    connect(timer, &QTimer::timeout, this, QOverload<>::of(&MorphoWidget::update));

    // Construct control widget an pass this MorphoWidget to it

    controlWidget = new ControlWidget(this);

    // Arrange both widgets side by side and centered on the screen

    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));
    setGeometry(geometry().x() - controlWidget->width() / 2, geometry().y(), width(), height());
    controlWidget->setGeometry(geometry().x() + width(), geometry().y(), controlWidget->width(), controlWidget->height());
    controlWidget->show();
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

void MorphoWidget::initializeGL()
{
    initializeOpenGLFunctions();

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

    glBlitFramebuffer(0, 0, FBO::width, FBO::height, 0, 0, width(), height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

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