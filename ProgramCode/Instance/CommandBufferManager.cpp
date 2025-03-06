//
// Created by 51092 on 25-3-4.
//

#include "CommandBufferManager.h"

#include <stdexcept>

void VK::Instances::CommandBufferManager::createCommandBuffers(const VK::Device &device,
    const VK::QueueFamily &queueFamily, uint32_t size) {
    this->device = device.device;
    createCommandPool(queueFamily);
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = commandPool,
        .commandBufferCount = size
    };
    if (vkAllocateCommandBuffers(device.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VK::Instances::CommandBufferManager::destroyCommandBuffers() const{
    vkDestroyCommandPool(device, commandPool, nullptr);
}

void VK::Instances::CommandBufferManager::createCommandPool(const VK::QueueFamily &queueFamily) {
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamily.graphicsFamily.value()
    };
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}
