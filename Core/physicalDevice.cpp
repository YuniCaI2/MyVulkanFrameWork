//
// Created by 51092 on 25-2-24.
//

#include "physicalDevice.h"

#include <iostream>
#include <vector>
#include <stdexcept>

void VK::PhysicalDevice::pickPhysicalDevice(const VkInstance& instance) {
    uint32_t deviceCount = 0;
    // 第一次调用获取设备数量
    if (vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr) != VK_SUCCESS || deviceCount == 0) {
        throw std::runtime_error("Failed to enumerate physical devices");
    }

    // 第二次调用获取设备列表
    std::vector<VkPhysicalDevice> devices(deviceCount);
    if (vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to get physical devices");
    }

    // 寻找合适设备
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            m_physicalDevice = device;
            return;
        }
    }

    throw std::runtime_error("Failed to find a suitable GPU");
}

VK::PhysicalDevice VK::PhysicalDevice::setDeviceExtension(const char* extension) {
    m_deviceExtensions.push_back(extension);
    return *this;
}

bool VK::PhysicalDevice::isDeviceSuitable(const VkPhysicalDevice& physicalDevice) {
    // 获取设备属性
    VkPhysicalDeviceProperties deviceProperties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    // 获取设备特性
    VkPhysicalDeviceFeatures deviceFeatures{};
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    // 检查设备类型和几何着色器支持
    return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) &&
           (deviceFeatures.geometryShader);
}

VkPhysicalDevice VK::PhysicalDevice::Device() const {
    std::cout << m_physicalDevice;
    return m_physicalDevice;
}

void VK::PhysicalDevice::createPhysicalDevice(const VkInstance& instance)
{
    pickPhysicalDevice(instance);
}



