//
// Created by 51092 on 25-3-4.
//

#ifndef FRAMEBUFFERS_H
#define FRAMEBUFFERS_H
#include <vector>
#include <vulkan/vulkan_core.h>
#include "../Core/Device.h"
#include "../Presentation/SwapChain.h"
#include "RenderPass.h"

namespace VK::Render {
    class FrameBuffer {
    public:
        VkFramebuffer Buffer{};
        void createFrameBuffers(const VK::Device& device, const RenderPass& renderPass,const VK::SwapChain &swapChain,
            const std::vector<VkImageView>& attachments);
        void destroyFrameBuffers() const;

    private:
        VkDevice m_Device{};
    };
}



#endif //FRAMEBUFFERS_H
