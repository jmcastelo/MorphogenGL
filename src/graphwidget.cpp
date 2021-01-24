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

#include "graphwidget.h"
#include "edge.h"
#include "node.h"
#include "cycle.h"
#include "cyclesearch.h"
#include "generator.h"

#include <math.h>

#include <QKeyEvent>
#include <QRandomGenerator>
#include <QMenu>

GraphWidget::GraphWidget(GeneratorGL* generatorGL, QWidget *parent)
    : QGraphicsView(parent) , generator { generatorGL }
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setFont(QFont("Arial", 8, QFont::Light));
    scene->setSceneRect(0.0, 0.0, 10000.0, 10000.0);
    setScene(scene);

    connect(scene, &QGraphicsScene::selectionChanged, this, &GraphWidget::newSelectedNodes);

    setDragMode(QGraphicsView::RubberBandDrag);
    setRubberBandSelectionMode(Qt::ContainsItemShape);

    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::NoFocus);

    scale(qreal(1.0), qreal(1.0));
    center = sceneRect().center();
    centerOn(sceneRect().center());
}

void GraphWidget::newSelectedNodes()
{
    QVector<OperationNode*> opNodes;
    QVector<SeedNode*> seedNodes;

    const QList<QGraphicsItem*> items = scene()->selectedItems();

    for (QGraphicsItem* item : items)
    {
        if (OperationNode *node = qgraphicsitem_cast<OperationNode*>(item))
            opNodes << node;
        else if (SeedNode *node = qgraphicsitem_cast<SeedNode*>(item))
            seedNodes << node;
    }

    if (seedNodes.size() + opNodes.size() > 1)
        emit multipleNodesSelected(!opNodes.empty());
    else if (seedNodes.size() == 1 && opNodes.empty())
        emit singleNodeSelected(seedNodes.front());
    else if (opNodes.size() == 1 && seedNodes.empty())
        emit singleNodeSelected(opNodes.front());
    else if (seedNodes.empty() && opNodes.empty())
        emit singleNodeSelected(nullptr);
}

void GraphWidget::addOperationNodeUnderCursor(QAction *action)
{
    Node *node = new OperationNode(this, action->text());
    scene()->addItem(node);
    node->setPos(mapToScene(mapFromGlobal(action->data().toPoint())));
}

void GraphWidget::addSeedNodeUnderCursor()
{
    QAction* action = qobject_cast<QAction*>(sender());
    QVariant data = action->data();

    Node *node = new SeedNode(this);
    scene()->addItem(node);
    node->setPos(mapToScene(mapFromGlobal(data.toPoint())));
}

void GraphWidget::removeNode(Node *node)
{
    for(Edge *edge: node->edges())
    {
        edge->sourceNode()->removeEdge(edge);
        edge->destNode()->removeEdge(edge);

        scene()->removeItem(edge);
        delete edge;
    }

    scene()->removeItem(node);
    delete node;

    searchElementaryCycles();
}

void GraphWidget::removeEdge(Edge *edge)
{
    generator->disconnectOperations(edge->sourceNode()->id, edge->destNode()->id);

    scene()->removeItem(edge);
    delete edge;

    searchElementaryCycles();
}

void GraphWidget::selectNodeToConnect(Node *node)
{
    selectedNode = node;
}

void GraphWidget::deselectNodeToConnect()
{
    selectedNode = nullptr;
}

bool GraphWidget::nodeSelectedToConnect()
{
    return selectedNode != nullptr;
}

bool GraphWidget::moreThanOneNode()
{
    QVector<Node*> nodes;

    const QList<QGraphicsItem*> items = scene()->items();

    for (QGraphicsItem* item : items)
    {
        if (OperationNode *node = qgraphicsitem_cast<OperationNode*>(item))
            nodes << node;
        else if (SeedNode *node = qgraphicsitem_cast<SeedNode*>(item))
            nodes << node;
    }

    return nodes.size() > 1;
}

void GraphWidget::newNodeSelected(Node* node)
{
    emit singleNodeSelected(node);
}

bool GraphWidget::nodesSelected()
{
    QVector<Node*> nodes;

    const QList<QGraphicsItem*> items = scene()->selectedItems();

    for (QGraphicsItem*item : items)
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
            nodes << node;
        else if (SeedNode* node = qgraphicsitem_cast<SeedNode*>(item))
            nodes << node;
    }

    return nodes.size() > 1;
}

bool GraphWidget::operationNodesSelected()
{
    QVector<Node*> nodes;

    const QList<QGraphicsItem*> items = scene()->selectedItems();

    for (QGraphicsItem*item : items)
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
            nodes << node;

    return nodes.size() > 1;
}

void GraphWidget::clearSelectedOperationNodes()
{
    const QList<QGraphicsItem*> items = scene()->selectedItems();

    for (QGraphicsItem*item : items)
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
            generator->clearOperation(node->id);

}

void GraphWidget::removeSelectedNodes()
{
    const QList<QGraphicsItem*> items = scene()->selectedItems();

    for (QGraphicsItem* item : items)
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
            removeNode(node);
        else if (SeedNode* node = qgraphicsitem_cast<SeedNode*>(item))
            removeNode(node);
    }

    searchElementaryCycles();
}

void GraphWidget::copyNode(Node *node)
{
    copiedNodes[0].clear();
    copiedNodes[0].insert(node->id, node);
    copyNodes(true);
}

void GraphWidget::makeNodeSnapshot()
{
    copiedNodes[0].clear();

    const QList<QGraphicsItem*> items = scene()->selectedItems();

    for (QGraphicsItem* item : items)
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
            copiedNodes[0].insert(node->id, node);
        else if (SeedNode* node = qgraphicsitem_cast<SeedNode*>(item))
            copiedNodes[0].insert(node->id, node);
    }

    copyNodes(true);
}

void GraphWidget::copyNodes(bool connectionA)
{
    // Compute coordinates relative to center of copied nodes

    QPointF center(0.0, 0.0);

    foreach (Node* node, copiedNodes[0])
        center += node->pos();

    center /= copiedNodes[0].size();

    QMap<QUuid, QPointF> relativeCoords;

    foreach (Node* node, copiedNodes[0])
        relativeCoords.insert(node->id, node->pos() - center);

    // Copy-construct new nodes
    // Isomorphism relates copied and new nodes

    QMap<QUuid, QUuid> isomorphism;

    copiedNodes[1].clear();

    foreach (Node* srcNode, copiedNodes[0])
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(srcNode))
        {
            OperationNode* dstNode  = new OperationNode(*node);
            dstNode->setPos(relativeCoords.value(node->id));
            isomorphism.insert(node->id, dstNode->id);
            copiedNodes[1].insert(dstNode->id, dstNode);
        }
        else if (SeedNode* node = qgraphicsitem_cast<SeedNode*>(srcNode))
        {
            SeedNode* dstNode  = new SeedNode(*node);
            dstNode->setPos(relativeCoords.value(node->id));
            isomorphism.insert(node->id, dstNode->id);
            copiedNodes[1].insert(dstNode->id, dstNode);
        }
    }

    // Link new nodes according to isomorphism

    newEdges.clear();

    foreach (Node* node, copiedNodes[0])
    {
        for (Edge* edge : node->edges())
        {
            QUuid sourceId = edge->sourceNode()->id;
            QUuid destId = edge->destNode()->id;

            if (copiedNodes[0].contains(sourceId) && copiedNodes[0].contains(destId))
            {
                QUuid newSourceId = isomorphism.value(sourceId);
                QUuid newDestId = isomorphism.value(destId);

                Edge* newEdge = new Edge(this, copiedNodes[1].value(newSourceId), copiedNodes[1].value(newDestId));

                if (connectionA)
                    generator->connectCopiedOperationsA(sourceId, destId, newSourceId, newDestId);
                else
                    generator->connectCopiedOperationsB(sourceId, destId, newSourceId, newDestId);

                newEdge->setPredge(edge->isPredge());

                newEdges.push_back(newEdge);
            }
        }
    }
}

void GraphWidget::pasteCopiedNodes()
{
    // Get cursor position in scene coordinate system

    QAction* action = qobject_cast<QAction*>(sender());
    QVariant data = action->data();
    QPoint cursorPos = data.toPoint();
    QPointF origin = mapToScene(mapFromGlobal(cursorPos));

    // Add items to scene

    foreach (Node* node, copiedNodes[1])
    {
        scene()->addItem(node);
        node->setPos(origin + node->pos());
    }

    for (Edge* edge : newEdges)
    {
        scene()->addItem(edge);
        edge->adjust();
    }

    generator->pasteOperations();

    copiedNodes[0] = copiedNodes[1];

    copyNodes(false);

    searchElementaryCycles();
}

void GraphWidget::connectNodes(Node *node)
{
    if (!selectedNode->connectedTo(node))
    {
        generator->connectOperations(selectedNode->id, node->id, 1.0f);
        scene()->addItem(new Edge(this, selectedNode, node));
        searchElementaryCycles();
    }

    selectedNode = nullptr;
}

void GraphWidget::setOperationParameters(QUuid id)
{
    emit showOperationParameters(id);
}

void GraphWidget::updateOperation(QUuid id)
{
    emit updateOperationParameters(id);
}

void GraphWidget::onDestroyOperationNode(QUuid id)
{
    generator->removeOperation(id);
    emit removeOperationParameters(id);
}

void GraphWidget::onDestroySeedNode(QUuid id)
{
    generator->removeSeed(id);
}

void GraphWidget::updateNodes()
{
    const QList<QGraphicsItem*> items = scene()->items();

    for (QGraphicsItem* item : items)
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
            node->update();
        else if (SeedNode* node = qgraphicsitem_cast<SeedNode*>(item))
            node->update();
    }
}

void GraphWidget::drawBlendFactors(bool draw)
{
    const QList<QGraphicsItem*> items = scene()->items();

    for (QGraphicsItem* item : items)
    {
        if (Edge* edge = qgraphicsitem_cast<Edge*>(item))
        {
            edge->drawBlendFactor(draw);
            edge->update();
        }
    }
}

QPointF GraphWidget::nodePosition(QUuid id)
{
    QPointF position;

    const QList<QGraphicsItem*> items = scene()->items();

    for (QGraphicsItem* item : items)
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
        {
            if (node->id == id)
            {
                position = node->pos();
                break;
            }
        }
        if (SeedNode* node = qgraphicsitem_cast<SeedNode*>(item))
        {
            if (node->id == id)
            {
                position = node->pos();
                break;
            }
        }
    }

    return position;
}

void GraphWidget::clearScene()
{
    scene()->clear();
}

void GraphWidget::loadSeedNode(QUuid id, QPointF position)
{
    Node *node = new SeedNode(this, id, position);
    scene()->addItem(node);
}

void GraphWidget::loadOperationNode(QUuid id, QString name, QPointF position)
{
    Node *node = new OperationNode(this, name, id, position);
    scene()->addItem(node);
}

Node* GraphWidget::getNode(QUuid id)
{
    Node* found = nullptr;

    const QList<QGraphicsItem*> items = scene()->items();

    for (QGraphicsItem* item : items)
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
        {
            if (node->id == id)
            {
                found = node;
                break;
            }
        }
        if (SeedNode* node = qgraphicsitem_cast<SeedNode*>(item))
        {
            if (node->id == id)
            {
                found = node;
                break;
            }
        }
    }

    return found;
}

void GraphWidget::connectNodes(QMap<QUuid, QMap<QUuid, InputData*>> connections)
{
    QMap<QUuid, QMap<QUuid, InputData*>>::iterator dst = connections.begin();

    while (dst != connections.end())
    {
        Node* dstNode = getNode(dst.key());

        if (dstNode)
        {
            QMap<QUuid, InputData*>::iterator src = dst.value().begin();

            while (src != dst.value().constEnd())
            {
                Node* srcNode = getNode(src.key());

                if (srcNode)
                {
                    Edge* edge = new Edge(this, srcNode, dstNode);

                    edge->setPredge(src.value()->type == InputType::Blit);

                    scene()->addItem(edge);
                }

                src++;
            }
        }

        dst++;
    }

    searchElementaryCycles();
}

void GraphWidget::markNodes(QVector<QUuid> ids)
{
    const QList<QGraphicsItem*> items = scene()->items();

    for (QGraphicsItem* item : items)
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
        {
            node->marked = false;

            for (QUuid id : ids)
                if (node->id == id)
                    node->marked = true;

            node->update();
        }
    }
}

void GraphWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!pointIntersectsItem(mapToScene(mapFromGlobal(event->globalPos()))))
    {        
        QMenu menu(this);

        // Add operation

        QMenu *operationsMenu = menu.addMenu("Add operation");

        for (QString opName : generator->availableOperations)
        {
            QAction* action = operationsMenu->addAction(opName);
            action->setData(QVariant(QCursor::pos()));
        }

        connect(operationsMenu, &QMenu::triggered, this, &GraphWidget::addOperationNodeUnderCursor);

        // Add seed

        QAction* addSeedAction = menu.addAction("Add seed", this, &GraphWidget::addSeedNodeUnderCursor);
        addSeedAction->setData(QVariant(QCursor::pos()));

        // Paste node buffer

        if (!copiedNodes[1].empty())
        {
            menu.addSeparator();

            QAction* pasteAction = menu.addAction("Paste", this, &GraphWidget::pasteCopiedNodes);
            pasteAction->setData(QVariant(QCursor::pos()));
        }

        menu.exec(event->globalPos());
    }
    else
        QGraphicsView::contextMenuEvent(event);
}

bool GraphWidget::pointIntersectsItem(QPointF point)
{
    QVector<Node*> nodes;
    QVector<Edge*> edges;

    const QList<QGraphicsItem*> items = scene()->items(point);

    for (QGraphicsItem* item : items)
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
            nodes << node;
        else if (SeedNode* node = qgraphicsitem_cast<SeedNode*>(item))
            nodes << node;
        else if (Edge* edge = qgraphicsitem_cast<Edge*>(item))
            edges << edge;
    }

    return !nodes.empty() || !edges.empty();
}

void GraphWidget::wheelEvent(QWheelEvent *event)
{
    qreal factor = pow(2.0, -event->angleDelta().y() / 480.0);
    scaleFactor = transform().scale(factor, factor).mapRect(QRectF(0, 0, 1, 1)).width();

    scale(factor, factor);

    center = mapToScene(rect().center());
    centerOn(center);
}

void GraphWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(1, Qt::lightGray);
    painter->fillRect(rect, gradient);

    QRectF bound = scene()->itemsBoundingRect();
    QRectF intersection = rect.intersected(bound);

    if (intersection != bound)
    {
        qreal arrowWidth = 40.0;
        qreal arrowHeight = 10.0;
        qreal margin = 5.0;

        // Avoid scale transformation

        QTransform t = painter->transform();
        qreal m11 = t.m11();
        qreal m22 = t.m22();
        painter->save();
        painter->setTransform(QTransform(1, t.m12(), t.m13(), t.m21(), 1, t.m23(), t.m31(), t.m32(), t.m33()));

        painter->setPen(QPen(Qt::darkGray, 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        if (intersection.top() == rect.top() || bound.bottom() < rect.top())
        {
            QPainterPath path(QPointF(m11 * rect.center().x() - 0.5 * arrowWidth, m22 * rect.top() + arrowHeight + margin));
            path.lineTo(QPointF(m11 * rect.center().x(), m22 * rect.top() + margin));
            path.lineTo(QPointF(m11 * rect.center().x() + 0.5 * arrowWidth, m22 * rect.top() + arrowHeight + margin));
            painter->drawPath(path);
        }
        if (intersection.bottom() == rect.bottom() || bound.top() > rect.bottom())
        {
            QPainterPath path(QPointF(m11 * rect.center().x() - 0.5 * arrowWidth, m22 * rect.bottom() - arrowHeight - margin));
            path.lineTo(QPointF(m11 * rect.center().x(), m22 * rect.bottom() - margin));
            path.lineTo(QPointF(m11 * rect.center().x() + 0.5 * arrowWidth, m22 * rect.bottom() - arrowHeight - margin));
            painter->drawPath(path);
        }
        if (intersection.left() == rect.left() || bound.right() < rect.left())
        {
            QPainterPath path(QPointF(m11 * rect.left() + arrowHeight + margin, m22 * rect.center().y() - 0.5 * arrowWidth));
            path.lineTo(QPointF(m11 * rect.left() + margin, m22 * rect.center().y()));
            path.lineTo(QPointF(m11 * rect.left() + arrowHeight + margin, m22 * rect.center().y() + 0.5 * arrowWidth));
            painter->drawPath(path);
        }
        if (intersection.right() == rect.right() || bound.left() > rect.right())
        {
            QPainterPath path(QPointF(m11 * rect.right() - arrowHeight - margin, m22 * rect.center().y() - 0.5 * arrowWidth));
            path.lineTo(QPointF(m11 * rect.right() - margin, m22 * rect.center().y()));
            path.lineTo(QPointF(m11 * rect.right() - arrowHeight - margin, m22 * rect.center().y() + 0.5 * arrowWidth));
            painter->drawPath(path);
        }

        painter->restore();
    }
}

void GraphWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->modifiers() == Qt::ControlModifier && event->buttons() == Qt::LeftButton)
    {
        if (!pointIntersectsItem(mapToScene(mapFromGlobal(event->globalPos()))))
        {
            QPointF delta = event->localPos() - prevPos;
            center -= delta / scaleFactor;
            centerOn(center);

            // Draw background

            resetCachedContent();

            prevPos = event->localPos();
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}

void GraphWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->modifiers() == Qt::ControlModifier && event->buttons() == Qt::LeftButton)
    {
        if (!pointIntersectsItem(mapToScene(mapFromGlobal(event->globalPos()))))
            prevPos = event->localPos();
    }

    if (!pointIntersectsItem(mapToScene(mapFromGlobal(event->globalPos()))))
        deselectNodeToConnect();

    QGraphicsView::mousePressEvent(event);
}

void GraphWidget::searchElementaryCycles()
{
    QVector<Node*> nodes;
    QVector<Edge*> edges;

    const QList<QGraphicsItem *> items = scene()->items();

    for (QGraphicsItem* item : items)
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
            nodes << node;
        else if (SeedNode* node = qgraphicsitem_cast<SeedNode*>(item))
            nodes << node;
        else if (Edge* edge = qgraphicsitem_cast<Edge*>(item))
        {
            edges << edge;
            edge->clearCycles();
        }
        else if (Cycle* cycle = qgraphicsitem_cast<Cycle*>(item))
        {
            scene()->removeItem(cycle);
            delete cycle;
        }
    }

    QVector<bool> row(nodes.size(), false);
    QVector<QVector<bool>> adjacencyMatrix(nodes.size(), row);

    for (Edge* edge : edges)
    {
        int i = nodes.indexOf(edge->sourceNode());
        int j = nodes.indexOf(edge->destNode());
        adjacencyMatrix[i][j] = true;
    }

    ElementaryCyclesSearch ecs(adjacencyMatrix, nodes);
    QVector<QVector<Node*>> cycles = ecs.getElementaryCycles();

    for (QVector<Node*> nodeCycle : cycles)
    {
        Cycle* cycle = new Cycle(this, nodeCycle);
        scene()->addItem(cycle);
    }
}
