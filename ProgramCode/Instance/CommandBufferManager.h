//
// Created by 51092 on 25-3-4.
//

#ifndef COMMANDBUFFERMANAGER_H
#define COMMANDBUFFERMANAGER_H
#include <vector>
#include <vulkan/vulkan.h>

#include "../Core/Device.h"
#include "../Queue/QueueFamily.h"
#include "../Render/FrameBuffers.h"
#include "../Render/Pipeline.h"
#include "../Render/RenderPass.h"

namespace VK::Instances {
    class CommandBufferManager {
    public:
        std::vector<VkCommandBuffer> commandBuffers;
        void createCommandBuffers(const VK::Device& device, const VK::QueueFamily& queueFamily, uint32_t size);
        static void recordCommandBuffer(const VkCommandBuffer& commandBuffer,
            const VK::Render::RenderPass& renderPass, const VK::Render::Pipeline& graphicsPipeline,
            const VK::Render::FrameBuffers& frameBuffers,
            const VK::SwapChain& swapChain,
            const VK::Instances::DescriptorManager& descriptorManager,
            uint32_t imageIndex
            );
        void destroyCommandBuffers();
    private:
        VkDevice device{};
        VkCommandPool commandPool{};
        void createCommandPool(const VK::QueueFamily& queueFamily);
    };
}


#endif //COMMANDBUFFERMANAGER_H
