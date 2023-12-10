#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform float scaleX;
uniform float scaleY;

void main()
{
    vec2 xy = vec2(texCoords.x * scaleX, texCoords.y * scaleY);
    if (distance(xy, vec2(0.5 * scaleX, 0.5 * scaleY)) > 0.5)
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    else
        fragColor = texture(inTexture, texCoords);
}
