//
// Created by cai on 25-2-27.
//

#ifndef VERTEX_H
#define VERTEX_H
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

struct Vertex {
    glm::vec3 pos{};
    glm::vec3 color{};
    glm::vec3 normal{};
    glm::vec2 texCoord{};

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);//类似OpenGL
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;//移至每个顶点后的下一个数据条目,在内存中加载的数据

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};
        attributeDescriptions[0].binding = 0;//告诉Vulkan每个顶点数据来自哪个绑定
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;//对应的是vec3
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; //对应的是vec3
        attributeDescriptions[1].offset = offsetof(Vertex, color);//内存中位置的偏移
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT; //对应的是vec3
        attributeDescriptions[2].offset = offsetof(Vertex, normal);//内存中位置的偏移
        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, texCoord);//纹理坐标

        return attributeDescriptions;
    }

};

#endif //VERTEX_H
