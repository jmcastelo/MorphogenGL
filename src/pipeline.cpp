/*
*  Copyright 2020 Jose Maria Castelo Ares
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
#include "parameter.h"

Pipeline::Pipeline(GLuint id, float blend, QOpenGLContext* mainContext) : textureID { id }, blendFactor { blend }, sharedContext { mainContext }
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
        HueShift::name,
        MorphologicalGradient::name,
        PolarConvolution::name,
        Rotation::name,
        Saturation::name,
        Scale::name,
        Value::name
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
        operation = new Brightness(false, ":/shaders/screen.vert", ":/shaders/brightness.frag", sharedContext, 0.0f);
    }
    else if (operationName == ColorMix::name)
    {
        std::vector<float> rgbMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
        operation = new ColorMix(false, ":/shaders/screen.vert", ":/shaders/colormix.frag", sharedContext, rgbMatrix);
    }
    else if (operationName == Contrast::name)
    {
        operation = new Contrast(false, ":/shaders/screen.vert", ":/shaders/contrast.frag", sharedContext, 1.0f);
    }
    else if (operationName == Convolution::name)
    {
        std::vector<float> kernel = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        operation = new Convolution(false, ":/shaders/screen.vert", ":/shaders/convolution.frag", sharedContext, kernel, 0.01f);
    }
    else if (operationName == Dilation::name)
    {
        operation = new Dilation(false, ":/shaders/screen.vert", ":/shaders/dilation.frag", sharedContext, 0.01f);
    }
    else if (operationName == Erosion::name)
    {
        operation = new Erosion(false, ":/shaders/screen.vert", ":/shaders/erosion.frag", sharedContext, 0.01f);
    }
    else if (operationName == GammaCorrection::name)
    {
        operation = new GammaCorrection(false, ":/shaders/screen.vert", ":/shaders/gamma.frag", sharedContext, 1.0f, 1.0f, 1.0f);
    }
    else if (operationName == HueShift::name)
    {
        operation = new HueShift(false, ":/shaders/screen.vert", ":/shaders/hueshift.frag", sharedContext, 0.0f);
    }
    else if (operationName == MorphologicalGradient::name)
    {
        operation = new MorphologicalGradient(false, ":/shaders/screen.vert", ":/shaders/morphogradient.frag", sharedContext, 0.01f, 0.01f);
    }
    else if (operationName == PolarConvolution::name)
    {
        std::vector<PolarKernel*> polarKernels = { new PolarKernel(8, 0.01f, 0.0f, 1.0f, 0.0f, -1.0f, 1.0f) };
        operation = new PolarConvolution(false, ":/shaders/screen.vert", ":/shaders/polar-convolution.frag", sharedContext, polarKernels, 1.0f);
    }
    else if (operationName == Rotation::name)
    {
        operation = new Rotation(false, ":/shaders/transform.vert", ":/shaders/screen.frag", sharedContext, 0.0f, GL_NEAREST);
    }
    else if (operationName == Saturation::name)
    {
        operation = new Saturation(false, ":/shaders/screen.vert", ":/shaders/saturation.frag", sharedContext, 0.5f);
    }
    else if (operationName == Scale::name)
    {
        operation = new Scale(false, ":/shaders/transform.vert", ":/shaders/screen.frag", sharedContext, 1.0f, GL_NEAREST);
    }
    else if (operationName == Value::name)
    {
        operation = new Value(false, ":/shaders/screen.vert", ":/shaders/value.frag", sharedContext, 0.5f);
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
    std::vector<float> matrixElements,
    std::vector<PolarKernel*> polarKernels)
{
    ImageOperation* operation = nullptr;

    if (operationName == Brightness::name)
    {
        operation = new Brightness(enabled, ":/shaders/screen.vert", ":/shaders/brightness.frag", sharedContext, floatParameters[0]);
    }
    else if (operationName == ColorMix::name)
    {
        operation = new ColorMix(enabled, ":/shaders/screen.vert", ":/shaders/colormix.frag", sharedContext, matrixElements);
    }
    else if (operationName == Contrast::name)
    {
        operation = new Contrast(enabled, ":/shaders/screen.vert", ":/shaders/contrast.frag", sharedContext, floatParameters[0]);
    }
    else if (operationName == Convolution::name)
    {
        operation = new Convolution(enabled, ":/shaders/screen.vert", ":/shaders/convolution.frag", sharedContext, kernelElements, floatParameters[0]);
    }
    else if (operationName == Dilation::name)
    {
        operation = new Dilation(enabled, ":/shaders/screen.vert", ":/shaders/dilation.frag", sharedContext, floatParameters[0]);
    }
    else if (operationName == Erosion::name)
    {
        operation = new Erosion(enabled, ":/shaders/screen.vert", ":/shaders/erosion.frag", sharedContext, floatParameters[0]);
    }
    else if (operationName == GammaCorrection::name)
    {
        operation = new GammaCorrection(enabled, ":/shaders/screen.vert", ":/shaders/gamma.frag", sharedContext, floatParameters[0], floatParameters[1], floatParameters[2]);
    }
    else if (operationName == HueShift::name)
    {
        operation = new HueShift(enabled, ":/shaders/screen.vert", ":/shaders/hueshift.frag", sharedContext, floatParameters[0]);
    }
    else if (operationName == MorphologicalGradient::name)
    {
        operation = new MorphologicalGradient(enabled, ":/shaders/screen.vert", ":/shaders/morphogradient.frag", sharedContext, floatParameters[0], floatParameters[1]);
    }
    else if (operationName == PolarConvolution::name)
    {
        operation = new PolarConvolution(enabled, ":/shaders/screen.vert", ":/shaders/polar-convolution.frag", sharedContext, polarKernels, floatParameters[0]);
    }
    else if (operationName == Rotation::name)
    {
        operation = new Rotation(enabled, ":/shaders/transform.vert", ":/shaders/screen.frag", sharedContext, floatParameters[0], interpolationParameters[0]);
    }
    else if (operationName == Saturation::name)
    {
        operation = new Saturation(enabled, ":/shaders/screen.vert", ":/shaders/saturation.frag", sharedContext, floatParameters[0]);
    }
    else if (operationName == Scale::name)
    {
        operation = new Scale(enabled, ":/shaders/transform.vert", ":/shaders/screen.frag", sharedContext, floatParameters[0], interpolationParameters[0]);
    }
    else if (operationName == Value::name)
    {
        operation = new Value(enabled, ":/shaders/screen.vert", ":/shaders/value.frag", sharedContext, floatParameters[0]);
    }

    if (operation) imageOperations.push_back(operation);
}

void Pipeline::resize()
{
    for (auto& operation : imageOperations)
        operation->resize();
}
