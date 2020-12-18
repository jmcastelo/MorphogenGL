#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;
uniform bool apply;

void main()
{
	if (apply && distance(texCoords, vec2(0.5, 0.5)) > 0.5)
		fragColor = vec4(0.0, 0.0, 0.0, 1.0);
	else
		fragColor = texture(inTexture, texCoords);
}