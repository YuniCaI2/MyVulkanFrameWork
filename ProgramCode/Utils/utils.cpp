//
// Created by 51092 on 25-2-26.
//

#include "utils.h"
#include <stdexcept>
#include <fstream>
#include <tiny_gltf.h>
#include <opencv2/opencv.hpp>  // 包含 OpenCV，用于加载 OBJ 模型的纹理图像

#include "../Instance/Buffer.h"

VkImageView Utils::createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                                   uint32_t mipLevels, uint32_t arrayNum) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    if (arrayNum > 1) {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE; //兼容CUBEMAP
    }
    if (arrayNum > 1 && (format == VK_FORMAT_R8G8B8A8_SRGB || format == VK_FORMAT_B8G8R8A8_SRGB || format ==
                         VK_FORMAT_R8G8B8A8_UNORM)) {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    }
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = arrayNum;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }
    return imageView;
}

std::vector<char> Utils::readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    //第一个参数的含义是从文件末尾读取，第二个参数是将文件读取为二进制文件
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0); //将光标移到首位
    file.read(buffer.data(), static_cast<uint32_t>(fileSize));
    file.close();
    return buffer;
}

VkFormat Utils::findSupportedFormat(const VkPhysicalDevice &physicalDevice, const std::vector<VkFormat> &candidates,
                                    VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format: candidates) {
        VkFormatProperties props; //这玩意支持三个字段
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supporting format!");
}

VkFormat Utils::findDepthFormat(const VkPhysicalDevice &physicalDevice) {
    return findSupportedFormat(physicalDevice,
                               {
                                   VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                   VK_FORMAT_D24_UNORM_S8_UINT
                               },
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

uint32_t Utils::findMemoryType(const VkPhysicalDevice &physicalDevice, uint32_t typeFilter,
                               VkMemoryPropertyFlags properties) {
    //typeFilter 是支持的资源类型索引的位掩码
    //第一个是内存的索引的位掩码，第二个是内存的属性
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("failed to find a suitable memory type");
}

VkCommandBuffer Utils::beginSingleTimeCommands(const VK::Device &device, const VkCommandPool &commandPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device.vkDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    //命令缓冲区被使用一次后被丢弃

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Utils::endSingleTimeCommands(const VK::Device &device, const VkCommandPool &commandPool,
                                  VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device.graphicsQueue);
    vkFreeCommandBuffers(device.vkDevice, commandPool, 1, &commandBuffer);
}

bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Utils::transitionImageLayout(const VK::Device &device, const VkCommandPool &commandPool,
                                  VkImage image, VkFormat format, VkImageLayout oldLayout,
                                  VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);
    VkImageMemoryBarrier barrier{};
    //此处使用其进行对布局的转换

    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;


    barrier.image = image;
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    barrier.subresourceRange.baseMipLevel = 0;
    //图像不是数组，也没有mipmap的级别，因此只指定了一个级别和图层
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.layerCount = layerCount;
    //不需要等待任何之前的操作完成即可执行后续操作。//前一个阶段如何被访问,accessMask 指定了某个操作对资源的具体访问类型（如读取、写入等）
    //表示在目标阶段（destination stage）不需要任何特定类型的访问。
    VkPipelineStageFlags sourceStage, destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (
        oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    ) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        //早期片段测试
    } else if (
        oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_GENERAL
    ) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT; //为了CUBEMAP
    } else if (
        oldLayout == VK_IMAGE_LAYOUT_GENERAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    ) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; //为了CUBEMAP
    } else if (
        oldLayout == VK_IMAGE_LAYOUT_GENERAL &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    ) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT; //为了CUBEMAP
    } else {
        throw std::runtime_error("Unsupported layout transition");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage, //哪个GPU管线阶段执行屏障,哪一个阶段进行等待。这两个指定在非图形管线是没有意义的
        0, //指定屏障的依赖类型,这里是全局依赖，需要所有资源都被同步
        0, nullptr, //全局内存屏障， 这里如此不会影响主机和内存之间的内存访问
        0, nullptr, //缓冲区内存屏障，表明不涉及对缓冲区的数据并发访问
        1, &barrier //屏障的数量和指针
    );

    Utils::endSingleTimeCommands(device, commandPool, commandBuffer);
}

void Utils::checkVkResult(VkResult result) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error("VkResult error");
    }
}

//获取合适的MSAA采样数量
VkSampleCountFlagBits Utils::getMaxUsableSampleCount(const VkPhysicalDevice &physicalDevice) {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.
                                limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

void Utils::generateMipmaps(VK::Instances::Image image, const VK::Device &device, VkCommandPool commandPool,
                            VkImageLayout imageLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image.image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = image.arrayNum;
    barrier.subresourceRange.levelCount = 1;


    int32_t mipWidth = image.width;
    int32_t mipHeight = image.height;

    for (auto i = 1; i < image.mipmapLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = imageLayout;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        if (imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &barrier);
        } else {
            throw std::runtime_error("Unsupported layout transition");
        }

        VkImageBlit blit = {};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1}; //源mipmap的尺寸
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = image.arrayNum;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = image.arrayNum;

        vkCmdBlitImage(commandBuffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                             nullptr, 0, nullptr, 1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }
    barrier.subresourceRange.baseMipLevel = image.mipmapLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);

    endSingleTimeCommands(device, commandPool, commandBuffer);
}


void Utils::LoadSingleImage(VK::Instances::Image &image, const std::string &PATH, const VK::Device &device,
                            VkCommandPool commandPool, uint32_t mipmapLevel) {
    //不支持加载复合图片
    cv::Mat cvImage = cv::imread(PATH);
    if(cvImage.channels() == 4)
        cvtColor(cvImage, cvImage, cv::COLOR_BGRA2RGBA);
    else if(cvImage.channels() == 3)
        cvtColor(cvImage, cvImage, cv::COLOR_BGR2RGBA);

    if (cvImage.empty()) {
        throw std::runtime_error("Failed to load image");
    }
    if (cvImage.channels() == 4) {
        VkDeviceSize imageSize = cvImage.cols * cvImage.rows * 4;
        auto *imageData = reinterpret_cast<uint8_t *>(cvImage.data);

        VK::Instances::Buffer stagingBuffer;
        stagingBuffer.createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.Map(); // 映射内存
        memcpy(stagingBuffer.data, imageData, static_cast<size_t>(imageSize)); // 复制图像数据
        stagingBuffer.UnMap(); // 取消映射

        image.createImage(device, cvImage.cols, cvImage.rows, mipmapLevel, 1,
                          VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                          VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        Utils::transitionImageLayout(device, commandPool, image.image,
                                     VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1); // 转换图像布局为传输目标
        image.copy(stagingBuffer.buffer, commandPool); // 复制数据到图像
        if (mipmapLevel > 1) {
            Utils::generateMipmaps(image, device, commandPool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            image.imageView = Utils::createImageView(device.vkDevice, image.image,
                                                     VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipmapLevel,
                                                     1); // 创建图像视图
        } else {
            Utils::transitionImageLayout(device, commandPool, image.image,
                                         VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1); // 转换图像布局为着色器可读
            image.imageView = Utils::createImageView(device.vkDevice, image.image,
                                                     VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1); // 创建图像视图
        }
        stagingBuffer.destroyBuffer(); // 销毁暂存缓冲区
    }
}
