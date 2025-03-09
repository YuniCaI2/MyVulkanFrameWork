//
// Created by 51092 on 25-3-4.
//

#ifndef COMMANDBUFFERMANAGER_H
#define COMMANDBUFFERMANAGER_H
#include <vector>

#include "../Core/Device.h"
#include "../Render/FrameBuffer.h"


namespace VK::Instances {
    class CommandBufferManager {
    public:
        std::vector<VkCommandBuffer> commandBuffers;
        VkCommandPool commandPool{};
        void createCommandBuffers(const VK::Device& device, uint32_t size);
        void destroyCommandBuffers() const;
    private:
        VK::Device device{};
        void createCommandPool();
    };
}


#endif //COMMANDBUFFERMANAGER_H
