#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform float spatialKernel[49];
uniform vec2 offset[49];
uniform float rangeSigma;
uniform int numElements;

void main()
{
    vec3 centerColor = texture(inTexture, texCoords).rgb;

    vec3 color = vec3(0.0);
    float sum = 0.0;

    for (int i = 0; i < numElements; i++)
    {
        vec3 offsetColor = texture(inTexture, texCoords + offset[i]).rgb;

        float rangeKernel = exp(-0.5 * dot(centerColor - offsetColor, centerColor - offsetColor) / (rangeSigma * rangeSigma));
        float kernel = spatialKernel[i] * rangeKernel;

        color += kernel * offsetColor;

        sum += kernel;
    }

    fragColor = vec4(color / sum, 1.0);
}
