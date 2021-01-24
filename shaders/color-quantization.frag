#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform int redLevels;
uniform int greenLevels;
uniform int blueLevels;

void main()
{
    vec3 levels = vec3(redLevels, greenLevels, blueLevels);
    fragColor = vec4(floor(levels * texture(inTexture, texCoords).rgb - 0.5) / (levels - 1.0), 1.0);
}
