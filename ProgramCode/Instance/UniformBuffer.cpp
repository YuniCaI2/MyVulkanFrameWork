//
// Created by 51092 on 25-3-6.
//

#include "UniformBuffer.h"
#include <glm/glm.hpp>
#include<iostream>
#include "UniformBufferObject.h"


void VK::Instances::UniformBuffer::update(const UniformBuffer &uniformBuffer, const VkExtent2D &extent,
    const Camera& camera, const std::vector<Light>& lights) {

    UniformBufferObject ubo = {};
    // ubo.model = glm::mat4(1.0f);
    // ubo.model = glm::translate(ubo.model, glm::vec3(0.0f, 0.0f, -5.0f));

    ubo.viewPos = camera.Position;//相机坐标
    // std::cout << camera.Position.x << camera.Position.y << camera.Position.z << std::endl;

    ubo.view = camera.GetViewMatrix();
    ubo.proj = glm::perspective(glm::radians(camera.Zoom),
        static_cast<float>(extent.width) / static_cast<float>(extent.height), 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;
    ubo.lightCount = lights.size();
    for (int i = 0; i < lights.size(); i++) {
        ubo.light[i].color = lights[i].color;
        ubo.light[i].position = lights[i].position;
        ubo.light[i].direction = lights[i].direction;
        ubo.light[i].type = static_cast<int>(lights[i].type);
        ubo.light[i].spotAngle = lights[i].spotAngle;
        ubo.light[i].spotFalloff = lights[i].spotFalloff;
    }
    //因为glm是为opengl设计的，在opengl中，y轴朝下才是正的，Vulkan是反过来的。
    memcpy(uniformBuffer.buffer.data, &ubo, sizeof(ubo));
    //前一个参数是映射内存的指针
    //将小缓冲区数据传递给着色器的更有效方法是推送常量。我们可能会在以后的章节中讨论这些内容，这里不是最有效的。

}
