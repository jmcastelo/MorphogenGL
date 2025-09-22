#ifndef INPUTDATA_H
#define INPUTDATA_H



#include <QOpenGLFunctions>



// Types of inputs

enum class InputType { Normal, Blit, Seed };



// Input for image operation, corresponding to edges

class InputData
{
public:
    InputData(InputType inputType, GLuint* texId, float blendFactor) :
        mType { inputType },
        mTextureId { texId },
        mBlendFactor { blendFactor }
    {}

    InputType type() const { return mType; }
    void setType(InputType type){ mType = type; }

    GLuint* textureId() { return mTextureId; }
    void setTextureId(GLuint* texId) { mTextureId = texId; }

    float blendFactor() const { return mBlendFactor; }
    void setBlendFactor(float blendFactor) { mBlendFactor = blendFactor; }

private:
    InputType mType;
    GLuint* mTextureId;
    float mBlendFactor;
};



#endif // INPUTDATA_H
