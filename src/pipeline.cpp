/*
*  Copyright 2020 José María Castelo Ares
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

#include "pipeline.h"

Pipeline::Pipeline(GLuint id, float blend) : textureID{ id }, blendFactor{ blend }
{
    // Duplicate of GeneratorGL's

    availableImageOperations = {
        Brightness::name,
        ColorMix::name,
        Contrast::name,
        Convolution::name,
        Dilation::name,
        Erosion::name,
        GammaCorrection::name,
        MorphologicalGradient::name,
        Rotation::name,
        Scale::name
    };
}

Pipeline::~Pipeline()
{
    for (auto operation : imageOperations)
        delete operation;
}

void Pipeline::iterate(GLuint id)
{
    // Start with given input image

    textureID = id;

    // Operations iteration

    for (auto& operation : imageOperations)
    {
        if (operation->isEnabled())
        {
            operation->applyOperation(textureID);
            textureID = operation->getTextureID();
        }
    }       
}

GLuint Pipeline::getTextureID()
{
    for (auto& operation : imageOperations)
        if (operation->isEnabled())
            textureID = operation->getTextureID();
     
    return textureID;
}

void Pipeline::swapImageOperations(int operationIndex0, int operationIndex1)
{
    ImageOperation* operation = imageOperations[operationIndex0];

    imageOperations.erase(imageOperations.begin() + operationIndex0);
    imageOperations.insert(imageOperations.begin() + operationIndex1, operation);
}

void Pipeline::removeImageOperation(int operationIndex)
{
    if (!imageOperations.empty())
    {
        std::vector<ImageOperation*>::iterator it = imageOperations.begin();
        imageOperations.erase(it + operationIndex);
    }
}

void Pipeline::insertImageOperation(int newOperationIndex, int currentOperationIndex)
{
    ImageOperation* operation = nullptr;

    QString operationName = availableImageOperations[newOperationIndex];

    if (operationName == Brightness::name)
    {
        operation = new Brightness(false, ":/shaders/screen.vert", ":/shaders/brightness.frag", 0.0f);
    }
    else if (operationName == ColorMix::name)
    {
        std::vector<float> rgbMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
        operation = new ColorMix(false, ":/shaders/screen.vert", ":/shaders/colormix.frag", rgbMatrix);
    }
    else if (operationName == Contrast::name)
    {
        operation = new Contrast(false, ":/shaders/screen.vert", ":/shaders/contrast.frag", 1.0f);
    }
    else if (operationName == Convolution::name)
    {
        std::vector<float> kernel = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        operation = new Convolution(false, ":/shaders/screen.vert", ":/shaders/convolution.frag", kernel, 0.01f);
    }
    else if (operationName == Dilation::name)
    {
        operation = new Dilation(false, ":/shaders/screen.vert", ":/shaders/dilation.frag", 0.01f);
    }
    else if (operationName == Erosion::name)
    {
        operation = new Erosion(false, ":/shaders/screen.vert", ":/shaders/erosion.frag", 0.01f);
    }
    else if (operationName == GammaCorrection::name)
    {
        operation = new GammaCorrection(false, ":/shaders/screen.vert", ":/shaders/gamma.frag", 1.0f, 1.0f, 1.0f);
    }
    else if (operationName == MorphologicalGradient::name)
    {
        operation = new MorphologicalGradient(false, ":/shaders/screen.vert", ":/shaders/morphogradient.frag", 0.01f, 0.01f);
    }
    else if (operationName == Rotation::name)
    {
        operation = new Rotation(false, ":/shaders/transform.vert", ":/shaders/screen.frag", 0.0f, GL_NEAREST);
    }
    else if (operationName == Scale::name)
    {
        operation = new Scale(false, ":/shaders/transform.vert", ":/shaders/screen.frag", 1.0f, GL_NEAREST);
    }

    if (operation)
    {
        std::vector<ImageOperation*>::iterator it = imageOperations.begin();
        imageOperations.insert(it + currentOperationIndex, operation);
    }
}

void Pipeline::loadImageOperation(
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

    if (operationName == Brightness::name)
    {
        operation = new Brightness(enabled, ":/shaders/screen.vert", ":/shaders/brightness.frag", floatParameters[0]);
    }
    else if (operationName == ColorMix::name)
    {
        operation = new ColorMix(enabled, ":/shaders/screen.vert", ":/shaders/colormix.frag", matrixElements);
    }
    else if (operationName == Contrast::name)
    {
        operation = new Contrast(enabled, ":/shaders/screen.vert", ":/shaders/contrast.frag", floatParameters[0]);
    }
    else if (operationName == Convolution::name)
    {
        operation = new Convolution(enabled, ":/shaders/screen.vert", ":/shaders/convolution.frag", kernelElements, floatParameters[0]);
    }
    else if (operationName == Dilation::name)
    {
        operation = new Dilation(enabled, ":/shaders/screen.vert", ":/shaders/dilation.frag", floatParameters[0]);
    }
    else if (operationName == Erosion::name)
    {
        operation = new Erosion(enabled, ":/shaders/screen.vert", ":/shaders/erosion.frag", floatParameters[0]);
    }
    else if (operationName == GammaCorrection::name)
    {
        operation = new GammaCorrection(enabled, ":/shaders/screen.vert", ":/shaders/gamma.frag", floatParameters[0], floatParameters[1], floatParameters[2]);
    }
    else if (operationName == MorphologicalGradient::name)
    {
        operation = new MorphologicalGradient(enabled, ":/shaders/screen.vert", ":/shaders/morphogradient.frag", floatParameters[0], floatParameters[1]);
    }
    else if (operationName == Rotation::name)
    {
        operation = new Rotation(enabled, ":/shaders/transform.vert", ":/shaders/screen.frag", floatParameters[0], interpolationParameters[0]);
    }
    else if (operationName == Scale::name)
    {
        operation = new Scale(enabled, ":/shaders/transform.vert", ":/shaders/screen.frag", floatParameters[0], interpolationParameters[0]);
    }

    if (operation) imageOperations.push_back(operation);
}

void Pipeline::resize()
{
    for (auto& operation : imageOperations)
        operation->resize();
}