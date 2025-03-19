//
// Created by 51092 on 25-3-18.
//

#ifndef CUBEMAP_H
#define CUBEMAP_H
#include "Image.h"
#include "Buffer.h"
#include "../Render/Shader.h"
namespace VK::Instances {
    class CubeMap {
    public:
        void createCubeMap(const VK::Device& device, const VkCommandPool& commandPool, const VkSampler& sampler,const std::string& cubeMapPath);
        void Destroy() const;
        Image image{};

    private:
        Image rawImage{};
        VkDevice m_Device{};
        VkDescriptorPool descriptorPool{};
        VkDescriptorSetLayout descriptorSetLayout{};
        VkDescriptorSet descriptorSet{};
        VK::Render::Shader computeShader;
        VkPipeline computePipeline{};
        VkPipelineLayout computePipelineLayout{};
        void LoadEXRRawImage(const VK::Device& device, const VkCommandPool& commandPool, const std::string& Path);
        void createDescriptor(const VK::Device& device,  const VkSampler& sampler);
        void LoadShader(const std::string& Path);
        void LoadPipeline(const VK::Device& device, const VkCommandPool& commandPool);
    };
}



#endif //CUBEMAP_H
