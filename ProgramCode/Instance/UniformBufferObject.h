//
// Created by 51092 on 25-2-27.
//

#ifndef UNIFORMBUFFEROBJECT_H
#define UNIFORMBUFFEROBJECT_H
#include <glm/glm.hpp>

struct LightUniform {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 color;
    alignas(4) uint32_t type;
    alignas(16) glm::vec3 direction;
    alignas(4) float spotAngle;
    alignas(4) float spotFalloff;
};
// 静态断言确保大小为 80 字节（根据实际情况调整）

struct UniformBufferObject {
    alignas(16) glm::vec3 viewPos;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(4) int32_t lightCount;
    LightUniform light[2];
};

#endif //UNIFORMBUFFEROBJECT_H
