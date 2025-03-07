//
// Created by 51092 on 25-3-4.
//

#ifndef COMMANDBUFFERMANAGER_H
#define COMMANDBUFFERMANAGER_H
#include <vector>

#include "../Core/Device.h"
#include "../Queue/QueueFamily.h"
#include "../Render/FrameBuffers.h"


namespace VK::Instances {
    class CommandBufferManager {
    public:
        std::vector<VkCommandBuffer> commandBuffers;
        VkCommandPool commandPool{};
        void createCommandBuffers(const VK::Device& device, const VK::QueueFamily& queueFamily, uint32_t size);
        void destroyCommandBuffers() const;
    private:
        VkDevice device{};
        void createCommandPool(const VK::QueueFamily& queueFamily);
    };
}


#endif //COMMANDBUFFERMANAGER_H
