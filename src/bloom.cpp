#include "bloom.h"
#include "bloom_shaders.h"
#include <iostream>

static unsigned int quadVAO = 0, quadVBO = 0;
static unsigned int fboScene, rboScene;
static unsigned int texScene;          // 原始场景颜色
static unsigned int fboBloom[2];
static unsigned int texBloom[2];       // 0:提取亮部, 1:模糊后
static unsigned int shaderExtract, shaderBlur, shaderFinal;
static int screenW, screenH;

// 工具函数：编译着色器
static unsigned int compileShader(const char* vertSrc, const char* fragSrc) {
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertSrc, NULL);
    glCompileShader(vs);
    // 可加编译检查，此处省略
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragSrc, NULL);
    glCompileShader(fs);
    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// 全屏四边形顶点数据
static void setupQuad() {
    if (quadVAO != 0) return;
    float vertices[] = {
        // x, y, u, v
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void initBloom(int width, int height) {
    screenW = width; screenH = height;
    setupQuad();

    // 场景 FBO + 颜色纹理
    glGenFramebuffers(1, &fboScene);
    glBindFramebuffer(GL_FRAMEBUFFER, fboScene);
    glGenTextures(1, &texScene);
    glBindTexture(GL_TEXTURE_2D, texScene);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texScene, 0);
    // 深度缓冲
    glGenRenderbuffers(1, &rboScene);
    glBindRenderbuffer(GL_RENDERBUFFER, rboScene);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboScene);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Bloom 用的两个 FBO（乒乓模糊）
    glGenFramebuffers(2, fboBloom);
    glGenTextures(2, texBloom);
    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[i]);
        glBindTexture(GL_TEXTURE_2D, texBloom[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texBloom[i], 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 编译着色器
    shaderExtract = compileShader(bloomExtractVert, bloomExtractFrag);
    shaderBlur    = compileShader(bloomBlurVert, bloomBlurFrag);
    shaderFinal   = compileShader(bloomFinalVert, bloomFinalFrag);
}

unsigned int getBloomSceneFBO() {
    return fboScene;
}

void resizeBloom(int width, int height) {
    screenW = width; screenH = height;

    // 删除旧纹理
    glDeleteTextures(1, &texScene);
    glDeleteRenderbuffers(1, &rboScene);
    glDeleteFramebuffers(1, &fboScene);
    glDeleteTextures(2, texBloom);
    glDeleteFramebuffers(2, fboBloom);

    // ----- 重建场景 FBO -----
    glGenFramebuffers(1, &fboScene);
    glBindFramebuffer(GL_FRAMEBUFFER, fboScene);

    glGenTextures(1, &texScene);
    glBindTexture(GL_TEXTURE_2D, texScene);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texScene, 0);

    glGenRenderbuffers(1, &rboScene);
    glBindRenderbuffer(GL_RENDERBUFFER, rboScene);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboScene);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "resizeBloom: scene FBO incomplete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ----- 重建两个 Bloom 乒乓纹理 -----
    glGenFramebuffers(2, fboBloom);
    glGenTextures(2, texBloom);
    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[i]);
        glBindTexture(GL_TEXTURE_2D, texBloom[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texBloom[i], 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void applyBloom(unsigned int sceneTexture /* 如果传0则使用内部texScene */) {
    // 如果传入了外部纹理则用外部，否则用我们自己的fboScene
    unsigned int srcTex = sceneTexture != 0 ? sceneTexture : texScene;

    // 1. 提取亮部到 texBloom[0]
    glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[0]);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderExtract);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTex);
    glUniform1i(glGetUniformLocation(shaderExtract, "sceneTexture"), 0);
    glUniform1f(glGetUniformLocation(shaderExtract, "threshold"), 0.6f);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 2. 水平模糊到 texBloom[1]
    glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[1]);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderBlur);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texBloom[0]);
    glUniform1i(glGetUniformLocation(shaderBlur, "image"), 0);
    glUniform1i(glGetUniformLocation(shaderBlur, "horizontal"), 1);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 3. 垂直模糊到 texBloom[0]
    glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[0]);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderBlur);
    glBindTexture(GL_TEXTURE_2D, texBloom[1]);
    glUniform1i(glGetUniformLocation(shaderBlur, "horizontal"), 0);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 4. 合成到默认帧缓冲（屏幕）
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderFinal);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTex);
    glUniform1i(glGetUniformLocation(shaderFinal, "sceneTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texBloom[0]);
    glUniform1i(glGetUniformLocation(shaderFinal, "bloomBlur"), 1);
    glUniform1f(glGetUniformLocation(shaderFinal, "exposure"), 1.0f);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}