#ifndef INPUTDATA_H
#define INPUTDATA_H



#include <QOpenGLFunctions>



// Types of inputs

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



#endif // INPUTDATA_H
