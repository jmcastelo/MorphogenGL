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

PlotsWidget::PlotsWidget(int width, int height, QWidget* parent) : QWidget(parent)
{
    allocatePixelsArray(width, height);

    rgbWidget = new RGBWidget;
    returnMap = new ReturnMap(width, height);

    tabWidget = new QTabWidget;
    tabWidget->addTab(returnMap, "Return map");
    tabWidget->addTab(rgbWidget, "RGB Scatter plot");

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(tabWidget);

    connect(this, &PlotsWidget::setSelectedPoint, returnMap, &ReturnMap::setPoint);

    setLayout(layout);
    resize(512, 512);
}

PlotsWidget::~PlotsWidget()
{
    delete rgbWidget;

    delete context;
    delete surface;

    if (pixels) delete pixels;
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

void PlotsWidget::setTextureID(GLuint id)
{
    textureID = id;
}

bool PlotsWidget::plotsActive()
{
    return returnMap->active() || rgbWidget->isVisible();
}

void PlotsWidget::allocatePixelsArray(int width, int height)
{
    if (pixels) delete pixels;
    pixels = new GLfloat[width * height * 3];
}

void PlotsWidget::setImageSize(int width, int height)
{
    allocatePixelsArray(width, height);
    returnMap->setImageSize(width, height);
}

void PlotsWidget::getPixels()
{
    context->makeCurrent(surface);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    context->doneCurrent();

    // Update rgb widget only if visible

    if (tabWidget->currentWidget() == rgbWidget)
        rgbWidget->setPixels(pixels);

    if (returnMap->active())
        returnMap->setPixels(pixels);
}
