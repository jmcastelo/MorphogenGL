#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform vec3 gamma;

void main()
{
    fragColor = vec4(pow(texture(inTexture, texCoords).rgb, gamma), 1.0);
}
