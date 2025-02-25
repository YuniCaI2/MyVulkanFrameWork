//
// Created by 51092 on 25-2-23.
//

#ifndef DEVICE_H
#define DEVICE_H
#include <vulkan/vulkan_raii.hpp>

#if defined(_WIN32) || defined(_WIN64)
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
#elif defined(__APPLE__) && defined(__MACH__)
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    "VK_KHR_portability_subset"
};
#endif

namespace VK {
    class Device {
    public:

    };

}


#endif //DEVICE_H
