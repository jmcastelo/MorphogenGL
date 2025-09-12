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



#include "nodemanager.h"



// Image operation node

ImageOperationNode::~ImageOperationNode()
{
    foreach (ImageOperationNode* input, inputNodes)
        input->removeOutput(this);

    foreach (ImageOperationNode* output, outputNodes)
        output->removeInput(this);

    delete operation;

    foreach (InputData* data, inputs)
        delete data;
}



void ImageOperationNode::addSeedInput(QUuid id, InputData* data)
{
    inputs.insert(id, data);
    operation->setInputData(inputsList());
}



void ImageOperationNode::removeSeedInput(QUuid id)
{
    if (inputs.contains(id))
    {
        delete inputs.value(id);
        inputs.remove(id);

        operation->setInputData(inputsList());
    }
}



void ImageOperationNode::addInput(ImageOperationNode *node, InputData* data)
{
    inputNodes.insert(node->id, node);
    inputs.insert(node->id, data);

    operation->setInputData(inputsList());
}



void ImageOperationNode::removeInput(ImageOperationNode *node)
{
    inputNodes.remove(node->id);

    delete inputs.value(node->id);
    inputs.remove(node->id);

    operation->setInputData(inputsList());
}



int ImageOperationNode::numInputs()
{
    return inputs.size();
}



int ImageOperationNode::numNonNormalInputs()
{
    int count = 0;

    foreach (InputData* data, inputs)
        if (data->type != InputType::Normal)
            count++;

    return count;
}



int ImageOperationNode::numOutputs()
{
    return outputNodes.size();
}



bool ImageOperationNode::isBlitConnected()
{
    foreach (ImageOperationNode* node, outputNodes)
        foreach (InputData* input, node->inputs)
            if (input->type == InputType::Blit)
                return true;
    return false;
}



void ImageOperationNode::addOutput(ImageOperationNode *node)
{
    outputNodes.insert(node->id, node);
}



void ImageOperationNode::removeOutput(ImageOperationNode *node)
{
    outputNodes.remove(node->id);
}



void ImageOperationNode::setInputType(QUuid id, InputType type)
{
    inputs[id]->type = type;

    if (type == InputType::Normal)
    {
        inputs[id]->textureID = inputNodes.value(id)->operation->getTextureID();
        inputNodes.value(id)->operation->enableBlit(false);
    }
    else if (type == InputType::Blit)
    {
        inputs[id]->textureID = inputNodes.value(id)->operation->getTextureBlit();
        inputNodes.value(id)->operation->enableBlit(true);
    }
}



void ImageOperationNode::setInputSeedTexId(QUuid id, GLuint** texId)
{
    if (inputs.contains(id))
    {
        inputs[id]->textureID = texId;
        operation->setInputData(inputsList());
    }
}



bool ImageOperationNode::allInputsComputed()
{
    foreach (ImageOperationNode* node, inputNodes)
        if(!node->computed && inputs.value(node->id)->type == InputType::Normal)
            return false;

    return true;
}



void ImageOperationNode::setComputed(bool done)
{
    computed = done;
}



float ImageOperationNode::blendFactor(QUuid id)
{
    return inputs.value(id)->blendFactor;
}



void ImageOperationNode::setBlendFactor(QUuid id, float factor)
{
    inputs[id]->blendFactor = factor;
}



void ImageOperationNode::equalizeBlendFactors()
{
    int numInputs = inputs.size();

    foreach (InputData* data, inputs)
        data->blendFactor = 1.0 / numInputs;
}



QList<InputData*> ImageOperationNode::inputsList()
{
    QList<InputData*> inputData;

    foreach(InputData* inData, inputs)
        inputData.append(inData);

    return inputData;
}



void ImageOperationNode::setOperation(ImageOperation *newOperation)
{
    delete operation;

    operation = newOperation;

    // Set operation's input data

    operation->setInputData(inputsList());

    // Set new texture id on output nodes

    foreach (ImageOperationNode* node, outputNodes)
    {
        if (node->inputs.value(id)->type == InputType::Normal)
        {
            node->inputs[id]->textureID = operation->getTextureID();
            node->operation->setInputData(node->inputsList());
            operation->enableBlit(false);
        }
        else if (node->inputs.value(id)->type == InputType::Blit)
        {
            node->inputs[id]->textureID = operation->getTextureBlit();
            node->operation->setInputData(node->inputsList());
            operation->enableBlit(true);
        }
    }
}



// NodeManager

NodeManager::NodeManager()
{
    /*availableOperations = {
        BilateralFilter::name,
        Brightness::name,
        ColorMix::name,
        ColorQuantization::name,
        Contrast::name,
        Convolution::name,
        Dilation::name,
        EqualizeHistogram::name,
        Erosion::name,
        GammaCorrection::name,
        Geometry::name,
        HueShift::name,
        Identity::name,
        Logistic::name,
        Mask::name,
        Median::name,
        Memory::name,
        MorphologicalGradient::name,
        Pixelation::name,
        Power::name,
        Rotation::name,
        Saturation::name,
        Scale::name,
        Value::name
    };*/
}



void NodeManager::init(QOpenGLContext* mainContext)
{
    sharedContext = mainContext;
}



NodeManager::~NodeManager()
{
    foreach(Seed* seed, seeds)
        delete seed;

    foreach(ImageOperationNode* node, operationNodes)
        delete node;
}



void NodeManager::sortOperations()
{
    //QList<ImageOperation*> tmpSortedOperations = sortedOperations;

    sortedOperations.clear();

    QList<QPair<QUuid, QString>> sortedOperationsData;
    QList<QUuid> unsortedOperationsIds;

    QMap<QUuid, ImageOperationNode*> pendingNodes = operationNodes;

    // Set operations as non-computed (pending)

    foreach (ImageOperationNode* node, operationNodes)
    {
        node->setComputed(false);
    }

    // First operations are those whose inputs are all blit or seed, they do not depend on pending operations
    // Also operations with no inputs but with some outputs, sorting algorithm depends on operations having inputs
    // Discard isolated nodes, with no inputs nor outputs

    foreach (ImageOperationNode* node, operationNodes)
    {
        if (node->numInputs() == 0 && node->numOutputs() == 0)
        {
            pendingNodes.remove(node->id);
            unsortedOperationsIds.push_back(node->id);
        }
        else if ((node->numInputs() == 0 && node->numOutputs() > 0) ||
                 (node->numInputs() > 0 && node->numNonNormalInputs() == node->numInputs()))
        {
            sortedOperations.push_back(node->operation);
            sortedOperationsData.push_back(QPair<QUuid, QString>(node->id, node->operation->name()));
            pendingNodes.remove(node->id);
            node->setComputed(true);
        }
    }

    // Sorting algorithm

    while (!pendingNodes.empty())
    {
        QVector<ImageOperationNode*> computedNodes;

        foreach (ImageOperationNode* node, pendingNodes)
        {
            if (node->allInputsComputed())
            {
                sortedOperations.push_back(node->operation);
                sortedOperationsData.push_back(QPair<QUuid, QString>(node->id, node->operation->name()));
                computedNodes.push_back(node);
                node->setComputed(true);
            }
        }

        foreach (ImageOperationNode* node, computedNodes)
        {
            pendingNodes.remove(node->id);
        }

        if (computedNodes.empty())
            break;
    }

    //if (tmpSortedOperations != sortedOperations)
        emit sortedOperationsChanged(sortedOperationsData, unsortedOperationsIds);
}



void NodeManager::iterate()
{
    foreach (ImageOperation* operation, sortedOperations)
    {
        operation->blit();
    }

    foreach (ImageOperation* operation, sortedOperations)
    {
        operation->applyOperation();
    }

    iteration++;

    foreach (Seed* seed, seeds)
    {
        if (!seed->isFixed() && !seed->isCleared())
            seed->clear();
    }
}



void NodeManager::clearOperation(QUuid id)
{
    if (operationNodes.contains(id))
        operationNodes.value(id)->operation->clear();
}



void NodeManager::clearAllOperations()
{
    foreach (ImageOperationNode* node, operationNodes)
    {
        node->operation->clear();
    }
}



void NodeManager::setOutput(QUuid id)
{
    if (operationNodes.contains(id))
    {
        // Avoid disabling blit for output node if it is blit connected (predge)

        if (!outputID.isNull() && operationNodes.contains(outputID) && !operationNodes.value(outputID)->isBlitConnected())
            operationNodes.value(outputID)->operation->enableBlit(false);

        outputID = id;

        // We enable blit for output node so that QOpenGLWidget renders the previous texture
        // This kind of double buffering allows for smoother, flickerless rendering

        operationNodes.value(id)->operation->enableBlit(true);
        outputTextureID = operationNodes.value(id)->operation->getTextureBlit();

        emit outputTextureChanged(**outputTextureID);
        emit outputFBOChanged(operationNodes.value(id)->operation->getFBO());
    }
    else if (seeds.contains(id))
    {
        outputID = id;
        outputTextureID = seeds.value(id)->getTextureID();

        emit outputTextureChanged(**outputTextureID);
        emit outputFBOChanged(seeds.value(id)->getFBO());
    }
}



bool NodeManager::connectOperations(QUuid srcId, QUuid dstId, float factor)
{
    if (srcId != dstId)
    {
        if (operationNodes.contains(srcId) && operationNodes.contains(dstId) && !operationNodes.value(dstId)->inputNodes.contains(srcId))
        {
            operationNodes.value(dstId)->addInput(operationNodes.value(srcId), new InputData(InputType::Normal, operationNodes.value(srcId)->operation->getTextureID(), factor));
            operationNodes.value(srcId)->addOutput(operationNodes.value(dstId));

            sortOperations();
            return true;
        }
        else if (seeds.contains(srcId) && operationNodes.contains(dstId) && !operationNodes.value(dstId)->inputNodes.contains(srcId))
        {
            operationNodes.value(dstId)->addSeedInput(srcId, new InputData(InputType::Seed, seeds.value(srcId)->getTextureID(), factor));

            sortOperations();
            return true;
        }
    }

    return false;
}



void NodeManager::connectCopiedOperationsA(QUuid srcId0, QUuid dstId0, QUuid srcId1, QUuid dstId1)
{
    if (operationNodes.contains(srcId0))
    {
        float factor = operationNodes.value(dstId0)->inputs.value(srcId0)->blendFactor;

        copiedOperationNodes[0].value(dstId1)->addInput(copiedOperationNodes[0].value(srcId1), new InputData(InputType::Normal, copiedOperationNodes[0].value(srcId1)->operation->getTextureID(), factor));
        copiedOperationNodes[0].value(srcId1)->addOutput(copiedOperationNodes[0].value(dstId1));
    }
    else if (seeds.contains(srcId0))
    {
        float factor = operationNodes.value(dstId0)->inputs.value(srcId0)->blendFactor;
        copiedOperationNodes[0].value(dstId1)->addSeedInput(srcId1, new InputData(InputType::Seed, seeds.value(srcId0)->getTextureID(), factor));
    }
}



void NodeManager::connectCopiedOperationsB(QUuid srcId0, QUuid dstId0, QUuid srcId1, QUuid dstId1)
{
    if (copiedOperationNodes[1].contains(srcId0))
    {
        float factor = copiedOperationNodes[1].value(dstId0)->inputs.value(srcId0)->blendFactor;

        copiedOperationNodes[0].value(dstId1)->addInput(copiedOperationNodes[0].value(srcId1), new InputData(InputType::Normal, copiedOperationNodes[0].value(srcId1)->operation->getTextureID(), factor));
        copiedOperationNodes[0].value(srcId1)->addOutput(copiedOperationNodes[0].value(dstId1));
    }
    else if (copiedSeeds[1].contains(srcId0))
    {
        float factor = copiedOperationNodes[1].value(dstId0)->inputs.value(srcId0)->blendFactor;
        copiedOperationNodes[0].value(dstId1)->addSeedInput(srcId1, new InputData(InputType::Seed, copiedSeeds[0].value(srcId1)->getTextureID(), factor));
    }
}



void NodeManager::disconnectOperations(QUuid srcId, QUuid dstId)
{
    if (operationNodes.contains(srcId))
    {
        operationNodes.value(dstId)->removeInput(operationNodes.value(srcId));
        operationNodes.value(srcId)->removeOutput(operationNodes.value(dstId));
    }
    else if (seeds.contains(srcId))
    {
        operationNodes.value(dstId)->removeSeedInput(srcId);
    }

    sortOperations();
}



void NodeManager::setOperationInputType(QUuid srcId, QUuid dstId, InputType type)
{
    if (operationNodes.contains(srcId))
    {
        operationNodes.value(dstId)->setInputType(srcId, type);
        sortOperations();
    }
    else if (copiedOperationNodes[0].contains(srcId))
    {
        copiedOperationNodes[0].value(dstId)->setInputType(srcId, type);
    }
}



void NodeManager::pasteOperations()
{
    operationNodes.insert(copiedOperationNodes[0]);
    seeds.insert(copiedSeeds[0]);

    copiedOperationNodes[1] = copiedOperationNodes[0];
    copiedSeeds[1] = copiedSeeds[0];

    copiedOperationNodes[0].clear();
    copiedSeeds[0].clear();

    sortOperations();
}



void NodeManager::swapTwoOperations(QUuid id1, QUuid id2)
{
    //ImageOperation* operation1 = operationNodes.value(id1)->operation->clone();
    //operation1->setParameters();

    //ImageOperation* operation2 = operationNodes.value(id2)->operation->clone();
    //operation2->setParameters();

    ImageOperation* operation1 = new ImageOperation(*operationNodes.value(id1)->operation);
    ImageOperation* operation2 = new ImageOperation(*operationNodes.value(id2)->operation);

    operationNodes.value(id1)->setOperation(operation2);
    operationNodes.value(id2)->setOperation(operation1);

    sortOperations();

    if (isOutput(id1))
        setOutput(id1);
    else if (isOutput(id2))
        setOutput(id2);
}



float NodeManager::blendFactor(QUuid srcId, QUuid dstId)
{
    return operationNodes.value(dstId)->blendFactor(srcId);
}



void NodeManager::setBlendFactor(QUuid srcId, QUuid dstId, float factor)
{
    operationNodes.value(dstId)->setBlendFactor(srcId, factor);
}



void NodeManager::equalizeBlendFactors(QUuid id)
{
    operationNodes.value(id)->equalizeBlendFactors();
}



ImageOperation* NodeManager::getOperation(QUuid id)
{
    return operationNodes.contains(id) ? operationNodes.value(id)->operation : nullptr;
}



/*QUuid NodeManager::addOperation(QString operationName)
{
    ImageOperation* operation = newOperation(operationName);

    QUuid id = QUuid::createUuid();

    ImageOperationNode *node = new ImageOperationNode(id);
    node->operation = operation;
    operationNodes.insert(id, node);

    return id;
}*/



QUuid NodeManager::copyOperation(QUuid srcId)
{
    //ImageOperation* operation = operationNodes.value(srcId)->operation->clone();
    //operation->setParameters();

    ImageOperation* operation = new ImageOperation(*operationNodes.value(srcId)->operation);

    QUuid id = QUuid::createUuid();

    ImageOperationNode *node = new ImageOperationNode(id);
    node->operation = operation;
    copiedOperationNodes[0].insert(id, node);

    return id;
}



/*void NodeManager::setOperation(QUuid id, QString operationName)
{
    ImageOperation* operation = newOperation(operationName);

    operationNodes.value(id)->setOperation(operation);

    // If it's output node, set new output texture id

    if (isOutput(id))
        setOutput(id);

    sortOperations();
}*/



void NodeManager::removeOperation(QUuid id)
{
    delete operationNodes.value(id);
    operationNodes.remove(id);

    if (id == outputID)
        outputTextureID = nullptr;

    sortOperations();
}



void NodeManager::enableOperation(QUuid id, bool enabled)
{
    operationNodes.value(id)->operation->enable(enabled);
}



bool NodeManager::isOperationEnabled(QUuid id)
{
    return operationNodes.value(id)->operation->enabled();
}



void NodeManager::loadOperation(QUuid id, ImageOperation* operation)
{
    ImageOperationNode *node = new ImageOperationNode(id);
    node->operation = operation;

    loadedOperationNodes.insert(id, node);
}



void NodeManager::connectLoadedOperations(QMap<QUuid, QMap<QUuid, InputData*>> connections)
{
    QMap<QUuid, QMap<QUuid, InputData*>>::iterator dst = connections.begin();

    while (dst != connections.end())
    {
        QMap<QUuid, InputData*>::iterator src = dst.value().begin();

        while (src != dst.value().constEnd())
        {
           if (loadedOperationNodes.contains(src.key()))
           {
               InputData* inputData = src.value();

               if (inputData->type == InputType::Normal)
               {
                   inputData->textureID = loadedOperationNodes.value(src.key())->operation->getTextureID();
                   loadedOperationNodes.value(src.key())->operation->enableBlit(false);
               }
               else if (inputData->type == InputType::Blit)
               {
                   inputData->textureID = loadedOperationNodes.value(src.key())->operation->getTextureBlit();
                   loadedOperationNodes.value(src.key())->operation->enableBlit(true);
               }

               loadedOperationNodes.value(dst.key())->addInput(loadedOperationNodes.value(src.key()), inputData);
               loadedOperationNodes.value(src.key())->addOutput(loadedOperationNodes.value(dst.key()));
           }
           else if (loadedSeeds.contains(src.key()))
           {
               InputData* inputData = src.value();
               inputData->textureID = loadedSeeds.value(src.key())->getTextureID();

               loadedOperationNodes.value(dst.key())->addSeedInput(src.key(), inputData);
           }

            src++;
        }

        dst++;
    }
}



EdgeWidget* NodeManager::addEdgeWidget(QUuid srcId, QUuid dstId, float factor)
{
    EdgeWidget* edgeWidget = new EdgeWidget(factor, operationNodes.contains(srcId));

    connect(edgeWidget, &EdgeWidget::blendFactorChanged, this, [=, this](float newFactor){
        setBlendFactor(srcId, dstId, newFactor);
    });

    connect(edgeWidget, &EdgeWidget::edgeTypeChanged, this, [=, this](bool predge){
        if (predge)
            setOperationInputType(srcId, dstId, InputType::Blit);
        else
            setOperationInputType(srcId, dstId, InputType::Normal);
    });

    connect(edgeWidget, &EdgeWidget::remove, this, [=, this](){
        disconnectOperations(srcId, dstId);
        emit nodesDisconnected(srcId, dstId);
    });

    return edgeWidget;
}




QPair<QUuid, OperationWidget*> NodeManager::addNewOperation()
{
    ImageOperation* operation = new ImageOperation("New Operation", sharedContext);

    QUuid id = QUuid::createUuid();

    ImageOperationNode *node = new ImageOperationNode(id);
    node->operation = operation;
    operationNodes.insert(id, node);

    OperationWidget* opWidget = new OperationWidget(operation, false, true);

    connect(opWidget, &OperationWidget::outputChanged, this, [=, this](bool checked){
        if (checked)
        {
            setOutput(id);
            emit outputNodeChanged(opWidget);
        }
        else
            emit outputNodeChanged(nullptr);
    });

    connect(this, &NodeManager::outputNodeChanged, opWidget, &OperationWidget::toggleOutputAction);

    connect(opWidget, &OperationWidget::connectTo, this, [=, this](){
        if (connSrcId.isNull())
            connSrcId = id;
        else if (connectOperations(connSrcId, id, 1.0))
        {
            emit nodesConnected(connSrcId, id, addEdgeWidget(connSrcId, id, 1.0));
            connSrcId = QUuid();
        }
        else
            connSrcId = QUuid();
    });

    connect(opWidget, &OperationWidget::remove, this, [=, this](){
        emit nodeRemoved(id);
        removeOperation(id);
    });

    return QPair<QUuid, OperationWidget*>(id, opWidget);

/*
    if (operationName == BilateralFilter::name)
    {
        operation = new BilateralFilter(false, sharedContext, 3, 1.0f, 10.0f, 0.1f, 1.0f);
    }
    else if (operationName == Brightness::name)
    {
        operation = new Brightness(false, sharedContext, 0.0f, 1.0f);
    }
    else if (operationName == ColorMix::name)
    {
        std::vector<float> rgbMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
        operation = new ColorMix(false, sharedContext, rgbMatrix, 1.0f);
    }
    else if (operationName == ColorQuantization::name)
    {
        operation = new ColorQuantization(false, sharedContext, 10, 10, 10, 1.0f);
    }
    else if (operationName == Contrast::name)
    {
        operation = new Contrast(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == Convolution::name)
    {
        std::vector<float> kernel = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        operation = new Convolution(false, sharedContext, kernel, 1.0f, 1.0f, 1.0f);
    }
    else if (operationName == Dilation::name)
    {
        operation = new Dilation(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == EqualizeHistogram::name)
    {
        operation = new EqualizeHistogram(false, sharedContext, 1, 64, 1.0f);
    }
    else if (operationName == Erosion::name)
    {
        operation = new Erosion(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == GammaCorrection::name)
    {
        operation = new GammaCorrection(false, sharedContext, 1.0f, 1.0f, 1.0f, 1.0f);
    }
    else if (operationName == Geometry::name)
    {
        operation = new Geometry(false, sharedContext, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, GL_NEAREST);
    }
    else if (operationName == HueShift::name)
    {
        operation = new HueShift(false, sharedContext, 0.0f, 1.0f);
    }
    else if (operationName == Identity::name)
    {
        operation = new Identity(false, sharedContext);
    }
    else if (operationName == Logistic::name)
    {
        operation = new Logistic(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == Mask::name)
    {
        operation = new Mask(false, sharedContext, 0.0f, 0.5f);
    }
    else if (operationName == Median::name)
    {
        operation = new Median(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == Memory::name)
    {
        operation = new Memory(false, sharedContext, 1, 0.1f, 0.5f);
    }
    else if (operationName == MorphologicalGradient::name)
    {
        operation = new MorphologicalGradient(false, sharedContext, 1.0f, 1.0f, 1.0f);
    }
    else if (operationName == Pixelation::name)
    {
        operation = new Pixelation(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == Power::name)
    {
        operation = new Power(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == Rotation::name)
    {
        operation = new Rotation(false, sharedContext, 0.0f, GL_NEAREST);
    }
    else if (operationName == Saturation::name)
    {
        operation = new Saturation(false, sharedContext, 0.5f, 1.0f);
    }
    else if (operationName == Scale::name)
    {
        operation = new Scale(false, sharedContext, 1.0f, GL_NEAREST);
    }
    else if (operationName == Value::name)
    {
        operation = new Value(false, sharedContext, 0.5f, 1.0f);
    }

    if (operation)
        operation->setParameters();

    return operation;*/
}



ImageOperation* NodeManager::loadImageOperation(
    QString operationName,
    bool enabled,
    std::vector<bool> boolParameters,
    std::vector<int> intParameters,
    std::vector<float> floatParameters,
    std::vector<int> interpolationParameters,
    std::vector<float> kernelElements,
    std::vector<float> matrixElements)
{
    ImageOperation* operation = nullptr;
/*
    if (operationName == BilateralFilter::name)
    {
        operation = new BilateralFilter(enabled, sharedContext, intParameters[0], floatParameters[0], floatParameters[1], floatParameters[2], floatParameters[3]);
    }
    else if (operationName == Brightness::name)
    {
        operation = new Brightness(enabled, sharedContext, floatParameters[0], floatParameters[1]);
    }
    else if (operationName == ColorMix::name)
    {
        operation = new ColorMix(enabled, sharedContext, matrixElements, floatParameters[0]);
    }
    else if (operationName == ColorQuantization::name)
    {
        operation = new ColorQuantization(enabled, sharedContext, intParameters[0], intParameters[1], intParameters[2], floatParameters[0]);
    }
    else if (operationName == Contrast::name)
    {
        operation = new Contrast(enabled, sharedContext, floatParameters[0], floatParameters[1]);
    }
    else if (operationName == Convolution::name)
    {
        operation = new Convolution(enabled, sharedContext, kernelElements, floatParameters[0], floatParameters[1], floatParameters[2]);
    }
    else if (operationName == Dilation::name)
    {
        operation = new Dilation(enabled, sharedContext, floatParameters[0], floatParameters[1]);
    }
    else if (operationName == EqualizeHistogram::name)
    {
        operation = new EqualizeHistogram(enabled, sharedContext, intParameters[0], intParameters[1], floatParameters[0]);
    }
    else if (operationName == Erosion::name)
    {
        operation = new Erosion(enabled, sharedContext, floatParameters[0], floatParameters[1]);
    }
    else if (operationName == GammaCorrection::name)
    {
        operation = new GammaCorrection(enabled, sharedContext, floatParameters[0], floatParameters[1], floatParameters[2], floatParameters[3]);
    }
    else if (operationName == Geometry::name)
    {
        operation = new Geometry(enabled, sharedContext, floatParameters[0], floatParameters[1], floatParameters[2], floatParameters[3], floatParameters[4], interpolationParameters[0]);
    }
    else if (operationName == HueShift::name)
    {
        operation = new HueShift(enabled, sharedContext, floatParameters[0], floatParameters[1]);
    }
    else if (operationName == Identity::name)
    {
        operation = new Identity(enabled, sharedContext);
    }
    else if (operationName == Logistic::name)
    {
        operation = new Logistic(enabled, sharedContext, floatParameters[0], floatParameters[1]);
    }
    else if (operationName == Mask::name)
    {
        operation = new Mask(enabled, sharedContext, floatParameters[0], floatParameters[1]);
    }
    else if (operationName == Median::name)
    {
        operation = new Median(enabled, sharedContext, floatParameters[0], floatParameters[1]);
    }
    else if (operationName == Memory::name)
    {
        operation = new Memory(enabled, sharedContext, intParameters[0], floatParameters[0], floatParameters[1]);
    }
    else if (operationName == MorphologicalGradient::name)
    {
        operation = new MorphologicalGradient(enabled, sharedContext, floatParameters[0], floatParameters[1], floatParameters[2]);
    }
    else if (operationName == Pixelation::name)
    {
        operation = new Pixelation(enabled, sharedContext, floatParameters[0], floatParameters[1]);
    }
    else if (operationName == Power::name)
    {
        operation = new Power(enabled, sharedContext, floatParameters[0], floatParameters[1]);
    }
    else if (operationName == Rotation::name)
    {
        operation = new Rotation(enabled, sharedContext, floatParameters[0], interpolationParameters[0]);
    }
    else if (operationName == Saturation::name)
    {
        operation = new Saturation(enabled, sharedContext, floatParameters[0], floatParameters[1]);
    }
    else if (operationName == Scale::name)
    {
        operation = new Scale(enabled, sharedContext, floatParameters[0], interpolationParameters[0]);
    }
    else if (operationName == Value::name)
    {
        operation = new Value(enabled, sharedContext, floatParameters[0], floatParameters[1]);
    }

    if (operation)
        operation->setParameters();
*/
    return operation;
}



QPair<QUuid, SeedWidget *> NodeManager::addSeed()
{
    Seed* seed = new Seed(sharedContext);

    QUuid id = QUuid::createUuid();
    seeds.insert(id, seed);

    SeedWidget* seedWidget = new SeedWidget(seed);

    connect(seedWidget, &SeedWidget::outputChanged, this, [=, this](bool checked){
        if (checked)
        {
            setOutput(id);
            emit outputNodeChanged(seedWidget);
        }
        else
            emit outputNodeChanged(nullptr);
    });

    connect(this, &NodeManager::outputNodeChanged, seedWidget, &SeedWidget::toggleOutputAction);

    connect(seedWidget, &SeedWidget::connectTo, this, [=, this](){
        if (connSrcId.isNull())
            connSrcId = id;
        else if (connectOperations(connSrcId, id, 1.0))
        {
            emit nodesConnected(connSrcId, id, addEdgeWidget(connSrcId, id, 1.0));
            connSrcId = QUuid();
        }
        else
            connSrcId = QUuid();
    });

    connect(seedWidget, &SeedWidget::typeChanged, this, [=, this](){
        if (isOutput(id))
        {
            outputTextureID = seeds.value(id)->getTextureID();

            emit outputTextureChanged(**outputTextureID);
            emit outputFBOChanged(seeds.value(id)->getFBO());
        }

        resetInputSeedTexId(id);
    });

    connect(seedWidget, &SeedWidget::remove, this, [=, this](){
        emit nodeRemoved(id);
        removeSeed(id);
    });

    return QPair<QUuid, SeedWidget*>(id, seedWidget);
}



QUuid NodeManager::copySeed(QUuid srcId)
{
    Seed* seed = new Seed(*seeds.value(srcId));

    QUuid id = QUuid::createUuid();
    copiedSeeds[0].insert(id, seed);

    return id;
}



void NodeManager::removeSeed(QUuid id)
{
    if (seeds.contains(id))
    {
        delete seeds.value(id);
        seeds.remove(id);

        foreach (ImageOperationNode* node, operationNodes)
            node->removeSeedInput(id);
    }
}



void NodeManager::loadSeed(QUuid id, int type, bool fixed)
{
    Seed* seed = new Seed(sharedContext);

    seed->setType(type);
    seed->setFixed(fixed);

    loadedSeeds.insert(id, seed);
}

void NodeManager::loadSeedImage(QUuid id, QString filename)
{
    seeds.value(id)->loadImage(filename);
}



int NodeManager::getSeedType(QUuid id)
{
    return seeds.value(id)->type();
}



void NodeManager::setSeedType(QUuid id, int set)
{
    seeds.value(id)->setType(set);
}



void NodeManager::resetInputSeedTexId(QUuid id)
{
    if (seeds.contains(id))
    {
        foreach (ImageOperationNode* opNode, operationNodes)
            opNode->setInputSeedTexId(id, seeds[id]->getTextureID());
    }
}



bool NodeManager::isSeedFixed(QUuid id)
{
    return seeds.value(id)->isFixed();
}



void NodeManager::setSeedFixed(QUuid id, bool fixed)
{
    seeds.value(id)->setFixed(fixed);
}



void NodeManager::drawSeed(QUuid id)
{
    seeds.value(id)->draw();
}



void NodeManager::drawAllSeeds()
{
    foreach (Seed* seed, seeds)
        if (!seed->isFixed())
            seed->draw();
}



QImage NodeManager::outputImage()
{
    ImageOperation* operation = getOperation(outputID);
    if (operation)
    {
        return operation->outputImage();
    }
    else
    {
        QImage image(FBO::width, FBO::height, QImage::Format_RGBA8888);
        image.fill(Qt::black);
        return image;
    }
}



void NodeManager::setTextureFormat(TextureFormat format)
{
    FBO::texFormat = format;

    foreach (Seed* seed, seeds)
        seed->setTextureFormat();

    foreach (ImageOperationNode* node, operationNodes)
        node->operation->setTextureFormat();

    setOutput(outputID);
}



void NodeManager::resize(GLuint width, GLuint height)
{
    FBO::width = width;
    FBO::height = height;

    foreach (Seed* seed, seeds)
        seed->resize();

    foreach (ImageOperationNode* node, operationNodes)
        node->operation->resize();

    setOutput(outputID);
}
