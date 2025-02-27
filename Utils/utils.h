//
// Created by 51092 on 25-2-26.
//

#ifndef UTILS_H
#define UTILS_H
#include <vulkan/vulkan.h>

namespace Utils {
    VkImageView createImageView(VkDevice device,VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
     uint32_t mipLevels);
}
#endif //UTILS_H
