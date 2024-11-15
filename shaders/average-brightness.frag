#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform vec2 offset[24];
uniform vec3 factor;
uniform float opacity;

void main() {
    // Calculate the average brightness of a 5x5 sample of the texture
    vec3 avgColor = vec3(0.0);
    for (int i = 0; i < 24; i++)
        avgColor += texture(inTexture, texCoords + offset[i]).rgb;
    avgColor /= 24.0;

    // Adjust the brightness of the current pixel based on the average brightness
    vec3 srcColor = texture(inTexture, texCoords).rgb;
    //vec3 dstColor = srcColor * (0.5 + (avgColor.r + avgColor.g + avgColor.b) / 3.0);

    //float avgBrightness = (avgColor.r + avgColor.g + avgColor.b) / 3.0;
    //float curBrightness = (srcColor.r + srcColor.g + srcColor.b) / 3.0;

    vec3 dstColor = clamp(srcColor + (avgColor - srcColor) * factor, 0.0, 1.0);

    fragColor = vec4(mix(srcColor, dstColor, opacity), 1.0);
}
