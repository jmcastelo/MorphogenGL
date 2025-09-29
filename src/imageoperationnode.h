#ifndef IMAGEOPERATIONNODE_H
#define IMAGEOPERATIONNODE_H



#include "imageoperation.h"
#include "inputdata.h"

#include <QUuid>
#include <QMap>
#include <QList>



class ImageOperationNode
{
public:
    QUuid id;

    ImageOperation* operation;

    QMap<QUuid, ImageOperationNode*> inputNodes;
    QMap<QUuid, ImageOperationNode*> outputNodes;

    QMap<QUuid, InputData*> inputs;

    bool computed = false;

    ImageOperationNode(QUuid uuid) : id { uuid } { outputNodes.clear(); };
    ~ImageOperationNode();

    void addSeedInput(QUuid id, InputData* data);
    void removeSeedInput(QUuid id);

    void addInput(ImageOperationNode* node, InputData* data);
    void removeInput(ImageOperationNode* node);
    void setInputType(QUuid id, InputType type);

    void setInputSeedTexId(QUuid id, GLuint *texId);

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

    QList<InputData *> inputsList();
};

#endif // IMAGEOPERATIONNODE_H
