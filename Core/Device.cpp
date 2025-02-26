//
// Created by 51092 on 25-2-23.
//

#include "Device.h"

#include "Instance.h"
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

void VK::Device::Destroy() {
    vkDestroyDevice(device, nullptr);
}

void VK::Device::createDevice(const VK::QueueFamily &queueFamily,
                   const VK::PhysicalDevice& physicalDevice,
                   bool enableValidationLayers) {
    loadQueueFamilyIndices(queueFamily);
    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.flags = 0;
    queueCreateInfo.queueFamilyIndex = queueFamilies[0];
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfos.push_back(queueCreateInfo);
    for (auto i = 1 ; i < queueFamilies.size(); i++) {
        if (queueFamilies[i] == queueFamilies[i -1]) {
            continue;
        } else {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.flags = 0;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.queueFamilyIndex = queueFamilies[i];
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfo.pNext = nullptr;
            queueCreateInfos.push_back(queueCreateInfo);
        }
    }
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    //创建逻辑设备的结构体
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.flags =0;
    deviceCreateInfo.pNext = nullptr;
    if (enableValidationLayers) {
        deviceCreateInfo.enabledLayerCount = 1;
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        deviceCreateInfo.enabledLayerCount = 0;
    }
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physicalDevice.m_physicalDevice,&deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    //create queue
    int queueIndexOffset = 0;
    vkGetDeviceQueue(device, queueFamilies[0], 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueFamilies[0], 1, &presentQueue);
    vkGetDeviceQueue(device, queueFamilies[2], 0, &transferQueue);
    vkGetDeviceQueue(device, queueFamilies[3], 0, &computeQueue);
}

void VK::Device::loadQueueFamilyIndices(const QueueFamily &queueFamily) {
    if (queueFamily.graphicsFamily.has_value()) {
        queueFamilies.push_back(queueFamily.graphicsFamily.value());
    }
    if (queueFamily.presentFamily.has_value()) {
        queueFamilies.push_back(queueFamily.presentFamily.value());
    }
    if (queueFamily.transferFamily.has_value()) {
        queueFamilies.push_back(queueFamily.transferFamily.value());
    }
    if (queueFamily.computeFamily.has_value()) {
        queueFamilies.push_back(queueFamily.computeFamily.value());
    }
}

