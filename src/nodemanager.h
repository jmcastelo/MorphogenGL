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



#include "imageoperation.h"
#include "parameter.h"
#include "seed.h"
#include "blender.h"
#include "texformat.h"
#include "operationwidget.h"

#include "fbo.h"
#include <QObject>
#include <QVector>
#include <QString>
#include <QImage>
#include <QOpenGLContext>
#include <QUuid>
#include <QMap>



struct ImageOperationNode
{
    QUuid id;

    ImageOperation* operation;

    QMap<QUuid, ImageOperationNode*> inputNodes;
    QMap<QUuid, ImageOperationNode*> outputNodes;

    QMap<QUuid, InputData*> inputs;

    bool computed = false;

    ImageOperationNode(QUuid uuid) : id { uuid } {};
    ~ImageOperationNode();

    void addSeedInput(QUuid id, InputData* data);
    void removeSeedInput(QUuid id);

    void addInput(ImageOperationNode* node, InputData* data);
    void removeInput(ImageOperationNode* node);
    void setInputType(QUuid id, InputType type);

    int numInputs();
    int numNonNormalInputs();
    int numOutputs();

    bool isBlitConnected();

    void addOutput(ImageOperationNode* node);
    void removeOutput(ImageOperationNode* node);

    void setComputed(bool done);
    bool allInputsComputed();

    float blendFactor(QUuid id);
    void setBlendFactor(QUuid id, float factor);
    void equalizeBlendFactors();

    void setOperation(ImageOperation* newOperation);

    QVector<InputData*> inputsVector();
};



class NodeManager : public QObject
{
    Q_OBJECT

public:
    NodeManager();
    ~NodeManager();

    QList<QString> availableOperations;

    void init(QOpenGLContext* mainContext);

    bool isActive() { return active; }
    void setState(bool state) { active = state; }

    void connectOperations(QUuid srcId, QUuid dstId, float factor);
    void connectCopiedOperationsA(QUuid srcId0, QUuid dstId0, QUuid srcId1, QUuid dstId1);
    void connectCopiedOperationsB(QUuid srcId0, QUuid dstId0, QUuid srcId1, QUuid dstId1);
    void disconnectOperations(QUuid srcId, QUuid dstId);

    void setOperationInputType(QUuid srcId, QUuid dstId, InputType type);

    float blendFactor(QUuid srcId, QUuid dstId);
    void setBlendFactor(QUuid srcId, QUuid dstId, float factor);
    void equalizeBlendFactors(QUuid id);

    ImageOperation* getOperation(QUuid id);
    OperationWidget* addNewOperation();
    //QUuid addOperation(QString operationName);
    QUuid copyOperation(QUuid srcId);
    //void setOperation(QUuid id, QString operationName);
    void removeOperation(QUuid id);
    void enableOperation(QUuid id, bool enabled);
    bool isOperationEnabled(QUuid id);
    bool hasOperationParamaters(QUuid id);

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

    QUuid addSeed();
    QUuid copySeed(QUuid srcId);
    void removeSeed(QUuid id);
    void loadSeedImage(QUuid id, QString filename);
    int getSeedType(QUuid id);
    void setSeedType(QUuid id, int set);
    bool isSeedFixed(QUuid id);
    void setSeedFixed(QUuid id, bool fixed);
    void drawSeed(QUuid id);
    void drawAllSeeds();

    void clearLoadedSeeds() { loadedSeeds.clear(); }
    void loadSeed(QUuid id, int type, bool fixed);

    bool isOutput(QUuid id) { return id == outputID; }
    void setOutput(QUuid id);
    QUuid getOutput() { return outputID; }
    GLuint getOUtputFBO() { return outputFBO; }
    GLuint** getOutputTextureID() { return outputTextureID; }

    QImage outputImage();

    void setTextureFormat(TextureFormat format);

    void pasteOperations();

    void swapLoadedOperations() { operationNodes = loadedOperationNodes; }
    void swapLoadedSeeds() { seeds = loadedSeeds; }

    void swapTwoOperations(QUuid id1, QUuid id2);

    void sortOperations();
    void iterate();

    void clearOperation(QUuid id);
    void clearAllOperations();

    void resetIterationNumer() { iteration = 0; }
    int getIterationNumber() { return iteration; }

    int getWidth() { return FBO::width; }
    int getHeight() { return FBO::height; }

    QMap<QUuid, ImageOperationNode*> getOperationNodes() { return operationNodes; }
    QMap<QUuid, Seed*> getSeeds() { return seeds; }
    QList<QUuid> getOperationNodesIDs() { return operationNodes.keys(); }

    bool isNode(QUuid id) { return operationNodes.contains(id) || seeds.contains(id); }

    QString version = "1.0 beta";

public slots:
    void resize(GLuint width, GLuint height);

signals:
    void outputFBOChanged(GLuint fbo);
    void outputTextureChanged(GLuint id);
    void sortedOperationsChanged(QList<QPair<QUuid, QString>> sortedData, QList<QUuid> unsortedData);

private:
    QMap<QUuid, ImageOperationNode*> operationNodes;
    QMap<QUuid, ImageOperationNode*> copiedOperationNodes[2];
    QMap<QUuid, ImageOperationNode*> loadedOperationNodes;

    QMap<QUuid, Seed*> seeds;
    QMap<QUuid, Seed*> copiedSeeds[2];
    QMap<QUuid, Seed*> loadedSeeds;

    QList<ImageOperation*> sortedOperations;

    QOpenGLContext* sharedContext;
    GLuint outputFBO;
    QUuid outputID;
    GLuint** outputTextureID = nullptr;
    unsigned int iteration = 0;

    bool active = false;
};



#endif // NODEMANAGER_H
