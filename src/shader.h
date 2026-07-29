#ifndef SHADER_H
#define SHADER_H

#include <string>

unsigned int createShaderProgram();
unsigned int createLightShaderProgram();
void updateLightPosition(unsigned int shaderProgram, float x, float y, float z);
void updateViewPosition(unsigned int shaderProgram, float x, float y, float z);
void updateLightParams(unsigned int shaderProgram, float intensity, float ambient, float specular);

#endif