#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

// ========== 函数声明 ==========
bool loadModel(const std::string& path, std::vector<float>& outVertices, std::vector<unsigned int>& outIndices);

#endif