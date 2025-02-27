#ifndef QUEUEFAMILY_H
#define QUEUEFAMILY_H

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>



namespace VK {

    class QueueFamily {
    public:
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> transferFamily;

         void createQueueFamily(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface = VK_NULL_HANDLE);

        // 检查必要队列是否存在
        bool isComplete() const;

        // 辅助函数检查专用传输队列
        bool hasDedicatedTransfer() const;

    private:
        void queryQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    };

} // namespace VK

#endif // QUEUEFAMILY_H
