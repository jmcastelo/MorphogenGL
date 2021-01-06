#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform float kernel[500];
uniform float centerElement;
uniform vec2 offset[500];
uniform int numElements;

void main()
{
    vec3 color = centerElement * texture(inTexture, texCoords).rgb;

    for (int i = 0; i < numElements; i++)
        color += kernel[i] * texture(inTexture, texCoords + offset[i]).rgb;

    fragColor = vec4(color, 1.0);
}
