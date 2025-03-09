//
// Created by 51092 on 25-3-6.
//

#include "SyncManager.h"


void VK::Instances::SyncManager::createSyncObjects(const VK::Device& device,const uint32_t &FenceNum, const uint32_t &SemaphoreNum) {
    this->device = device.vkDevice;
    Fences.resize(FenceNum);
    Semaphores.resize(SemaphoreNum);
    VkSemaphoreCreateInfo SemaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VkFenceCreateInfo FenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,//代表初始化Fence可用
    };
    for (uint32_t i = 0; i < SemaphoreNum; i++) {
        if (vkCreateSemaphore(device.vkDevice,  &SemaphoreInfo, nullptr, &Semaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphore");
        }
    }
    for (uint32_t i = 0; i < FenceNum; i++) {
        if (vkCreateFence(device.vkDevice,  &FenceInfo, nullptr, &Fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create fence");
        }
    }

}

void VK::Instances::SyncManager::destroySyncObjects() const{
    for (auto & Semaphore : Semaphores) {
        vkDestroySemaphore(device, Semaphore, nullptr);
    }
    for (auto & Fence : Fences) {
        vkDestroyFence(device, Fence, nullptr);
    }
}
