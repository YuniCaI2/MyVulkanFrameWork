//
// Created by 51092 on 25-3-6.
//

#ifndef IMAGE_H
#define IMAGE_H
#include <vulkan/vulkan_core.h>

#include "../Core/Device.h"


namespace VK::Instances {
    class Image {
    public:
        VkImage image{};
        VkImageView imageView{};
        VkDeviceMemory imageMemory{};
        void* data{nullptr};
        void Map();
        void UnMap() const;
        void destroyImage() const;
        void copy(VkBuffer buffer, VkCommandPool commandPool) const;
        void createImage( const VK::Device& device,
uint32_t width,uint32_t height,
uint32_t mipLevels,
VkSampleCountFlagBits numSamples,
VkFormat format, VkImageTiling tiling,
VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
        uint32_t width{};
        uint32_t height{};
    private:
        VkDeviceSize size{};
        VK::Device device{};
    };
}



#endif //IMAGE_H
