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

#include <QGraphicsView>
#include <QUuid>
#include <QMap>

class Node;
class OperationNode;
class SeedNode;
class Edge;
class GeneratorGL;
struct InputData;

class GraphWidget : public QGraphicsView
{
    Q_OBJECT

public:
    GeneratorGL* generator;

    GraphWidget(GeneratorGL* generatorGL, QWidget *parent = nullptr);

    void copyNode(Node* node);
    void removeNode(Node* node);
    void removeEdge(Edge* edge);
    void selectNodeToConnect(Node* node);
    void deselectNodeToConnect();
    void connectNodes(Node* node);

    bool nodeSelectedToConnect();
    bool moreThanOneNode();

    void newNodeSelected(Node*);
    bool nodesSelected();
    bool operationNodesSelected();

    void clearSelectedOperationNodes();

    void makeNodeSnapshot();
    void removeSelectedNodes();

    void setOperationParameters(QUuid id);
    void updateOperation(QUuid id);

    void onDestroyOperationNode(QUuid id);
    void onDestroySeedNode(QUuid id);

    void updateNodes();

    void drawBlendFactors(bool draw);

    QPointF nodePosition(QUuid id);

    void clearScene();
    void loadSeedNode(QUuid id, QPointF position);
    void loadOperationNode(QUuid id, QString name, QPointF position);
    void connectNodes(QMap<QUuid, QMap<QUuid, InputData*>> connections);

    void markNodes(QVector<QUuid> ids);

signals:
    void singleNodeSelected(Node*);
    void multipleNodesSelected(bool operationNodesSelected);
    void showOperationParameters(QUuid id);
    void removeOperationParameters(QUuid id);
    void updateOperationParameters(QUuid id);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    Node *selectedNode = nullptr;

    QMap<QUuid, Node*> copiedNodes[2];
    QVector<Edge*> newEdges;

    QPointF prevPos;
    QPointF center;
    qreal scaleFactor = 1.0;

    Node* getNode(QUuid id);
    void copyNodes(bool connectionA);
    bool pointIntersectsItem(QPointF point);
    void searchElementaryCycles();

private slots:
    void newSelectedNodes();
    void addOperationNodeUnderCursor(QAction* action);
    void addSeedNodeUnderCursor();
    void pasteCopiedNodes();
};
