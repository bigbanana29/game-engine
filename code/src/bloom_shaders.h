#ifndef BLOOM_SHADERS_H
#define BLOOM_SHADERS_H

const char* bloomExtractVert = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
out vec2 TexCoords;
void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
)";

const char* bloomExtractFrag = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D sceneTexture;
uniform float threshold;  // 亮度阈值
void main() {
    vec3 color = texture(sceneTexture, TexCoords).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > threshold)
        FragColor = vec4(color, 1.0);
    else
        FragColor = vec4(0.0);
}
)";

const char* bloomBlurVert = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
out vec2 TexCoords;
void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
)";

const char* bloomBlurFrag = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D image;
uniform bool horizontal;
uniform float weight[5] = float[] (
    0.2270270270, 0.1945945946, 0.1216216216,
    0.0540540541, 0.0162162162
);
void main() {
    vec2 tex_offset = 1.0 / textureSize(image, 0);
    vec3 result = texture(image, TexCoords).rgb * weight[0];
    if (horizontal) {
        for (int i = 1; i < 5; ++i) {
            result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    } else {
        for (int i = 1; i < 5; ++i) {
            result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}
)";

const char* bloomFinalVert = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
out vec2 TexCoords;
void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
)";

const char* bloomFinalFrag = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D sceneTexture;
uniform sampler2D bloomBlur;
uniform float exposure; // 曝光度
void main() {
    vec3 sceneColor = texture(sceneTexture, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    vec3 finalColor = sceneColor + bloomColor; // 简单叠加
    // 简易色调映射
    finalColor = vec3(1.0) - exp(-finalColor * exposure);
    FragColor = vec4(finalColor, 1.0);
}
)";
#endif