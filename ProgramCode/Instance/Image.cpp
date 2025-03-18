//
// Created by 51092 on 25-3-6.
//

#include "Image.h"
#include "../Utils/utils.h"

void VK::Instances::Image::Map() {
    vkMapMemory(device.vkDevice, imageMemory, 0, size, 0, &data);
}

void VK::Instances::Image::UnMap() const {
    vkUnmapMemory(device.vkDevice, imageMemory);
}

void VK::Instances::Image::destroyImage() const {
    if (imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device.vkDevice, imageView, nullptr);
    }
    vkDestroyImage(device.vkDevice, image, nullptr);
    vkFreeMemory(device.vkDevice, imageMemory, nullptr);
}

// 辅助函数：计算每个像素的字节数
uint32_t getFormatSize(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_D32_SFLOAT:
            return 4;
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_SFLOAT: return 8;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_BC7_UNORM_BLOCK: return 16; // 压缩格式特殊处理
        default: throw std::runtime_error("Unsupported format");
    }
}

void VK::Instances::Image::copy(VkBuffer buffer, VkCommandPool commandPool) const {
    if (arrayNum <= 1) {
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
        region.imageOffset = {0, 0, 0}; //复制图像的偏移
        region.imageExtent = {width, height, 1};
        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, //布局
            1,
            &region
        );
        Utils::endSingleTimeCommands(device, commandPool, commandBuffer);
    } else {
        VkCommandBuffer cmdBuf = Utils::beginSingleTimeCommands(device, commandPool);

        std::vector<VkBufferImageCopy> regions(arrayNum);

        // 计算每层数据在buffer中的大小
        const VkDeviceSize layerSize = width * height * getFormatSize(format);

        for (uint32_t i = 0; i < arrayNum; ++i) {
            regions[i].bufferOffset = i * layerSize; // 自动计算偏移
            regions[i].bufferRowLength = 0; // 紧密排列
            regions[i].bufferImageHeight = 0;

            regions[i].imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0 + i, // 目标层
                .layerCount = 1
            };

            regions[i].imageOffset = {0, 0, 0};
            regions[i].imageExtent = {width, height, 1};
        }

        vkCmdCopyBufferToImage(
            cmdBuf,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            arrayNum,
            regions.data()
        );

        Utils::endSingleTimeCommands(device, commandPool, cmdBuf);
    }
}


void VK::Instances::Image::createImage(const VK::Device &device, uint32_t width, uint32_t height, uint32_t mipLevels,
                                       uint32_t arrayNum,
                                       VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
                                       VkImageUsageFlags usage,
                                       VkMemoryPropertyFlags properties) {
    this->device = device;
    this->arrayNum = arrayNum;
    this->format = format;
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = arrayNum;
    imageInfo.samples = numSamples;
    imageInfo.usage = usage;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    size = width * height * getFormatSize(format);
    this->width = width;
    this->height = height;

    if (vkCreateImage(device.vkDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device.vkDevice, image, &memRequirements);
    //查询图像所需的内存要求：大小、对齐要求、和图像支持的物理设备的内存类型

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
            Utils::findMemoryType(device.physicalDevice, memRequirements.memoryTypeBits, properties);
    if (vkAllocateMemory(device.vkDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate memory!");
    }
    vkBindImageMemory(device.vkDevice, image, imageMemory, 0);
}
