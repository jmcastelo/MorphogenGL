#ifndef INPUTDATA_H
#define INPUTDATA_H



#include <QOpenGLFunctions>



// Types of inputs

enum class InputType { Normal, Blit, Seed };



// Input for image operation, corresponding to edges

class InputData
{
public:
    InputData(InputType inputType, GLuint* pTexId, float blendFactor) :
        mType { inputType },
        mpTextureId { pTexId },
        mBlendFactor { blendFactor }
    {}

    InputType type() const { return mType; }
    void setType(InputType type){ mType = type; }

    GLuint* pTextureId() { return mpTextureId; }
    void setpTextureId(GLuint* pTexId) { mpTextureId = pTexId; }

    float blendFactor() const { return mBlendFactor; }
    void setBlendFactor(float blendFactor) { mBlendFactor = blendFactor; }

private:
    InputType mType;
    GLuint* mpTextureId;
    float mBlendFactor;
};



#endif // INPUTDATA_H
