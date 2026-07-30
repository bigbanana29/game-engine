#ifndef BLOOM_H
#define BLOOM_H
#include <glad/glad.h>

void initBloom(int width, int height);
void applyBloom(unsigned int sceneTexture);
void resizeBloom(int width, int height);
void cleanupBloom();
unsigned int getBloomSceneFBO();

#endif