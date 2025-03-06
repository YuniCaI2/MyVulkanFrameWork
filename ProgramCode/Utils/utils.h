//
// Created by 51092 on 25-2-26.
//

#ifndef UTILS_H
#define UTILS_H
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace Utils {
    VkImageView createImageView(VkDevice device,VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
     uint32_t mipLevels);
    std::vector<char> readFile(const std::string& filename);
    VkFormat findSupportedFormat(const VkPhysicalDevice& physicalDevice,const std::vector<VkFormat>& candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat(const VkPhysicalDevice& physicalDevice);
    uint32_t findMemoryType(const VkPhysicalDevice& physicalDevice,uint32_t typeFilter, VkMemoryPropertyFlags properties);
}
#endif //UTILS_H
