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

#pragma once

#include "pipeline.h"
#include "seed.h"
#include "blender.h"
#include "fbo.h"
#include "ffmpegencoder.h"
#include <vector>
#include <QString>
#include <QImage>
#include <QOpenGLContext>

class GeneratorGL
{
public:
    std::vector<QString> availableImageOperations;

    std::vector<Pipeline*> pipelines;
    Pipeline* outputPipeline;

    bool active = false;

    bool recording = false;
    
    int framesPerSecond = 50;
    QString recordFilename;
    QString preset = "ultrafast";
    int crf = 17;

    bool applyMask = false;

    GeneratorGL();
    ~GeneratorGL();

    void init(QOpenGLContext* mainContext);

    void addPipeline();
    void removePipeline(int pipelineIndex);
    void loadPipeline(float blendFactor);
    int getPipelinesSize() { return pipelines.size(); };

    void setPipelineBlendFactor(int pipelineIndex, float factor);
    double getPipelineBlendFactor(int pipelineIndex) { return pipelines[pipelineIndex]->blendFactor; }
    void equalizePipelineBlendFactors();

    void insertImageOperation(int pipelineIndex, int newOperationIndex, int currentOperationIndex);
    void swapImageOperations(int pipelineIndex, int operationIndex0, int operationIndex1);
    void removeImageOperation(int pipelineIndex, int operationIndex);
    
    QString getImageOperationName(int pipelineIndex, int operationIndex);
    int getImageOperationsSize(int pipelineIndex);

    void iterate();

    void resetIterationNumer() { iteration = 0; }
    int getIterationNumber() { return iteration; }

    void drawRandomSeed(bool grayscale);
    void drawSeedImage();
    void loadSeedImage(QString filename);

    GLuint getOutputTextureID() { return outputTextureID; };

    void resize(GLuint width, GLuint height);
    int getWidth() { return FBO::width; }
    int getHeight() { return FBO::height; }

    void startRecording(int width, int height, QOpenGLContext* mainContext);
    void stopRecording();
    void record();
    bool isRecording() { return recording; }
    int getFrameCount() { return encoder ? encoder->frameNumber : 0; }

    void setMask(bool apply);

private:
    QOpenGLContext* sharedContext;
    GLuint outputTextureID = 0;
    unsigned int iteration = 0;

    Seed* seed = nullptr;
    Blender* blender = nullptr;
    FBO* outputFBO[2] = { nullptr, nullptr };

    FFmpegEncoder* encoder = nullptr;
    
    void setBlendData();
};