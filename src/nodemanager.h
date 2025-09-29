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



#ifndef NODEMANAGER_H
#define NODEMANAGER_H



#include "imageoperationnode.h"
#include "factory.h"
#include "imageoperation.h"
#include "seed.h"
#include "inputdata.h"
#include "operationwidget.h"
#include "seedwidget.h"
#include "edgewidget.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QOpenGLContext>
#include <QUuid>
#include <QMap>



class NodeManager : public QObject
{
    Q_OBJECT

public:
    // NodeManager(RenderManager* renderManager);
    NodeManager(Factory* factory);
    ~NodeManager();

    QList<QString> availableOperations;

    void init(QOpenGLContext* shareContext);

    bool isActive() { return active; }
    void setState(bool state) { active = state; }

    bool connectOperations(QUuid srcId, QUuid dstId, float factor);
    void connectCopiedOperationsA(QUuid srcId0, QUuid dstId0, QUuid srcId1, QUuid dstId1);
    void connectCopiedOperationsB(QUuid srcId0, QUuid dstId0, QUuid srcId1, QUuid dstId1);
    void disconnectOperations(QUuid srcId, QUuid dstId);

    void setOperationInputType(QUuid srcId, QUuid dstId, InputType type);

    EdgeWidget* addEdgeWidget(QUuid srcId, QUuid dstId, float factor);

    float blendFactor(QUuid srcId, QUuid dstId);
    void setBlendFactor(QUuid srcId, QUuid dstId, float factor);
    void equalizeBlendFactors(QUuid id);

    ImageOperation* getOperation(QUuid id);
    // QPair<QUuid, OperationWidget*> addNewOperation();
    //QUuid addOperation(QString operationName);
    QUuid copyOperation(QUuid srcId);
    //void setOperation(QUuid id, QString operationName);
    void removeOperation(QUuid id);
    // void enableOperation(QUuid id, bool enabled);
    // bool isOperationEnabled(QUuid id);

    void clearLoadedOperations() { loadedOperationNodes.clear(); }
    void loadOperation(QUuid id, ImageOperation* operation);
    void connectLoadedOperations(QMap<QUuid, QMap<QUuid, InputData*>> conections);
    ImageOperation* loadImageOperation(
        QString operationName,
        bool enabled,
        std::vector<bool> boolParameters,
        std::vector<int> intParameters,
        std::vector<float> floatParameters,
        std::vector<int> interpolationParameters,
        std::vector<float> kernelElements,
        std::vector<float> matrixElements);

    // QPair<QUuid, SeedWidget*> addSeed();
    QUuid copySeed(QUuid srcId);
    void removeSeed(QUuid id);
    void loadSeedImage(QUuid id, QString filename);
    int getSeedType(QUuid id);
    void setSeedType(QUuid id, int set);
    void resetInputSeedTexId(QUuid id);
    bool isSeedFixed(QUuid id);
    void setSeedFixed(QUuid id, bool fixed);
    void drawSeed(QUuid id);
    void drawAllSeeds();

    void clearLoadedSeeds() { loadedSeeds.clear(); }
    void loadSeed(QUuid id, int type, bool fixed);

    bool isOutput(QUuid id) { return id == mOutputId; }
    void setOutput(QUuid id);
    QUuid getOutput() { return mOutputId; }
    GLuint getOUtputFBO() { return outputFBO; }
    // GLuint outputTextureID() { return *mOutputTextureId; }

    void pasteOperations();

    void swapLoadedOperations() { operationNodes = loadedOperationNodes; }
    void swapLoadedSeeds() { seeds = loadedSeeds; }

    void swapTwoOperations(QUuid id1, QUuid id2);

    void sortOperations();

    void clearOperation(QUuid id);
    void clearAllOperations();

    QMap<QUuid, ImageOperationNode*> getOperationNodes() { return operationNodes; }
    QMap<QUuid, Seed*> getSeeds() { return seeds; }
    QList<QUuid> getOperationNodesIDs() { return operationNodes.keys(); }

    bool isNode(QUuid id) { return operationNodes.contains(id) || seeds.contains(id); }

    QString version = "1.0 alpha";

signals:
    void outputNodeChanged(QWidget* widget);
    void outputFBOChanged(GLuint fbo);
    void outputTextureChanged(GLuint* pTexId);
    // void sortedOperationsChanged(QList<QPair<QUuid, QString>> sortedData, QList<QUuid> unsortedData);
    void sortedOperationsChanged(QList<ImageOperation*> operations);
    void nodesConnected(QUuid srcId, QUuid dstId, EdgeWidget* widget);
    void nodeRemoved(QUuid id);
    void nodesDisconnected(QUuid srcId, QUuid dstId);

public slots:
    void addOperationNode(QUuid id, ImageOperation* operation);
    void connectOperationWidget(QUuid id, OperationWidget* widget);
    void addSeedNode(QUuid id, Seed* seed);
    void connectSeedWidget(QUuid id, SeedWidget* widget);
    void onTexturesChanged();

private:
    // RenderManager* mRenderManager;
    Factory* mFactory;

    QMap<QUuid, ImageOperationNode*> operationNodes;
    QMap<QUuid, ImageOperationNode*> copiedOperationNodes[2];
    QMap<QUuid, ImageOperationNode*> loadedOperationNodes;

    QMap<QUuid, Seed*> seeds;
    QMap<QUuid, Seed*> copiedSeeds[2];
    QMap<QUuid, Seed*> loadedSeeds;

    QUuid connSrcId;

    QOpenGLContext* mShareContext;
    GLuint outputFBO;
    QUuid mOutputId;
    GLuint* mOutputTextureId = nullptr;
    unsigned int iteration = 0;

    bool active = false;
};



#endif // NODEMANAGER_H
