#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform float contrast;

void main()
{
	fragColor = vec4((texture(inTexture, texCoords).rgb - 0.5) * contrast + 0.5, 1.0);
}