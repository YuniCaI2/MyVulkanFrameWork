#ifndef DEVICE_H
#define DEVICE_H

#include <vulkan/vulkan_raii.hpp>
#include <optional>
#include "../Queue/QueueFamily.h"    // 确保 QueueFamily 在 VK 命名空间
#include "../Core/physicalDevice.h"  // 确保 PhysicalDevice 在 VK 命名空间



namespace VK {
    class Device {
    public:
        VkDevice device{};
        VkQueue graphicsQueue{};
        VkQueue presentQueue{};
        VkQueue transferQueue{};
        VkQueue computeQueue{};

        void Destroy();

        void createDevice(const QueueFamily& queueFamily, const PhysicalDevice& physicalDevice, bool enableValidationLayer);

    private:
        std::vector<uint32_t> queueFamilies;
        void loadQueueFamilyIndices(const QueueFamily& queueFamily);
    };
}

#endif // DEVICE_H
