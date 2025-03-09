//
// Created by 51092 on 25-3-6.
//

#ifndef SAMPLE_H
#define SAMPLE_H
#include <vulkan/vulkan_core.h>
#include "../Core/Device.h"

namespace VK::Instances {
    class Sampler {
    public:
        VkSampler sampler;
        void createSampler(const VK::Device& device);
        void destroySampler();
    private:
        VkDevice device{};
    };
}


#endif //SAMPLE_H
