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



#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H



#include "texformat.h"
#include "imageoperation.h"
#include "seed.h"
#include "factory.h"

#include <QObject>
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QImage>



class RenderManager : public QObject, protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    RenderManager(Factory* factory);
    ~RenderManager();

    void init(QOpenGLContext* shareContext);

    // Seed* createNewSeed();
    // void deleteSeed(Seed* seed);

    // ImageOperation* createNewOperation();

    bool active() const;
    void setActive(bool set);

    void iterate();

    QImage outputImage();
    bool rgbPixel(QPoint pos, float* rgb);

    TextureFormat texFormat();
    void setTextureFormat(TextureFormat format);

    GLuint texWidth();
    GLuint texHeight();

    void clearAllOpsTextures();

    void drawAllSeeds();

    void resetIterationNumer();
    int iterationNumber();

signals:
    void texturesChanged();

public slots:
    void resize(GLuint width, GLuint height);
    void setOutputTextureId(GLuint* pTexId);
    void initOperation(QUuid id, ImageOperation* operation);
    void initSeed(QUuid id, Seed* seed);
    void setSortedOperations(QList<ImageOperation*> sortedOperations);
    void reset();

private:
    Factory* mFactory;

    QOpenGLContext* mContext = nullptr;
    QOffscreenSurface* mSurface = nullptr;

    GLuint mOutFbo = 0;
    GLuint mReadFbo = 0;
    GLuint mDrawFbo = 0;

    // QList<Seed*> mSeeds;
    // QList<ImageOperation*> mOperations;
    QList<ImageOperation*> mSortedOperations;

    GLuint mTexWidth = 2048;
    GLuint mTexHeight = 2048;

    GLuint mOldTexWidth = 2048;
    GLuint mOldTexHeight = 2048;

    TextureFormat mTexFormat = TextureFormat::RGBA8;

    GLint mMaxArrayTexLayers;
    const GLint mNumArrayTexLayers = 32;

    QOpenGLShaderProgram* mBlenderProgram;
    QOpenGLShaderProgram* mIdentityProgram;

    GLuint mVao;
    GLuint mVboPos;
    GLuint mVboTex;

    GLuint* mOutputTexId = nullptr;
    GLuint mBlendArrayTexId = 0;

    bool mActive = false;
    unsigned int mIterationNumber = 0;

    GLsync fence = 0;

    GLenum getFormat(GLenum format);

    void setBlenderProgram();
    void setIdentityProgram();

    void verticesCoords(GLfloat& left, GLfloat& right, GLfloat& bottom, GLfloat& top);
    void setVao();
    void adjustOrtho();

    void genTexture(GLuint* texId);
    void genOpTextures(ImageOperation* operation);
    void resizeTextures();

    void genBlendArrayTexture();
    void recreateBlendArrayTexture();
    void copyTexturesToBlendArrayTexture(QList<GLuint *> textures);

    void clearTexture(GLuint* texId);

    void blit();
    void blend(ImageOperation* operation);
    void renderOperation(ImageOperation* operation);
    void render();
};



#endif // RENDERMANAGER_H
