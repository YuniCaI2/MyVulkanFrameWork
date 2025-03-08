//
// Created by 51092 on 25-3-6.
//

#include "Image.h"
#include "../Utils/utils.h"

void VK::Instances::Image::Map() {
    vkMapMemory(device.device, imageMemory, 0, size, 0,&data);
}

void VK::Instances::Image::UnMap() const {
    vkUnmapMemory(device.device, imageMemory);
}

void VK::Instances::Image::destroyImage() const {
    if (imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device.device, imageView, nullptr);
    }
    vkDestroyImage(device.device, image, nullptr);
    vkFreeMemory(device.device, imageMemory, nullptr);
}

void VK::Instances::Image::copy(VkBuffer buffer, VkCommandPool commandPool) const{
    VkCommandBuffer commandBuffer = Utils::beginSingleTimeCommands(device, commandPool);
    VkBufferImageCopy region{};;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    //上述代码代表图片紧密排列

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    //指定复制到图像的哪个部分
    region.imageOffset = {0, 0, 0};//复制图像的偏移
    region.imageExtent = {width, height, 1};
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,//布局
        1,
        &region
        );
    Utils::endSingleTimeCommands(device, commandPool, commandBuffer);
}

void VK::Instances::Image::createImage(const VK::Device &device, uint32_t width, uint32_t height, uint32_t mipLevels,
                                       VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                                       VkMemoryPropertyFlags properties) {
    this->device = device;
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = numSamples;
    imageInfo.usage = usage;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (format == VK_FORMAT_B8G8R8A8_SRGB ||
        format == VK_FORMAT_R8G8B8A8_SRGB) {
        size = width * height * 4;
    }
    this->width = width;
    this->height = height;

    if(vkCreateImage(device.device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device.device, image, &memRequirements);
    //查询图像所需的内存要求：大小、对齐要求、和图像支持的物理设备的内存类型

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = Utils::findMemoryType(device.physicalDevice,memRequirements.memoryTypeBits, properties);
    if(vkAllocateMemory(device.device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate memory!");
    }
    vkBindImageMemory(device.device, image, imageMemory, 0);
}
