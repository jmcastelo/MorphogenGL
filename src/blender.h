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



#ifndef BLENDER_H
#define BLENDER_H



#include "fbo.h"
#include <QUuid>
#include <QOpenGLContext>



enum class InputType { Normal, Blit, Seed };



// Input for image operation, corresponding to edges

struct InputData
{
    InputType type;
    GLuint** textureID;
    float blendFactor;

    InputData(InputType inputType, GLuint** ID, float factor) :
        type { inputType },
        textureID { ID },
        blendFactor { factor }
    {}
};



class Blender : public FBO
{
public:
    Blender(QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext);
    ~Blender();

    void setInputData(QList<InputData*> data) { inputs = data; }

    void resize() override;

    void blend();

private:
    QList<InputData*> inputs;
    FBO* fboOut;
};


#endif // BLENDER_H
