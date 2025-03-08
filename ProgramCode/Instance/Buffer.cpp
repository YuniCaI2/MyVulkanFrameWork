//
// Created by 51092 on 25-3-3.
//

#include "Buffer.h"
#include "../Utils/utils.h"
void VK::Instances::Buffer::createBuffer(const VK::Device &device, const VkDeviceSize &size,
    const VkBufferUsageFlags &usage, const VkMemoryPropertyFlags &properties) {
    this->device = device;
    this->size = size;
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device.device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = Utils::findMemoryType(device.physicalDevice,memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device.device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory");
    }

    vkBindBufferMemory(device.device, buffer, memory, 0);
}

void VK::Instances::Buffer::Map() {
    vkMapMemory(device.device, memory, 0, size, 0,&data);
}

void VK::Instances::Buffer::UnMap() const {
    vkUnmapMemory(device.device, memory);
}

void VK::Instances::Buffer::destroyBuffer() const {
    vkDestroyBuffer(device.device, buffer, nullptr);
    vkFreeMemory(device.device, memory, nullptr);
}

void VK::Instances::Buffer::copy(VkBuffer dstBuffer, VkCommandPool commandPool) const{
    VkCommandBuffer commandBuffer = Utils::beginSingleTimeCommands(device, commandPool);
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer,  dstBuffer,buffer, 1, &copyRegion);
    Utils::endSingleTimeCommands(device, commandPool, commandBuffer);
}
