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

#include "rgbwidget.h"
#include "heart.h"

RGBWidget::RGBWidget(Heart *theHeart, QWidget* parent) : QOpenGLWidget(parent), heart { theHeart }
{
    setWindowIcon(QIcon(":/icons/morphogengl.png"));
    resize(512, 512);
}

RGBWidget::~RGBWidget()
{
    if (vao && vao->isCreated()) vao->destroy();
    if (vbo && vbo->isCreated()) vbo->destroy();

    if (program) delete program;
    if (vao) delete vao;
    if (vbo) delete vbo;

    if (pixels) delete pixels;
}

void RGBWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    program = new QOpenGLShaderProgram();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/rgb.vert"))
        qDebug() << "Vertex shader error:\n" << program->log();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/rgb.frag"))
        qDebug() << "Fragment shader error:\n" << program->log();
    if (!program->link())
        qDebug() << "Shader link error:\n" << program->log();

    vao = new QOpenGLVertexArrayObject();
    vao->create();

    vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vbo->create();
    vbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);

    allocatePixelsArray();

    computeTransformMatrix();
}

void RGBWidget::paintGL()
{
    glViewport(0, 0, width(), height());
    glClear(GL_COLOR_BUFFER_BIT);

    program->bind();
    vao->bind();

    glDrawArrays(GL_POINTS, 0, FBO::width * FBO::height);

    vao->release();
    program->release();
}

void RGBWidget::resizeGL(int width, int height)
{
    computeTransformMatrix();
}

void RGBWidget::allocatePixelsArray()
{
    if (pixels) delete pixels;
    pixels = new GLfloat[FBO::width * FBO::height * 3];
}

void RGBWidget::getPixels()
{
    makeCurrent();

    glBindTexture(GL_TEXTURE_2D, heart->getGeneratorOutputTextureID());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, pixels);

    vao->bind();
    
    vbo->bind();
    vbo->allocate(pixels, FBO::width * FBO::height * 3 * sizeof(GLfloat));

    program->bind();

    int posLocation = program->attributeLocation("pos");
    program->setAttributeBuffer(posLocation, GL_FLOAT, 0, 3);
    program->enableAttributeArray(posLocation);

    vao->release();
    vbo->release();
    program->release();

    doneCurrent();
}

void RGBWidget::computeTransformMatrix()
{
    projection.setToIdentity();
    projection.perspective(60.0f, static_cast<float>(width()) / static_cast<float>(height()), 0.01f, 10.0f);
    
    QVector3D eye = rotation * QVector3D(0.0f, 0.0f, 2.0f);
    QVector3D up = rotation * QVector3D(0.0f, 1.0f, 0.0f);

    QMatrix4x4 view;
    view.setToIdentity();
    view.lookAt(eye, QVector3D(0.0f, 0.0f, 0.0f), up);
    
    QMatrix4x4 model;
    model.setToIdentity();
    model.scale(scale);
    model.translate(-0.5f, -0.5f, -0.5f);

    QMatrix4x4 transform = projection * view * model;

    program->bind();
    program->setUniformValue("transform", transform);
    program->release();

    update();
}

void RGBWidget::closeEvent(QCloseEvent* event)
{
    emit closing();
    event->accept();
}

void RGBWidget::wheelEvent(QWheelEvent* event)
{
    scale += event->angleDelta().y() / 6000.0f;
    if (scale < 0.0f) scale = 0.0f;
    computeTransformMatrix();
}

void RGBWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        prevPos = event->pos();
    }
}

void RGBWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        if (!prevPos.isNull() && prevPos != event->pos())
        {
            QVector3D from = getArcBallVector(prevPos);
            QVector3D to = getArcBallVector(event->pos());

            rotationQuaternion = QQuaternion::rotationTo(from, to);
            rotationQuaternion.normalize();

            rotation.rotate(rotationQuaternion);

            computeTransformMatrix();
        }

        prevPos = event->pos();
    }
}

QVector3D RGBWidget::getArcBallVector(const QPoint& screenPoint)
{
    QVector3D vector = QVector3D(1.0f - 2.0f * screenPoint.x() / width(), 2.0f * screenPoint.y() / height() - 1.0f, 0.0f);

    float lengthSquared = vector.lengthSquared();

    if (lengthSquared <= 1.0f)
        vector.setZ(sqrt(1.0f - lengthSquared));
    else
        vector.normalize();

    return vector;
}