#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform float kernel[9];
uniform vec2 offset[9];

void main()
{
    vec3 color = vec3(0.0);

    for (int i = 0; i < 9; i++)
        color += kernel[i] * texture(inTexture, texCoords + offset[i]).rgb;

    fragColor = vec4(color, 1.0);
}
