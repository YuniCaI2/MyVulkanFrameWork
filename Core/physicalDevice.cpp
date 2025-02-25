//
// Created by 51092 on 25-2-24.
//

#include "physicalDevice.h"

void VK::PhysicalDevice::pickPhysicalDevice(const vk::Instance &instance) {
    uint32_t deviceCount = 0;
    std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
    deviceCount = static_cast<uint32_t>(physicalDevices.size());
    if (deviceCount == 0) {
        throw std::runtime_error("failed to enumerate physical devices");
    }
    for (const auto &device : physicalDevices) {
        if (isDeviceSuitable(device)) {
            m_physicalDevice = device;
            break;
        }
    }
    if (m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to create physical device");
        exit(-1);
    }
}

VK::PhysicalDevice VK::PhysicalDevice::setDeviceExtension(const char* extension) {
    m_deviceExtensions.push_back(extension);
    return *this;
}

bool VK::PhysicalDevice::isDeviceSuitable(const vk::PhysicalDevice &physicalDevice) {
    vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevice.getProperties();
    vk::PhysicalDeviceFeatures physicalDeviceFeatures = physicalDevice.getFeatures();

    return physicalDeviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
        physicalDeviceFeatures.geometryShader;
}

vk::PhysicalDevice VK::PhysicalDevice::Device() const {
    return m_physicalDevice;
}

VK::PhysicalDevice::PhysicalDevice(const vk::Instance &instance) {
    pickPhysicalDevice(instance);
}
