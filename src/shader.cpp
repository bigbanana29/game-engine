#include "shader.h"
#include <glad/glad.h>
#include <iostream>

// ========== 主着色器（龙） ==========
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexCoord;
    layout (location = 3) in vec3 aTangent;
    uniform mat4 transform;
    out vec3 Normal;
    out vec3 FragPos;
    out vec2 TexCoord;
    out vec3 Tangent;
    void main() {
        gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);
        Normal = mat3(transform) * aNormal;
        FragPos = vec3(transform * vec4(aPos, 1.0));
        TexCoord = aTexCoord;
        Tangent = mat3(transform) * aTangent;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    in vec3 Normal;
    in vec3 FragPos;
    in vec2 TexCoord;
    in vec3 Tangent;
    
    uniform vec3 lightPos;
    uniform vec3 viewPos;
    uniform sampler2D textureSampler;
    uniform sampler2D normalSampler;
    // ====================
    uniform sampler2D roughnessSampler;
    uniform sampler2D metallicSampler;
    // ========== AO ==========
    uniform sampler2D aoSampler;
    uniform float aoValue;
    // ========== 透明度 ==========
    uniform sampler2D opacitySampler;
    // ========== END ==========
    // ========== END ==========
    
    // ========== UI 控制的 uniform ==========
    uniform float lightIntensity;
    uniform float ambientStrength;
    uniform float specularStrength;
    uniform float metallicValue;
    uniform float roughnessValue;
    uniform float diffuseStrength;
    uniform float textureStrength;
    uniform vec3 objectColor;   // 用户控制的颜色
    // ======================================

    // ========== 线框高亮 uniform ==========
    uniform bool isHighlighted;
    uniform vec3 highlightColor;
    uniform float highlightStrength;
    // ========== END ==========
    
    void main() {
        vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
        
        // ========== 采样纹理颜色 ==========
        vec4 texColor = texture(textureSampler, TexCoord);
        vec3 finalColor = mix(objectColor, texColor.rgb, textureStrength);  // 纹理颜色 × 用户颜色
        // ====================================
        
        // ========== 采样粗糙度和金属度 ==========
        float roughness = mix(roughnessValue, texture(roughnessSampler, TexCoord).r, 0.5f);
        float metallic = mix(metallicValue, texture(metallicSampler, TexCoord).r, 0.5f);       
         // ========== 采样 AO ==========
        float ao = texture(aoSampler, TexCoord).r;
        ao = mix(1.0f, ao, aoValue);
        // ========== END ==========
        // ====================================
        
        // ========== 法线贴图采样 ==========
        vec3 normalFromMap = texture(normalSampler, TexCoord).rgb;
        normalFromMap = normalize(normalFromMap * 2.0f - 1.0f);
        
        vec3 N = normalize(Normal);
        vec3 T = normalize(Tangent);
        vec3 B = normalize(cross(N, T));
        mat3 TBN = mat3(T, B, N);
        vec3 finalNormal = normalize(TBN * normalFromMap);
        // ====================================
        
        // ========== 光照计算 ==========
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(finalNormal, lightDir), 0.0f);
        vec3 diffuse = diff * lightColor * lightIntensity * diffuseStrength;
        
        vec3 ambient = ambientStrength * lightColor;
        
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, finalNormal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
        // ========== 粗糙度影响高光 ==========
        float roughnessFactor = 1.0f - roughness;
        vec3 specular = specularStrength * spec * lightColor * lightIntensity * roughnessFactor;
        // ========== END ==========
        
        // ========== 金属度影响漫反射和高光颜色 ==========
        vec3 metalColor = mix(finalColor, vec3(1.0f), metallic);
        vec3 nonMetalColor = finalColor * (1.0f - metallic);
        vec3 result = (ambient + diffuse) * nonMetalColor + specular * metalColor;
        // ========== END ==========

        // ========== 应用 AO（缝隙处变暗） ==========
        result *= (ao > 0.1f ? ao : 1.0f);        
        // ========== END ==========

        // ========== 线框高亮（选中的材质显示线框） ==========
        if (isHighlighted) {
            // 混合高亮颜色
            vec3 highlighted = mix(result, highlightColor, highlightStrength);
            // 应用线框效果（增加边缘轮廓感）
            // 这里用简单的高亮混合，后续可以在 main.cpp 中通过 glPolygonMode 实现线框
            result = highlighted;
        }
        // ========== END ==========
        
        // ========== 透明度支持（临时禁用测试） ==========
        // float alpha = 1.0f;
        // float alphaFromMap = texture(opacitySampler, TexCoord).r;
        // if (alphaFromMap > 0.01f) {  
        //     alpha = alphaFromMap;
        // }
        // FragColor = vec4(result, alpha);
        FragColor = vec4(result, 1.0f);  // 强制不透明
        // ========== END ==========
    }
)";

// ========== 光源小球的着色器 ==========
const char* lightVertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 transform;
    void main() {
        gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    }
)";

const char* lightFragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0f, 0.8f, 0.2f, 1.0f);
    }
)";

// ========== 编译着色器辅助函数 ==========
unsigned int compileShader(const std::string& vertexSource, const std::string& fragmentSource) {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vSrc = vertexSource.c_str();
    glShaderSource(vertexShader, 1, &vSrc, nullptr);
    glCompileShader(vertexShader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "顶点着色器编译失败:\n" << infoLog << std::endl;
        return 0;
    }
    
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fSrc = fragmentSource.c_str();
    glShaderSource(fragmentShader, 1, &fSrc, nullptr);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "片段着色器编译失败:\n" << infoLog << std::endl;
        return 0;
    }
    
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "着色器程序链接失败:\n" << infoLog << std::endl;
        return 0;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

// ========== 创建主着色器程序 ==========
unsigned int createShaderProgram() {
    return compileShader(vertexShaderSource, fragmentShaderSource);
}

// ========== 创建光源小球着色器程序 ==========
unsigned int createLightShaderProgram() {
    return compileShader(lightVertexShaderSource, lightFragmentShaderSource);
}

// ========== 更新光源位置 ==========
void updateLightPosition(unsigned int shaderProgram, float x, float y, float z) {
    int lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    if (lightPosLoc != -1) {
        glUniform3f(lightPosLoc, x, y, z);
    }
}

// ========== 更新观察者位置 ==========
void updateViewPosition(unsigned int shaderProgram, float x, float y, float z) {
    int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    if (viewPosLoc != -1) {
        glUniform3f(viewPosLoc, x, y, z);
    }
}

// ========== 更新光照参数 ==========
void updateLightParams(unsigned int shaderProgram, float intensity, float ambient, float specular) {
    int intensityLoc = glGetUniformLocation(shaderProgram, "lightIntensity");
    int ambientLoc = glGetUniformLocation(shaderProgram, "ambientStrength");
    int specularLoc = glGetUniformLocation(shaderProgram, "specularStrength");
    
    if (intensityLoc != -1) glUniform1f(intensityLoc, intensity);
    if (ambientLoc != -1) glUniform1f(ambientLoc, ambient);
    if (specularLoc != -1) glUniform1f(specularLoc, specular);
}