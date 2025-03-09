//
// Created by 51092 on 25-3-4.
//

#include "CommandBufferManager.h"

#include <stdexcept>

void VK::Instances::CommandBufferManager::createCommandBuffers(const VK::Device &device,uint32_t size) {
    this->device = device;
    createCommandPool();
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = size
    };
    commandBuffers.resize(size);
    if (vkAllocateCommandBuffers(device.vkDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VK::Instances::CommandBufferManager::destroyCommandBuffers() const{
    vkDestroyCommandPool(device.vkDevice, commandPool, nullptr);
}

void VK::Instances::CommandBufferManager::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = device.graphicsQueueFamilyIndex
    };
    if (vkCreateCommandPool(device.vkDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}
