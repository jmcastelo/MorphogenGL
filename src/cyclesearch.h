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

#include "node.h"
#include <QVector>

struct SCCResult
{
    QVector<QVector<int>> adjacencyList;
    int lowestNodeId;

    SCCResult(QVector<QVector<int>> adjList, int id) :
        adjacencyList { adjList },
        lowestNodeId { id }
    {}
};

class ElementaryCyclesSearch
{
public:
    ElementaryCyclesSearch(QVector<QVector<bool>> matrix, QVector<Node*> nodes);
    QVector<QVector<Node*>> getElementaryCycles();

private:
    QVector<QVector<Node*>> cycles;
    QVector<QVector<int>> adjacencyList;
    QVector<Node*> graphNodes;
    QVector<bool> blocked;
    QVector<QVector<int>> B;
    QVector<int> stack;

    QVector<QVector<int>> getAdjacencyList(QVector<QVector<bool>> matrix);
    bool findCycles(int v, int s, QVector<QVector<int>> adjList);
    void unblock(int node);
};

class StrongConnectedComponents
{
public:
    StrongConnectedComponents(QVector<QVector<int>> adjList);
    SCCResult* getAdjacencyList(int node);

private:
    QVector<QVector<int>> adjacencyListOriginal;
    QVector<QVector<int>> adjacencyList;
    QVector<bool> visited;
    QVector<int> stack;
    QVector<int> lowlink;
    QVector<int> number;
    int sccCounter = 0;
    QVector<QVector<int>> currentSCCs;

    void makeAdjListSubgraph(int node);
    QVector<int> getLowestIdComponent();
    QVector<QVector<int>> getAdjList(QVector<int> nodes);
    void getStrongConnectedComponents(int root);
};
