//
// Created by 51092 on 25-2-27.
//

#ifndef DESCRIPTORMANAGER_H
#define DESCRIPTORMANAGER_H
#include <vector>
#include <vulkan/vulkan_core.h>

namespace VK::Instances {
    class DescriptorManager {
    public:
        void initialManager(VkDevice device);
        void setUniformBuffer(const VkBuffer& buffer);
        void setSampler(const VkSampler& sampler);
        void setImageView(const VkImageView& imageView);
        void setMaxSets(const uint32_t& maxSets);
        void createSets();

        VkDevice device{};
        VkDescriptorPool descriptorPool{};
        std::vector<VkDescriptorSet> uniformDescriptorSets{};
        VkDescriptorSetLayout uniformDescriptorSetLayout{};
        VkDescriptorSetLayout textureDescriptorSetLayout{};
        std::vector<VkDescriptorSet> textureDescriptorSets{};
        std::vector<VkBuffer> uniformBuffers{};
        std::vector <VkImageView> textureImageViews{};
        VkSampler textureSampler{};
        uint32_t maxSets{};
    private:
        void createSetLayouts();
        void createPool();

    };
}



#endif //DESCRIPTORMANAGER_H
