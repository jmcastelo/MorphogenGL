


#include "imageoperationnode.h"



ImageOperationNode::~ImageOperationNode()
{
    foreach (ImageOperationNode* input, inputNodes)
    input->removeOutput(this);

    foreach (ImageOperationNode* output, outputNodes)
        output->removeInput(this);

    // delete operation;

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
        if (data->type() != InputType::Normal)
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
        if (input->type() == InputType::Blit)
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
    if (inputs.contains(id))
    {
        inputs[id]->setType(type);

        if (type == InputType::Normal)
        {
            // inputs[id]->setpTextureId(inputNodes.value(id)->operation->pOutTextureId());
            inputNodes.value(id)->operation->enableBlit(false);
            inputs[id]->setpTextureId(inputNodes.value(id)->operation->pOutTextureId());
        }
        else if (type == InputType::Blit)
        {
            // inputs[id]->setpTextureId(inputNodes.value(id)->operation->blitTextureId());
            inputNodes.value(id)->operation->enableBlit(true);
            inputs[id]->setpTextureId(inputNodes.value(id)->operation->pOutTextureId());
        }
    }
}



void ImageOperationNode::setInputSeedTexId(QUuid id, GLuint* texId)
{
    if (inputs.contains(id))
    {
        inputs[id]->setpTextureId(texId);
        operation->setInputData(inputsList());
    }
}



bool ImageOperationNode::allInputsComputed()
{
    foreach (ImageOperationNode* node, inputNodes)
    if(!node->computed && inputs.value(node->id)->type() == InputType::Normal)
        return false;

    return true;
}



void ImageOperationNode::setComputed(bool done)
{
    computed = done;
}



float ImageOperationNode::blendFactor(QUuid id)
{
    return inputs.value(id)->blendFactor();
}



void ImageOperationNode::setBlendFactor(QUuid id, float factor)
{
    inputs[id]->setBlendFactor(factor);
}



void ImageOperationNode::equalizeBlendFactors()
{
    int numInputs = inputs.size();

    foreach (InputData* data, inputs)
        data->setBlendFactor(1.0 / numInputs);
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
        if (node->inputs.value(id)->type() == InputType::Normal)
        {
            operation->enableBlit(false);
            node->inputs[id]->setpTextureId(operation->pOutTextureId());
            node->operation->setInputData(node->inputsList());

        }
        else if (node->inputs.value(id)->type() == InputType::Blit)
        {
            operation->enableBlit(true);
            node->inputs[id]->setpTextureId(operation->pOutTextureId());
            node->operation->setInputData(node->inputsList());

        }
    }
}
