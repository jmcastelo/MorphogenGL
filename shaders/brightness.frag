#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform float brightness;

void main()
{
	fragColor = vec4(brightness + texture(inTexture, texCoords).rgb, 1.0);
}