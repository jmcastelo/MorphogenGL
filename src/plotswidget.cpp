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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>



PlotsWidget::PlotsWidget(GLuint width, GLuint height, QWidget* parent) : QWidget(parent)
{
    rgbWidget = new RGBWidget;

    selectPathComboBox = new QComboBox;
    selectPathComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QPushButton* enableButton = new QPushButton(QIcon(QPixmap(":/icons/circle-green.png")), "");
    enableButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    enableButton->setCheckable(true);

    numItsLineEdit = new QLineEdit;
    QIntValidator* numItsValidator = new QIntValidator(2, 999999, numItsLineEdit);
    numItsLineEdit->setValidator(numItsValidator);
    numItsLineEdit->setText(QString::number(numIts));

    QPushButton* addPathButton = new QPushButton(QIcon(QPixmap(":/icons/list-add.png")), "");
    addPathButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QPushButton* removePathButton = new QPushButton(QIcon(QPixmap(":/icons/list-remove.png")), "");
    removePathButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    xCoordLineEdit = new QLineEdit;
    xCoordValidator = new QIntValidator(0, imageWidth - 1, xCoordLineEdit);
    xCoordLineEdit->setValidator(xCoordValidator);

    yCoordLineEdit = new QLineEdit;
    yCoordValidator = new QIntValidator(0, imageHeight - 1, xCoordLineEdit);
    yCoordLineEdit->setValidator(yCoordValidator);

    QPushButton* viewSourcePushButton = new QPushButton(QIcon(QPixmap(":/icons/eye.png")), "");
    viewSourcePushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    viewSourcePushButton->setCheckable(true);

    QHBoxLayout* controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(enableButton);
    controlsLayout->addWidget(numItsLineEdit);
    controlsLayout->addWidget(addPathButton);
    controlsLayout->addWidget(removePathButton);
    controlsLayout->addWidget(selectPathComboBox);
    controlsLayout->addWidget(xCoordLineEdit);
    controlsLayout->addWidget(yCoordLineEdit);
    controlsLayout->addWidget(viewSourcePushButton);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(controlsLayout);
    layout->addWidget(rgbWidget);

    connect(enableButton, &QPushButton::toggled, this, [&](bool checked){ enabled = checked; });
    connect(addPathButton, &QPushButton::clicked, this, &PlotsWidget::addColorPath);
    connect(removePathButton, &QPushButton::clicked, this, &PlotsWidget::removeColorPath);
    connect(viewSourcePushButton, &QPushButton::toggled, this, &PlotsWidget::drawCursor);
    connect(selectPathComboBox, &QComboBox::activated, this, &PlotsWidget::setControls);

    connect(numItsLineEdit, &QLineEdit::editingFinished, this, &PlotsWidget::setNumIts);

    setImageSize(width, height);

    cursor.setX(width / 2);
    cursor.setY(height / 2);

    addColorPath();

    setLayout(layout);
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



void PlotsWidget::updatePlots()
{
    if (enabled)
    {
        getPixels();

        addColorPoint();
        setVertices();

        rgbWidget->setPixels(pixels);

        for (int index = 0; index < colorPaths.size(); index++)
        {
            if (!colorPaths[index].linesEmpty())
            {
                rgbWidget->setNumVertices(numVertices);
                rgbWidget->setLines(allVertices);
            }
        }
    }
}



void PlotsWidget::setTextureID(GLuint id)
{
    textureID = id;
}



void PlotsWidget::allocatePixelsArray(GLuint width, GLuint height)
{
    if (pixels) delete pixels;
    pixels = new GLfloat[width * height * 3];
}



void PlotsWidget::setImageSize(int width, int height)
{
    imageWidth = width;
    imageHeight = height;
    allocatePixelsArray(width, height);
    xCoordValidator->setTop(width - 1);
    yCoordValidator->setTop(height - 1);
}



void PlotsWidget::getPixels()
{
    context->makeCurrent(surface);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    context->doneCurrent();
}



void PlotsWidget::setSelectedPoint(QPoint point)
{
    checkPoint(point);
    cursor = point;

    int index = selectPathComboBox->currentIndex();
    if (index >= 0)
    {
        colorPaths[index].setSource(point);
        xCoordLineEdit->setText(QString::number(point.x()));
        yCoordLineEdit->setText(QString::number(point.y()));
    }
}



void PlotsWidget::transformSources(QTransform transform)
{
    for (ColorPath &path : colorPaths)
    {
        QPoint source = transform.map(path.source());
        checkPoint(source);
        path.setSource(source);
    }

    setControls(selectPathComboBox->currentIndex());
}



void PlotsWidget::checkPoint(QPoint &point)
{
    if (point.x() < 0)
        point.setX(0);
    if (point.y() < 0)
        point.setY(0);
    if (point.x() >= static_cast<int>(imageWidth))
        point.setX(imageWidth - 1);
    if (point.y() >= static_cast<int>(imageHeight))
        point.setY(imageHeight - 1);
}



void PlotsWidget::addColorPath()
{
    colorPaths.append(ColorPath(cursor, numIts));

    selectPathComboBox->addItem(QString::number(colorPaths.size()));
    int index = selectPathComboBox->count() - 1;
    selectPathComboBox->setCurrentIndex(index);

    setControls(index);
}



void PlotsWidget::removeColorPath()
{
    int index = selectPathComboBox->currentIndex();
    if (index >= 0 && colorPaths.size() > 1)
    {
        selectPathComboBox->removeItem(index);
        colorPaths.removeAt(index);
        for (int i = 0; i < colorPaths.size(); i++)
            selectPathComboBox->setItemText(i, QString::number(i + 1));
        setControls(selectPathComboBox->currentIndex());
    }
}



void PlotsWidget::setControls(int index)
{
    xCoordLineEdit->setText(QString::number(colorPaths[index].source().x()));
    yCoordLineEdit->setText(QString::number(colorPaths[index].source().y()));

    cursor = colorPaths[index].source();

    emit selectedPointChanged(cursor);
}



void PlotsWidget::setNumIts()
{
    numIts = numItsLineEdit->text().toInt();

    for (ColorPath &path : colorPaths)
        path.setMaxNumPoints(numIts);
}



void PlotsWidget::addColorPoint()
{
    for (ColorPath &path : colorPaths)
    {
        int row = path.source().y() * imageWidth * 3;
        int col = path.source().x() * 3;

        path.addPoint(pixels[row + col], pixels[row + col + 1], pixels[row + col + 2]);
    }
}



void PlotsWidget::setVertices()
{
    allVertices.clear();
    numVertices.clear();

    foreach(ColorPath path, colorPaths)
    {
        allVertices += path.lines();
        numVertices.append(path.linesSize());
    }
}
