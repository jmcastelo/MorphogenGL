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

#include "generator.h"
#include "morphowidget.h"

GeneratorGL::GeneratorGL()
{
    // Duplicate of Pipeline's

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

    // Seed

    seed = new Seed(":/shaders/random-seed.vert", ":/shaders/random-seed.frag");

    // Blender

    blender = new Blender(":/shaders/screen.vert", ":/shaders/blend.frag");

    // Output pipeline

    outputPipeline = new Pipeline(outputTextureID, 1.0f);

    // Output FBO

    outputFBO[0] = new FBO(":/shaders/screen.vert", ":/shaders/screen.frag");
    outputFBO[1] = new FBO(":/shaders/screen.vert", ":/shaders/mask.frag");

    outputFBO[1]->program->bind();
    outputFBO[1]->program->setUniformValue("apply", applyMask);
    outputFBO[1]->program->release();
}

GeneratorGL::~GeneratorGL()
{
    for (auto pipeline : pipelines)
        delete pipeline;

    pipelines.clear();

    delete outputPipeline;

    delete seed;
    delete blender;

    delete outputFBO[0];
    delete outputFBO[1];

    if (encoder) delete encoder;
}

void GeneratorGL::addPipeline()
{
    // Set first pipeline's blend factor to unity, later pipeline's to zero
    
    if (pipelines.empty())
        pipelines.push_back(new Pipeline(outputTextureID, 1.0f));
    else
        pipelines.push_back(new Pipeline(outputTextureID, 0.0f));
}

void GeneratorGL::removePipeline(int pipelineIndex)
{
    if (!pipelines.empty() && pipelineIndex >= 0 && pipelineIndex < pipelines.size())
    {
        pipelines.erase(pipelines.begin() + pipelineIndex);

        // Recompute blend factors (their sum = 1)

        float sumBlendFactors = 0.0f;

        for (auto pipeline : pipelines)
            sumBlendFactors += pipeline->blendFactor;

        if (sumBlendFactors == 0.0f)
        {
            for (auto& pipeline : pipelines)
                pipeline->blendFactor = 1.0f / pipelines.size();
        }
        else
        {
            for (auto& pipeline : pipelines)
                pipeline->blendFactor /= sumBlendFactors;
        }
    }
}

void GeneratorGL::loadPipeline(float blendFactor)
{
    // Load pipeline with given blend factor

    pipelines.push_back(new Pipeline(outputTextureID, blendFactor));
}

void GeneratorGL::setPipelineBlendFactor(int pipelineIndex, float factor)
{
    // Keep within range

    if (factor < 0.0f)
        factor = 0.0f;
    if (factor > 1.0f)
        factor = 1.0f;

    if (pipelines.size() == 1)
        factor = 1.0f;

    float sumBlendFactors = 0.0f;

    for (int i = 0; i < static_cast<int>(pipelines.size()); i++)
        if (i != pipelineIndex)
            sumBlendFactors += pipelines[i]->blendFactor;

    if (sumBlendFactors == 0.0f)
    {
        for (int i = 0; i < static_cast<int>(pipelines.size()); i++)
            if (i != pipelineIndex)
                pipelines[i]->blendFactor = 1.0e-6;

        sumBlendFactors = 0.0;

        for (int i = 0; i < static_cast<int>(pipelines.size()); i++)
            if (i != pipelineIndex)
                sumBlendFactors += pipelines[i]->blendFactor;
    }

    // Scaling the blend factors like this ensures that its sum is unity

    float scale = (1.0f - factor) / sumBlendFactors;

    for (int i = 0; i < static_cast<int>(pipelines.size()); i++)
        if (i != pipelineIndex)
            pipelines[i]->blendFactor *= scale;

    pipelines[pipelineIndex]->blendFactor = factor;
}

void GeneratorGL::equalizePipelineBlendFactors()
{
    // Make all blend factors equal

    for (auto& pipeline : pipelines)
        pipeline->blendFactor = 1.0f / pipelines.size();
}

void GeneratorGL::insertImageOperation(int pipelineIndex, int newOperationIndex, int currentOperationIndex)
{
    if (pipelineIndex >= 0)
    {
        pipelines[pipelineIndex]->insertImageOperation(newOperationIndex, currentOperationIndex);
    }        
    else
        outputPipeline->insertImageOperation(newOperationIndex, currentOperationIndex);
}

void GeneratorGL::swapImageOperations(int pipelineIndex, int operationIndex0, int operationIndex1)
{
    if (pipelineIndex >= 0)
    {
        pipelines[pipelineIndex]->swapImageOperations(operationIndex0, operationIndex1);
    }        
    else
        outputPipeline->swapImageOperations(operationIndex0, operationIndex1);
}

void GeneratorGL::removeImageOperation(int pipelineIndex, int operationIndex)
{
    if (pipelineIndex >= 0)
    {
        pipelines[pipelineIndex]->removeImageOperation(operationIndex);
    }        
    else
        outputPipeline->removeImageOperation(operationIndex);
}

QString GeneratorGL::getImageOperationName(int pipelineIndex, int operationIndex)
{
    if (pipelineIndex >= 0)
        return pipelines[pipelineIndex]->getImageOperationName(operationIndex);
    else
        return outputPipeline->getImageOperationName(operationIndex);
}

int GeneratorGL::getImageOperationsSize(int pipelineIndex)
{
    if (pipelineIndex >= 0)
        return pipelines.empty() ? 0 : pipelines[pipelineIndex]->getImageOperationsSize();
    else
        return outputPipeline->getImageOperationsSize();
}

void GeneratorGL::setBlendData()
{
    blender->clearInputData();

    for (auto& pipeline : pipelines)
        blender->addInputData(pipeline->getTextureID(), pipeline->blendFactor);
}

void GeneratorGL::iterate()
{
    if (active)
    {
        // Iterate parallel pipelines

        for (auto& pipeline : pipelines)
            pipeline->iterate(outputTextureID);

        // Blend parallel pipelines output images

        if (!pipelines.empty())
        {
            setBlendData();
            blender->blend();
            outputTextureID = blender->getTextureID();
        }

        // Iterate output pipeline

        outputPipeline->iterate(outputTextureID);

        // Last two FBOs useful when no parallel pipelines present and output pipeline empty

        outputFBO[0]->setInputTextureID(outputPipeline->getTextureID());
        outputFBO[0]->draw();
        outputFBO[1]->setInputTextureID(outputFBO[0]->getTextureID());
        outputFBO[1]->draw();

        outputTextureID = outputFBO[1]->getTextureID();

        iteration++;
    }
}

void GeneratorGL::drawRandomSeed(bool grayscale)
{
    seed->drawRandom(grayscale);
    
    // Draw last output FBO with seed texture as input to allow for resize when iterations inactive

    outputFBO[1]->setInputTextureID(seed->getTextureID());
    outputFBO[1]->draw();

    outputTextureID = outputFBO[1]->getTextureID();
}

void GeneratorGL::drawSeedImage()
{
    seed->drawImage();
    
    // Draw last output FBO with seed texture as input to allow for resize when iterations inactive

    outputFBO[1]->setInputTextureID(seed->getTextureID());
    outputFBO[1]->draw();

    outputTextureID = outputFBO[1]->getTextureID();
}

void GeneratorGL::loadSeedImage(QString filename)
{
    seed->loadImage(filename);
}

void GeneratorGL::resize(GLuint width, GLuint height)
{
    FBO::width = width;
    FBO::height = height;

    seed->resize();

    for (auto& pipeline : pipelines)
        pipeline->resize();

    blender->resize();
    blender->resizeOutputFBO();

    outputPipeline->resize();

    outputFBO[0]->resize();
    outputFBO[1]->resize();

    // Reset output texture ID because resize resets texture IDs

    outputTextureID = outputFBO[1]->getTextureID();
}

void GeneratorGL::startRecording(int width, int height)
{
    recording = true;
    encoder = new FFmpegEncoder(recordFilename.toStdString().c_str(), width, height, framesPerSecond, preset.toStdString().c_str(), QString::number(crf).toStdString().c_str());
}

void GeneratorGL::stopRecording()
{
    recording = false;
    delete encoder;
    encoder = nullptr;
}

void GeneratorGL::record()
{
    encoder->recordFrame();
}

void GeneratorGL::setMask(bool apply)
{
    applyMask = apply;

    FBO::morphoWidget->makeCurrent();

    outputFBO[1]->program->bind();
    outputFBO[1]->program->setUniformValue("apply", apply);
    outputFBO[1]->program->release();

    FBO::morphoWidget->doneCurrent();
}