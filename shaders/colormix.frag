#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;
uniform mat3 rgbMatrix;

void main()
{
	fragColor = vec4(rgbMatrix * texture(inTexture, texCoords).rgb, 1.0);
}