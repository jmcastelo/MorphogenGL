#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform float r;

void main(void)
{
    vec3 x = texture(inTexture, texCoords).rgb;
    fragColor = vec4(r * x * (1.0 - x), 1.0);
}
