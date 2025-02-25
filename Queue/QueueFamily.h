//
// Created by 51092 on 25-2-24.
//

#ifndef QUEUEFAMILY_H
#define QUEUEFAMILY_H
#include <optional>

#include "../Core/physicalDevice.h"

namespace VK {
    class QueueFamily {
    public:
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> transferFamily;
        //队列家族

    private:
        uint32_t findQueueFamilies(VK::PhysicalDevice &physicalDevice);
    };
}



#endif //QUEUEFAMILY_H
