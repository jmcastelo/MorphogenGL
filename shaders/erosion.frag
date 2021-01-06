#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform vec2 offset[8];

void main()
{
    vec3 color = texture(inTexture, texCoords + offset[0]).rgb;

    for (int i = 1; i < 8; i++)
        color = min(color,  texture(inTexture, texCoords + offset[i]).rgb);

    fragColor = vec4(color, 1.0);
}
