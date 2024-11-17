#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform vec2 offset[25];
uniform float opacity;

void main() {
    vec3 histogram[10];

    for (int i = 0; i < 10; i++)
        histogram[i] = vec3(0.0);

    for (int i = 0; i < 25; i++)
    {
        vec3 color = texture(inTexture, texCoords + offset[i]).rgb;

        vec3 index = floor(color * 9.0);

        histogram[int(index.r)].r += 1.0;
        histogram[int(index.g)].g += 1.0;
        histogram[int(index.b)].b += 1.0;
    }

    vec3 cdf[10];
    cdf[0] = histogram[0] / 25.0;

    for (int i = 1; i < 10; i++)
        cdf[i] = histogram[i] / 25.0 + cdf[i - 1];

    vec3 srcColor = texture(inTexture, texCoords).rgb;
    vec3 srcIndex = floor(srcColor * 9.0);

    vec3 dstColor = vec3(cdf[int(srcIndex.r)].r, cdf[int(srcIndex.g)].g, cdf[int(srcIndex.b)].b);

    fragColor = vec4(mix(srcColor, dstColor, opacity), 1.0);
}
