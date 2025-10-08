#ifndef INPUTDATA_H
#define INPUTDATA_H



#include <QOpenGLFunctions>
#include "parameters/number.h"


// Types of inputs

enum class InputType { Normal, Blit, Seed };



// Input for image operation, corresponding to edges

class InputData
{
public:
    InputData(InputType inputType, GLuint* pTexId, float blendFactor) :
        mType { inputType },
        mpTextureId { pTexId }
    {
        mBlendFactor = new Number<float>(blendFactor, 0.0, 1.0, 0.0, 1.0);
    }
    ~InputData()
    {
        delete mBlendFactor;
    }

    InputType type() const { return mType; }
    void setType(InputType type){ mType = type; }

    GLuint* pTextureId() { return mpTextureId; }
    void setpTextureId(GLuint* pTexId) { mpTextureId = pTexId; }

    Number<float>* blendFactor() const { return mBlendFactor; }
    void setBlendFactor(float value) { mBlendFactor->setValue(value); }

private:
    InputType mType;
    GLuint* mpTextureId;
    Number<float>* mBlendFactor;
};



#endif // INPUTDATA_H
