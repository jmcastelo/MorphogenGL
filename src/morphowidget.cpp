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

    frameTransform.scale(factor, factor);

    if (frameTransform.m11() > 1.0 || frameTransform.m22() > 1.0)
        frameTransform.reset();

    frame = frameTransform.mapRect(image);

    // This transform is used to zoom following the pointer

    QTransform superscale = QTransform().scale((frameBefore.width() - frame.width()) / (frameTransform.m11() * width()), (frameBefore.height() - frame.height()) / (frameTransform.m22() * height()));
    QPointF increment = superscale.map(event->position());

    frame = frameTransform.translate(increment.x(), increment.y()).mapRect(image);

    // Keep frame within image

    if (frame.y() < image.y())
        frame = frameTransform.translate(0.0, (image.y() - frame.y()) / frameTransform.m22()).mapRect(image);
    if (frame.y() + frame.height() > image.y() + image.height())
        frame = frameTransform.translate(0.0, (image.y() + image.height() - frame.y() - frame.height()) / frameTransform.m22()).mapRect(image);
    if (frame.x() < image.x())
        frame = frameTransform.translate((image.x() - frame.x()) / frameTransform.m11(), 0.0).mapRect(image);
    if (frame.x() + frame.width() > image.x() + image.width())
        frame = frameTransform.translate((image.x() + image.width() - frame.x() - frame.width()) / frameTransform.m11(), 0.0).mapRect(image);

    // Cursor

    QPointF point = selectedPointTransform.map(selectedPoint);
    cursor.setX(2.0 * ((point.x() - frame.left()) / frame.width() - 0.5));
    cursor.setY(2.0 * (0.5 - (point.y() - frame.top()) / frame.height()));
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

            QPointF delta = QTransform().scale(static_cast<qreal>(image.width()) / width(), static_cast<qreal>(image.height()) / height()).map(prevPos - event->localPos());

            frame = frameTransform.translate(delta.x(), delta.y()).mapRect(image);

            if (frame.top() < image.top() || frame.bottom() > image.bottom())
                frame = frameTransform.translate(0.0, -delta.y()).mapRect(image);
            if (frame.left() < image.left() || frame.right() > image.right())
                frame = frameTransform.translate(-delta.x(), 0.0).mapRect(image);

            prevPos = event->pos();

            // Cursor

            if (prevFrame != frame)
            {
                QPointF point = selectedPointTransform.translate(static_cast<qreal>(prevFrame.left() - frame.left()) / image.width(), static_cast<qreal>(prevFrame.top() - frame.top()) / image.height()).map(selectedPoint);
                cursor.setX(2.0 * ((point.x() - frame.left()) / frame.width() - 0.5));
                cursor.setY(2.0 * (0.5 - (point.y() - frame.top()) / frame.height()));
                updateCursor();

                prevFrame = frame;
            }
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
            prevFrame = frame;
        }
        else if (event->modifiers() == Qt::ControlModifier)
        {
            setSelectedPoint(event->localPos());
        }
    }

    event->accept();
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

void MorphoWidget::resetZoom(int newWidth, int newHeight)
{
    QRect oldImage = image;

    image = QRect(0, 0, newWidth, newHeight);

    qreal scaleX = static_cast<qreal>(newWidth) / oldImage.width();
    qreal scaleY = static_cast<qreal>(newHeight) / oldImage.height();

    frameTransform.setMatrix(frameTransform.m11(), 0.0, 0.0, 0.0, frameTransform.m22(), 0.0, scaleX * frameTransform.dx(), scaleY * frameTransform.dy(), 1.0);

    frame = frameTransform.mapRect(image);

    selectedPoint = QTransform().scale(scaleX, scaleY).map(selectedPoint);

    selectedPointTransform.setMatrix(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, scaleX * selectedPointTransform.dx(), scaleY * selectedPointTransform.dy(), 1.0);

    QPointF selPoint = selectedPointTransform.map(selectedPoint);
    cursor.setX(2.0 * ((selPoint.x() - frame.left()) / frame.width() - 0.5));
    cursor.setY(2.0 * (0.5 - (selPoint.y() - frame.top()) / frame.height()));
    updateCursor();

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
}

void MorphoWidget::updateOutputTextureID(GLuint id)
{
    outputTextureID = id;
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
