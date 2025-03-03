//
// Created by 51092 on 25-3-3.
//

#ifndef RENDERPASS_H
#define RENDERPASS_H
#include <vulkan/vulkan_core.h>
#include <vector>

namespace VK::Render {
    class RenderPass {
    public:
        VkRenderPass m_renderPass;

    private:
        VkDevice deivce;
    };
}



#endif //RENDERPASS_H
