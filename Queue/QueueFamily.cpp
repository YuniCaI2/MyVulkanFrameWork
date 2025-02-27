#include "QueueFamily.h"
#include <stdexcept>
#include <vector>

namespace VK {

void QueueFamily::createQueueFamily(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    queryQueueFamilies(physicalDevice, surface);
}

bool QueueFamily::isComplete() const
{
    return graphicsFamily.has_value() &&
           presentFamily.has_value() &&
           computeFamily.has_value() &&
           transferFamily.has_value();
}

bool QueueFamily::hasDedicatedTransfer() const
{
    return transferFamily.has_value() &&
           transferFamily != graphicsFamily &&
           transferFamily != computeFamily;
}

void QueueFamily::queryQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    // 第一遍寻找专用队列
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        const auto& queueFamily = queueFamilies[i];

        // Graphics Queue
        if (!graphicsFamily.has_value() &&
            (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            graphicsFamily = i;
        }

        // Compute Queue (专用)
        if (!computeFamily.has_value() &&
            (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
            !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            computeFamily = i;
        }

        // Transfer Queue (专用)
        if (!transferFamily.has_value() &&
            (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
            !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            !(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            transferFamily = i;
        }
    }

    // 第二遍寻找普通队列（回退策略）
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        const auto& queueFamily = queueFamilies[i];

        // Present Queue（需要Surface支持）
        if (surface != VK_NULL_HANDLE && !presentFamily.has_value()) {
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            if (presentSupport) {
                presentFamily = i;
            }
        }

        // 计算队列回退
        if (!computeFamily.has_value() &&
            (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            computeFamily = i;
        }

        // 传输队列回退
        if (!transferFamily.has_value() &&
            (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)) {
            transferFamily = i;
        }
    }

    // 最终回退
    if (!computeFamily.has_value()) computeFamily = graphicsFamily;
    if (!transferFamily.has_value()) transferFamily = graphicsFamily;
}

} // namespace VK
