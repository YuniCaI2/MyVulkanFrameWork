//
// Created by 51092 on 25-2-26.
//

#ifndef UTILS_H
#define UTILS_H
#include <vulkan/vulkan.h>
#include <vector>

namespace Utils {
    VkImageView createImageView(VkDevice device,VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
     uint32_t mipLevels);

    static std::vector<char> readFile(const std::string& filename);
}
#endif //UTILS_H
