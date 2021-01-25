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

#include "plotswidget.h"

PlotsWidget::PlotsWidget(QWidget* parent) : QWidget(parent)
{
    allocatePixelsArray();

    rgbWidget = new RGBWidget;

    tabWidget = new QTabWidget;
    tabWidget->addTab(rgbWidget, "RGB");

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(tabWidget);

    setLayout(layout);
    resize(512, 512);
}

PlotsWidget::~PlotsWidget()
{
    delete rgbWidget;

    delete context;
    delete surface;

    if (pixels) delete pixels;
    if (prevPixels) delete prevPixels;
}

void PlotsWidget::init(QOpenGLContext *mainContext)
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
    context->doneCurrent();
}

void PlotsWidget::updateOutputTextureID(GLuint id)
{
    outputTextureID = id;
}

void PlotsWidget::updatePlot()
{
    tabWidget->currentWidget()->update();
}

void PlotsWidget::allocatePixelsArray()
{
    if (pixels) delete pixels;
    pixels = new GLfloat[FBO::width * FBO::height * 3];

    if (prevPixels) delete prevPixels;
    prevPixels = new GLfloat[FBO::width * FBO::height * 3];
}

void PlotsWidget::getPixels()
{
    context->makeCurrent(surface);

    glBindTexture(GL_TEXTURE_2D, outputTextureID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, pixels);

    context->doneCurrent();

    if (tabWidget->isTabVisible(0))
        rgbWidget->setPixels(pixels);
}
