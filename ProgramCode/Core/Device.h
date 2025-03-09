#ifndef DEVICE_H
#define DEVICE_H

#include <map>
#include <vulkan/vulkan_raii.hpp>
#include <optional>
#include "../Queue/QueueFamily.h"    // 确保 QueueFamily 在 VK 命名空间
#include "../Core/physicalDevice.h"  // 确保 PhysicalDevice 在 VK 命名空间



namespace VK {
    class Device {
    public:
        VkDevice vkDevice{};
        VkQueue graphicsQueue{};
        VkQueue presentQueue{};
        VkQueue transferQueue{};
        VkQueue computeQueue{};
        uint32_t graphicsQueueFamilyIndex{};
        uint32_t presentQueueFamilyIndex{};
        uint32_t transferQueueFamilyIndex{};
        uint32_t computeQueueFamilyIndex{};

        VkPhysicalDevice physicalDevice{};

        std::map<uint32_t, std::vector<VkQueue>> queueFamiliesMap;

        void Destroy();

        void createDevice(const QueueFamily& queueFamily, const PhysicalDevice& physicalDevice, bool enableValidationLayer);
        static bool checkDeviceExtensionSupport(const VkPhysicalDevice& physicalDevice);

    private:
        std::vector<uint32_t> queueFamilies;
        void loadQueueFamilyIndices(const QueueFamily& queueFamily);
    };
}

#endif // DEVICE_H
