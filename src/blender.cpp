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

#include "blender.h"

Blender::Blender(QString vertexShader, QString fragmentShader, QOpenGLContext* mainContext) : FBO(vertexShader, fragmentShader, mainContext)
{
    context->makeCurrent(surface);

    program->bind();

    program->setUniformValue("inTexture", 0);
    program->setUniformValue("outTexture", 1);

    program->release();

    context->doneCurrent();

    fboOut = new FBO(":/shaders/screen.vert", ":/shaders/screen.frag", mainContext);
    fboOut->setInputTextureID(textureID);
}

Blender::~Blender()
{
    delete fboOut;
}

void Blender::resizeOutputFBO()
{
    fboOut->setInputTextureID(textureID);
    fboOut->resize();
}

void Blender::blend()
{
    float outFactor = 0.0f;

    for (auto& inData : inputData)
    {
        context->makeCurrent(surface);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        program->bind();

        program->setUniformValue("blendFactor", inData.blendFactor);
        program->setUniformValue("outFactor", outFactor);

        vao->bind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inData.textureID);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fboOut->getTextureID());

        glDrawArrays(GL_TRIANGLES, 0, 6);

        vao->release();
        program->release();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        context->doneCurrent();

        fboOut->draw();

        outFactor = 1.0f;
    }    
}