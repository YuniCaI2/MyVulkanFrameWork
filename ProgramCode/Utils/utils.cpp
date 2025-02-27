//
// Created by 51092 on 25-2-26.
//

#include "utils.h"
#include <stdexcept>
#include <fstream>

VkImageView Utils::createImageView(VkDevice device,VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount =1;
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

std::vector<char> Utils::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
                    //第一个参数的含义是从文件末尾读取，第二个参数是将文件读取为二进制文件
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);//将光标移到首位
    file.read(buffer.data(), static_cast<uint32_t>(fileSize));
    file.close();
    return buffer;
}
