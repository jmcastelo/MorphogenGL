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

#pragma once

#include <QGraphicsObject>
#include <QVector>
#include <QMenu>
#include <QAction>
#include <QUuid>

class GraphWidget;
class Edge;
class GeneratorGL;

// Abstract node

class Node : public QGraphicsObject
{
    Q_OBJECT

public:
    QUuid id;
    QString name;

    bool marked = false;

    Node(GraphWidget* graphWidget, QString name);
    ~Node();

    void addEdge(Edge *edge);
    void removeEdge(Edge *edge);
    QVector<Edge *> edges() const;

    bool connectedTo(Node *node);

    QRectF textBoundingRect() const;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

public slots:
    void setAsOutput();
    void selectToConnect();
    void copy();
    void remove();

protected:
    GraphWidget* graph;
    QVector<Edge *> edgeList;

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override = 0;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override = 0;

private:
    qreal ellipseMargin = 10.0;
    qreal penSize = 2.0;
};

// Operation node

class OperationNode : public Node
{
    Q_OBJECT

public:
    OperationNode(GraphWidget* graphWidget, QString name);
    OperationNode(GraphWidget* graphWidget, QString name, QUuid uuid, QPointF pos);
    OperationNode(const OperationNode& node);
    ~OperationNode();

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    bool hasInputs();

    void renameOperation(QString newName);

public slots:
    void setOperation(QAction *action);
    void enableOperation(bool checked);
    void equalizeBlendFactors();
    void clear();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

// Seed node

class SeedNode : public Node
{
    Q_OBJECT

public:
    SeedNode(GraphWidget* graphWidget, QString name = "Seed");
    SeedNode(GraphWidget* graphWidget, QUuid uuid, QPointF pos, QString name = "Seed");
    SeedNode(const SeedNode& node);
    ~SeedNode();

    enum { Type = UserType + 2 };
    int type() const override { return Type; }

public slots:
    void draw();
    void setType(QAction* action);
    void loadImage();
    void setFixed(bool fixed);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
};
