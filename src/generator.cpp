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

#include "generator.h"

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
    operation->setInputData(inputsVector());
}

void ImageOperationNode::removeSeedInput(QUuid id)
{
    if (inputs.contains(id))
    {
        delete inputs.value(id);
        inputs.remove(id);

        operation->setInputData(inputsVector());
    }
}

void ImageOperationNode::addInput(ImageOperationNode *node, InputData* data)
{
    inputNodes.insert(node->id, node);
    inputs.insert(node->id, data);

    operation->setInputData(inputsVector());
}

void ImageOperationNode::removeInput(ImageOperationNode *node)
{
    inputNodes.remove(node->id);

    delete inputs.value(node->id);
    inputs.remove(node->id);

    operation->setInputData(inputsVector());
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

QVector<InputData*> ImageOperationNode::inputsVector()
{
    QVector<InputData*> inputData;

    foreach(InputData* inData, inputs)
        inputData.push_back(inData);

    return inputData;
}

void ImageOperationNode::setOperation(ImageOperation *newOperation)
{
    delete operation;

    operation = newOperation;

    // Set operation's input data

    operation->setInputData(inputsVector());

    // Set new texture id on output nodes

    foreach (ImageOperationNode* node, outputNodes)
    {
        if (node->inputs.value(id)->type == InputType::Normal)
        {
            node->inputs[id]->textureID = operation->getTextureID();
            operation->enableBlit(false);
        }
        else if (node->inputs.value(id)->type == InputType::Blit)
        {
            node->inputs[id]->textureID = operation->getTextureBlit();
            operation->enableBlit(false);
        }
    }
}

// GeneratorGL

GeneratorGL::GeneratorGL()
{
    availableOperations = {
        BilateralFilter::name,
        Brightness::name,
        ColorMix::name,
        ColorQuantization::name,
        Contrast::name,
        Convolution::name,
        Dilation::name,
        Erosion::name,
        GammaCorrection::name,
        HueShift::name,
        Identity::name,
        Logistic::name,
        Mask::name,
        Median::name,
        Memory::name,
        MorphologicalGradient::name,
        Pixelation::name,
        PolarConvolution::name,
        Power::name,
        Rotation::name,
        Saturation::name,
        Scale::name,
        Value::name
    };
}

void GeneratorGL::init(QOpenGLContext* mainContext)
{
    sharedContext = mainContext;
}

GeneratorGL::~GeneratorGL()
{
    foreach(Seed* seed, seeds)
        delete seed;

    foreach(ImageOperationNode* node, operationNodes)
        delete node;
}

void GeneratorGL::sortOperations()
{
    sortedOperations.clear();

    QVector<QPair<QUuid, QString>> sortedOperationsData;

    QMap<QUuid, ImageOperationNode*> pendingNodes = operationNodes;

    // Set operations as non-computed (pending)

    foreach (ImageOperationNode* node, operationNodes)
        node->setComputed(false);

    // First operations are those whose inputs are all blit or seed, they do not depend on pending operations
    // Also operations with no inputs but with some outputs, sorting algorithm depends on operations having inputs
    // Discard isolated nodes, with no inputs nor outputs

    foreach (ImageOperationNode* node, operationNodes)
    {
        if (node->numInputs() == 0 && node->numOutputs() == 0)
        {
            pendingNodes.remove(node->id);
        }
        else if ((node->numInputs() == 0 && node->numOutputs() > 0) ||
                 (node->numInputs() > 0 && node->numNonNormalInputs() == node->numInputs()))
        {
            sortedOperations.push_back(node->operation);
            sortedOperationsData.push_back(QPair<QUuid, QString>(node->id, node->operation->getName()));
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
                sortedOperationsData.push_back(QPair<QUuid, QString>(node->id, node->operation->getName()));
                computedNodes.push_back(node);
                node->setComputed(true);
            }
        }

        for (ImageOperationNode* node : computedNodes)
            pendingNodes.remove(node->id);

        if (computedNodes.empty())
            break;
    }

    emit sortedOperationsChanged(sortedOperationsData);
}

void GeneratorGL::iterate()
{
    if (active)
    {
        foreach(ImageOperation* operation, sortedOperations)
            operation->applyOperation();

        foreach(ImageOperation* operation, sortedOperations)
            operation->blit();

        iteration++;
    }

    foreach (Seed* seed, seeds)
        if (!seed->isFixed() && !seed->isCleared())
            seed->clear();
}

void GeneratorGL::clearOperation(QUuid id)
{
    if (operationNodes.contains(id))
        operationNodes.value(id)->operation->clear();
}

void GeneratorGL::clearAllOperations()
{
    foreach (ImageOperationNode* node, operationNodes)
        node->operation->clear();
}

void GeneratorGL::setOutput(QUuid id)
{
    if (operationNodes.contains(id))
    {
        outputID = id;
        //outputTextureID = operationNodes.value(id)->operation->getTextureBlit();
        outputTextureID = operationNodes.value(id)->operation->getTextureID();
        emit outputTextureChanged(**outputTextureID);
    }
    else if (seeds.contains(id))
    {
        outputID = id;
        outputTextureID = seeds.value(id)->getTextureID();
        emit outputTextureChanged(**outputTextureID);
    }
}

void GeneratorGL::connectOperations(QUuid srcId, QUuid dstId, float factor)
{
    if (operationNodes.contains(srcId) && operationNodes.contains(dstId))
    {
        operationNodes.value(dstId)->addInput(operationNodes.value(srcId), new InputData(InputType::Normal, operationNodes.value(srcId)->operation->getTextureID(), factor));
        operationNodes.value(srcId)->addOutput(operationNodes.value(dstId));
    }
    else if (seeds.contains(srcId) && operationNodes.contains(dstId))
    {
        operationNodes.value(dstId)->addSeedInput(srcId, new InputData(InputType::Seed, seeds.value(srcId)->getTextureID(), factor));
    }

    sortOperations();
}

void GeneratorGL::connectCopiedOperationsA(QUuid srcId0, QUuid dstId0, QUuid srcId1, QUuid dstId1)
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

void GeneratorGL::connectCopiedOperationsB(QUuid srcId0, QUuid dstId0, QUuid srcId1, QUuid dstId1)
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

void GeneratorGL::disconnectOperations(QUuid srcId, QUuid dstId)
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

void GeneratorGL::setOperationInputType(QUuid srcId, QUuid dstId, InputType type)
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

void GeneratorGL::pasteOperations()
{
    operationNodes.insert(copiedOperationNodes[0]);
    seeds.insert(copiedSeeds[0]);

    copiedOperationNodes[1] = copiedOperationNodes[0];
    copiedSeeds[1] = copiedSeeds[0];

    copiedOperationNodes[0].clear();
    copiedSeeds[0].clear();

    sortOperations();
}

void GeneratorGL::swapTwoOperations(QUuid id1, QUuid id2)
{
    ImageOperation* operation1 = operationNodes.value(id1)->operation->clone();
    operation1->setParameters();

    ImageOperation* operation2 = operationNodes.value(id2)->operation->clone();
    operation2->setParameters();

    operationNodes.value(id1)->setOperation(operation2);
    operationNodes.value(id2)->setOperation(operation1);

    sortOperations();

    if (isOutput(id1))
        setOutput(id1);
    else if (isOutput(id2))
        setOutput(id2);
}

float GeneratorGL::blendFactor(QUuid srcId, QUuid dstId)
{
    return operationNodes.value(dstId)->blendFactor(srcId);
}

void GeneratorGL::setBlendFactor(QUuid srcId, QUuid dstId, float factor)
{
    operationNodes.value(dstId)->setBlendFactor(srcId, factor);
}

void GeneratorGL::equalizeBlendFactors(QUuid id)
{
    operationNodes.value(id)->equalizeBlendFactors();
}

ImageOperation* GeneratorGL::getOperation(QUuid id)
{
    return operationNodes.contains(id) ? operationNodes.value(id)->operation : nullptr;
}

QUuid GeneratorGL::addOperation(QString operationName)
{
    ImageOperation* operation = newOperation(operationName);

    QUuid id = QUuid::createUuid();

    ImageOperationNode *node = new ImageOperationNode(id);
    node->operation = operation;
    operationNodes.insert(id, node);

    return id;
}

QUuid GeneratorGL::copyOperation(QUuid srcId)
{
    ImageOperation* operation = operationNodes.value(srcId)->operation->clone();
    operation->setParameters();

    QUuid id = QUuid::createUuid();

    ImageOperationNode *node = new ImageOperationNode(id);
    node->operation = operation;
    copiedOperationNodes[0].insert(id, node);

    return id;
}

void GeneratorGL::setOperation(QUuid id, QString operationName)
{
    ImageOperation* operation = newOperation(operationName);

    operationNodes.value(id)->setOperation(operation);

    // If it's output node, set new output texture id

    if (id == outputID)
        outputTextureID = operation->getTextureID();

    sortOperations();
}

void GeneratorGL::removeOperation(QUuid id)
{
    delete operationNodes.value(id);
    operationNodes.remove(id);

    if (id == outputID)
        outputTextureID = nullptr;

    sortOperations();
}

void GeneratorGL::enableOperation(QUuid id, bool enabled)
{
    operationNodes.value(id)->operation->enable(enabled);
}

bool GeneratorGL::isOperationEnabled(QUuid id)
{
    return operationNodes.value(id)->operation->isEnabled();
}

bool GeneratorGL::hasOperationParamaters(QUuid id)
{
    return operationNodes.value(id)->operation->hasParameters();
}

void GeneratorGL::loadOperation(QUuid id, ImageOperation* operation)
{
    ImageOperationNode *node = new ImageOperationNode(id);
    node->operation = operation;

    loadedOperationNodes.insert(id, node);
}

void GeneratorGL::connectLoadedOperations(QMap<QUuid, QMap<QUuid, InputData*>> connections)
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

ImageOperation* GeneratorGL::newOperation(QString operationName)
{
    ImageOperation* operation = nullptr;

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
    else if (operationName == Erosion::name)
    {
        operation = new Erosion(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == GammaCorrection::name)
    {
        operation = new GammaCorrection(false, sharedContext, 1.0f, 1.0f, 1.0f, 1.0f);
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
        operation = new Mask(false, sharedContext);
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
    else if (operationName == PolarConvolution::name)
    {
        std::vector<PolarKernel*> polarKernels = { new PolarKernel(8, 0.01f, 0.0f, 1.0f, 0.0f, -1.0f, 1.0f) };
        operation = new PolarConvolution(false, sharedContext, polarKernels, 1.0f, 1.0f);
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

    return operation;
}

ImageOperation* GeneratorGL::loadImageOperation(
    QString operationName,
    bool enabled,
    std::vector<bool> boolParameters,
    std::vector<int> intParameters,
    std::vector<float> floatParameters,
    std::vector<int> interpolationParameters,
    std::vector<float> kernelElements,
    std::vector<float> matrixElements,
    std::vector<PolarKernel*> polarKernels)
{
    ImageOperation* operation = nullptr;

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
    else if (operationName == Erosion::name)
    {
        operation = new Erosion(enabled, sharedContext, floatParameters[0], floatParameters[1]);
    }
    else if (operationName == GammaCorrection::name)
    {
        operation = new GammaCorrection(enabled, sharedContext, floatParameters[0], floatParameters[1], floatParameters[2], floatParameters[3]);
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
        operation = new Mask(enabled, sharedContext);
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
    else if (operationName == PolarConvolution::name)
    {
        operation = new PolarConvolution(enabled, sharedContext, polarKernels, floatParameters[0], floatParameters[1]);
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

    return operation;
}

QUuid GeneratorGL::addSeed()
{
    Seed* seed = new Seed(sharedContext);

    QUuid id = QUuid::createUuid();
    seeds.insert(id, seed);

    return id;
}

QUuid GeneratorGL::copySeed(QUuid srcId)
{
    Seed* seed = new Seed(*seeds.value(srcId));

    QUuid id = QUuid::createUuid();
    copiedSeeds[0].insert(id, seed);

    return id;
}

void GeneratorGL::removeSeed(QUuid id)
{
    delete seeds.value(id);
    seeds.remove(id);

    foreach (ImageOperationNode* node, operationNodes)
        node->removeSeedInput(id);
}

void GeneratorGL::loadSeed(QUuid id, int type, bool fixed)
{
    Seed* seed = new Seed(sharedContext);

    seed->setType(type);
    seed->setFixed(fixed);

    loadedSeeds.insert(id, seed);
}

void GeneratorGL::loadSeedImage(QUuid id, QString filename)
{
    seeds.value(id)->loadImage(filename);
}

int GeneratorGL::getSeedType(QUuid id)
{
    return seeds.value(id)->getType();
}

void GeneratorGL::setSeedType(QUuid id, int set)
{
    seeds.value(id)->setType(set);
}

bool GeneratorGL::isSeedFixed(QUuid id)
{
    return seeds.value(id)->isFixed();
}

void GeneratorGL::setSeedFixed(QUuid id, bool fixed)
{
    seeds.value(id)->setFixed(fixed);
}

void GeneratorGL::drawSeed(QUuid id)
{
    seeds.value(id)->draw();
}

void GeneratorGL::drawAllSeeds()
{
    foreach (Seed* seed, seeds)
        if (!seed->isFixed())
            seed->draw();
}

QImage GeneratorGL::outputImage()
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

void GeneratorGL::setTextureFormat(TextureFormat format)
{
    FBO::texFormat = format;

    foreach (Seed* seed, seeds)
        seed->setTextureFormat();

    foreach (ImageOperationNode* node, operationNodes)
        node->operation->setTextureFormat();

    setOutput(outputID);
}

void GeneratorGL::resize(GLuint width, GLuint height)
{
    FBO::width = width;
    FBO::height = height;

    foreach (Seed* seed, seeds)
        seed->resize();

    foreach (ImageOperationNode* node, operationNodes)
        node->operation->resize();

    setOutput(outputID);

    emit imageSizeChanged(width, height);
}
