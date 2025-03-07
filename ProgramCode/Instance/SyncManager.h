//
// Created by 51092 on 25-3-6.
//

#ifndef SYNCMANAGER_H
#define SYNCMANAGER_H
#include <vector>
#include <vulkan/vulkan.h>
#include "../Core/Device.h"

namespace VK::Instances {
    class SyncManager {
    public:
        std::vector<VkFence> Fences;
        std::vector<VkSemaphore> Semaphores;

        void createSyncObjects(const VK::Device& device, const uint32_t& FenceNum, const uint32_t& SemaphoreNum);
        void destroySyncObjects() const;
    private:
        VkDevice device{};
    };
}



#endif //SYNCMANAGER_H
