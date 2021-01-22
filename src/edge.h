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

class GraphWidget;
class Node;
class Cycle;

class Edge : public QGraphicsObject
{
    Q_OBJECT

public:
    Edge(GraphWidget* graphWidget, Node *sourceNode, Node *destNode);

    Node *sourceNode() const;
    Node *destNode() const;

    void adjust();

    enum { Type = UserType + 3 };
    int type() const override { return Type; }

    int linkOffset = 0;

    QPointF srcPoint() { return sourcePoint; }
    QPointF dstPoint() { return destPoint; }

    bool isPredge() { return predge; }
    void setPredge(bool set);

    QVector<Cycle*> cycles() const { return cycleList; }
    void addCycle(Cycle* cycle) { cycleList.push_back(cycle); }
    void clearCycles() { cycleList.clear(); }

    void drawBlendFactor(bool draw) { paintBlendFactor = draw; }

    QRectF boundingRect() const override;

protected:
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    GraphWidget* graph;
    Node *source, *dest;

    QPointF sourcePoint;
    QPointF destPoint;

    QVector<Cycle*> cycleList;

    bool predge = false;

    qreal arrowSize = 10;

    bool paintBlendFactor = false;

    QPointF intersectionPoint(Node *node, Node *other, QPointF offset, QLineF line);
    void setLinkOffset();

    void setAsPredge();
    void setAsEdge();

    void setBlendFactor();

private slots:
    void remove();
};
