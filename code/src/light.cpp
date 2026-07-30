#define _USE_MATH_DEFINES
#include "light.h"
#include <cmath>

void generateSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, float radius, int segments) {
    vertices.clear();
    indices.clear();

    for (int i = 0; i <= segments; i++) {
        float theta = i * M_PI / segments;
        for (int j = 0; j <= segments; j++) {
            float phi = j * 2 * M_PI / segments;

            float x = radius * sin(theta) * cos(phi);
            float y = radius * cos(theta);
            float z = radius * sin(theta) * sin(phi);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            float len = sqrt(x*x + y*y + z*z);
            vertices.push_back(x / len);
            vertices.push_back(y / len);
            vertices.push_back(z / len);
        }
    }

    for (int i = 0; i < segments; i++) {
        for (int j = 0; j < segments; j++) {
            int a = i * (segments + 1) + j;
            int b = i * (segments + 1) + j + 1;
            int c = (i + 1) * (segments + 1) + j;
            int d = (i + 1) * (segments + 1) + j + 1;

            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(c);

            indices.push_back(b);
            indices.push_back(d);
            indices.push_back(c);
        }
    }
}