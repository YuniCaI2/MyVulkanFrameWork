//
// Created by 51092 on 25-2-26.
//

#ifndef UTILS_H
#define UTILS_H
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "../Core/Device.h"

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

enum class RenderPassType {
    FORWARD,
    GUI
};

namespace Utils {
    VkImageView createImageView(VkDevice device,VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
     uint32_t mipLevels);
    std::vector<char> readFile(const std::string& filename);
    VkFormat findSupportedFormat(const VkPhysicalDevice& physicalDevice,const std::vector<VkFormat>& candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat(const VkPhysicalDevice& physicalDevice);
    uint32_t findMemoryType(const VkPhysicalDevice& physicalDevice,uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkCommandBuffer beginSingleTimeCommands(const VK::Device& device,const VkCommandPool& commandPool);
    void endSingleTimeCommands(const VK::Device& device,const VkCommandPool& commandPool,VkCommandBuffer commandBuffer);
    void transitionImageLayout(const VK::Device& device, const VkCommandPool& commandPool,VkImage image, VkFormat format, VkImageLayout oldLayout,
        VkImageLayout newLayout, uint32_t mipLevels);
    void checkVkResult(VkResult result);

}
#endif //UTILS_H
