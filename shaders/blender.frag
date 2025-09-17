#version 330 core

uniform sampler2D inTextures[32];
uniform int texCount;
uniform float weights[32];

in vec2 texCoords;
out vec4 fragColor;

void main() {
    vec3 acc = vec3(0.0);

    for (int i = 0; i < texCount; i++) {
        acc += texture(inTextures[i], texCoords).rgb * weights[i];
    }

    fragColor = vec4(acc, 1.0);
}
