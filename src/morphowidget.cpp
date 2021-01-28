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

#include "morphowidget.h"

MorphoWidget::MorphoWidget(int width, int height, QWidget* parent) : QOpenGLWidget(parent)
{
    image = QRect(0, 0, width, height);
    frame = image;

    selectedPoint = QPointF(width / 2, height / 2);
    cursor = QPointF(0.0, 0.0);

    setWindowIcon(QIcon(":/icons/morphogengl.png"));
    
    resize(width, height);

    // Set widget on screen center

    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), QGuiApplication::screens().first()->geometry()));
}

MorphoWidget::~MorphoWidget()
{
    makeCurrent();
    glDeleteFramebuffers(1, &fbo);
    vao->destroy();
    vbo->destroy();
    doneCurrent();

    delete vao;
    delete vbo;
    delete program;
}

void MorphoWidget::closeEvent(QCloseEvent* event)
{
    emit closing();
    event->accept();
}

void MorphoWidget::wheelEvent(QWheelEvent* event)
{
    // Frame

    QRect frameBefore = frame;

    qreal factor = pow(2.0, -event->angleDelta().y() / 1200.0);

    transform.scale(factor, factor);

    if (transform.m11() > 1.0 || transform.m22() > 1.0)
        transform.reset();

    frame = transform.mapRect(image);

    // This transform is used to zoom following the pointer

    QTransform superscale((frameBefore.width() - frame.width()) / (transform.m11() * width()), 0.0, 0.0, (frameBefore.height() - frame.height()) / (transform.m22() * height()), 0.0, 0.0);
    QPointF increment = superscale.map(event->position());

    frame = transform.translate(increment.x(), increment.y()).mapRect(image);

    // Keep frame within image

    if (frame.y() < image.y())
        frame = transform.translate(0.0, (image.y() - frame.y()) / transform.m22()).mapRect(image);
    if (frame.y() + frame.height() > image.y() + image.height())
        frame = transform.translate(0.0, (image.y() + image.height() - frame.y() - frame.height()) / transform.m22()).mapRect(image);
    if (frame.x() < image.x())
        frame = transform.translate((image.x() - frame.x()) / transform.m11(), 0.0).mapRect(image);
    if (frame.x() + frame.width() > image.x() + image.width())
        frame = transform.translate((image.x() + image.width() - frame.x() - frame.width()) / transform.m11(), 0.0).mapRect(image);

    // Cursor

    QPointF point = transform.inverted().map(selectedPoint);
    cursor.setX(2.0 * (point.x() / image.width() - 0.5));
    cursor.setY(2.0 * (0.5 - point.y() / image.height()));
    updateCursor();

    event->accept();
}

void MorphoWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        if (event->modifiers() == Qt::NoModifier)
        {
            // Frame

            QPointF delta = prevPos - event->localPos();

            frame = transform.translate(delta.x(), delta.y()).mapRect(image);

            if (frame.top() < image.top() || frame.bottom() > image.bottom())
                frame = transform.translate(0.0, -delta.y()).mapRect(image);
            if (frame.left() < image.left() || frame.right() > image.right())
                frame = transform.translate(-delta.x(), 0.0).mapRect(image);

            prevPos = event->pos();

            // Cursor

            QPointF point = transform.inverted().map(selectedPoint);
            cursor.setX(2.0 * (point.x() / image.width() - 0.5));
            cursor.setY(2.0 * (0.5 - point.y() / image.height()));
            updateCursor();
        }
        else if (event->modifiers() == Qt::ControlModifier)
        {
            setSelectedPoint(event->localPos());
        }
    }

    event->accept();
}

void MorphoWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        if (event->modifiers() == Qt::NoModifier)
        {
            prevPos = event->localPos();
        }
        else if (event->modifiers() == Qt::ControlModifier)
        {
            setSelectedPoint(event->localPos());
        }
    }
}

void MorphoWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control)
    {
        drawingCursor = true;
    }
}

void MorphoWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control)
    {
        drawingCursor = false;
    }
}

void MorphoWidget::setSelectedPoint(QPointF pos)
{
    QPointF clickedPoint = QTransform().scale(1.0 / width(), 1.0 / height()).map(pos);

    if (clickedPoint.x() < 0.0)
        clickedPoint.setX(0.0);
    if (clickedPoint.x() > 1.0)
        clickedPoint.setX(1.0);
    if (clickedPoint.y() < 0.0)
        clickedPoint.setY(0.0);
    if (clickedPoint.y() > 1.0)
        clickedPoint.setY(1.0);

    selectedPoint = QTransform().translate(frame.left(), frame.top()).scale(frame.width(), frame.height()).map(clickedPoint);

    QPoint point = QPoint(floor(selectedPoint.x()), floor(selectedPoint.y()));

    // Check boundaries
    // Note: right() = left() + width() - 1, bottom() = top() + height() - 1

    if (point.x() < image.left())
        point.setX(image.left());
    if (point.x() > image.right())
        point.setX(image.right());
    if (point.y() < image.top())
        point.setY(image.top());
    if (point.y() > image.bottom())
        point.setY(image.bottom());

    emit selectedPointChanged(point);

    cursor.setX(2.0 * (clickedPoint.x() - 0.5));
    cursor.setY(2.0 * (0.5 - clickedPoint.y()));
    updateCursor();
}

void MorphoWidget::updateOutputTextureID(GLuint id)
{
    outputTextureID = id;
}

void MorphoWidget::resetZoom(int newWidth, int newHeight)
{
    image = QRect(0, 0, newWidth, newHeight);
    frame = image;

    selectedPoint = QPointF(0.5 * newWidth, 0.5 * newHeight);

    setSelectedPoint(QPointF(0.5 * width(), 0.5 * height()));

    transform.reset();
}

void MorphoWidget::updateCursor()
{
    GLfloat x = static_cast<GLfloat>(cursor.x());
    GLfloat y = static_cast<GLfloat>(cursor.y());

    GLfloat cursorVertices[] = {
        x, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        x, y, 1.0f, 1.0f, 1.0f, 1.0f,
        x, y, 1.0f, 1.0f, 1.0f, 1.0f,
        x, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, y, 1.0f, 1.0f, 1.0f, 0.0f,
        x, y, 1.0f, 1.0f, 1.0f, 1.0f,
        x, y, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, y, 1.0f, 1.0f, 1.0f, 0.0f
    };

    makeCurrent();

    vao->bind();

    vbo->bind();
    vbo->allocate(cursorVertices, sizeof(cursorVertices));

    program->bind();

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    vao->release();
    vbo->release();
    program->release();

    doneCurrent();
}

void MorphoWidget::initializeGL()
{
    initializeOpenGLFunctions();

    //qDebug () << (const char*)context()->functions()->glGetString(GL_VERSION);

    // Setup

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glGenFramebuffers(1, &fbo);

    program = new QOpenGLShaderProgram();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/cursor.vert"))
        qDebug() << "Vertex shader error:\n" << program->log();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/cursor.frag"))
        qDebug() << "Fragment shader error:\n" << program->log();
    if (!program->link())
        qDebug() << "Shader link error:\n" << program->log();

    vao = new QOpenGLVertexArrayObject();
    vao->create();

    vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vbo->create();
    vbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);

    updateCursor();

    emit openGLInitialized();
}

void MorphoWidget::paintGL()
{
    // Bind fbo as read frame buffer

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Set generator's output texture as fbo's texture

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTextureID, 0);

    // Render to default frame buffer (screen) from fbo

    glBlitFramebuffer(frame.x(), frame.y() + frame.height(), frame.x() + frame.width(), frame.y(), 0, 0, width(), height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

    if (drawingCursor)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        program->bind();
        vao->bind();

        glDrawArrays(GL_LINES, 0, 8);

        vao->release();
        program->release();

        glDisable(GL_BLEND);
    }
}

void MorphoWidget::resizeGL(int width, int height)
{
    emit screenSizeChanged(width, height);
}
