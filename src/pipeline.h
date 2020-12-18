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

#include "imageoperations.h"
#include <vector>
#include <QString>

// Pipeline: chain of iterated operations

class Pipeline
{
public:
    std::vector<ImageOperation*> imageOperations;

    float blendFactor;

    Pipeline(GLuint id, float blend);
    ~Pipeline();

    void iterate(GLuint id);

    ImageOperation* getImageOperation(int index) { return imageOperations[index]; };

    void swapImageOperations(int operationIndex0, int operationIndex1);
    void removeImageOperation(int operationIndex);
    void insertImageOperation(int newOperationIndex, int currentOperationIndex);
    void loadImageOperation(
        QString operationName,
        bool enabled,
        std::vector<bool> boolParameters,
        std::vector<int> intParameters,
        std::vector<float> floatParameters,
        std::vector<int> interpolationParameters,
        std::vector<float> kernelElements,
        std::vector<float> matrixElements);

    QString getImageOperationName(int operationIndex) { return imageOperations[operationIndex]->getName(); };
    int getImageOperationsSize() { return imageOperations.size(); };

    GLuint getTextureID();

    void resize();

private:
    std::vector<QString> availableImageOperations;
    GLuint textureID;
};