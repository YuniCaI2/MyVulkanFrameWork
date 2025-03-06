//
// Created by 51092 on 25-2-27.
//

#include "DescriptorManager.h"
#include "UniformBufferObject.h"
#include <stdexcept>

void VK::Instances::DescriptorManager::initialManager(VkDevice device) {
    this->device = device;
    createSetLayouts();
    textureImageViews.clear();
    uniformBuffers.clear();
}

void VK::Instances::DescriptorManager::setUniformBuffer(const VkBuffer &buffer) {
    uniformBuffers.push_back(buffer);
}

void VK::Instances::DescriptorManager::setSampler(const VkSampler &sampler) {
    this->textureSampler = sampler;
}

void VK::Instances::DescriptorManager::setImageView(const VkImageView &imageView) {
    textureImageViews.push_back(imageView);
}

void VK::Instances::DescriptorManager::setMaxSets(const uint32_t &maxSets) {
    this->maxSets = maxSets;
}

void VK::Instances::DescriptorManager::createSets() {
    createPool();
    //uniformSets
    if (! uniformDescriptorSets.empty()) {
        std::vector<VkDescriptorSetLayout> setLayouts(uniformDescriptorSets.size(), uniformDescriptorSetLayout);
        VkDescriptorSetAllocateInfo uniformDescriptorSetAllocateInfo = {};
        uniformDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        uniformDescriptorSetAllocateInfo.descriptorPool = descriptorPool;
        uniformDescriptorSetAllocateInfo.descriptorSetCount = uniformDescriptorSets.size();
        uniformDescriptorSetAllocateInfo.pSetLayouts = setLayouts.data();
        if(vkAllocateDescriptorSets(device, &uniformDescriptorSetAllocateInfo, uniformDescriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate uniform descriptor sets!");
        }
    }
    //texture
    if (! textureDescriptorSets.empty()) {
        std::vector<VkDescriptorSetLayout> setLayouts(textureDescriptorSets.size(), textureDescriptorSetLayout);
        VkDescriptorSetAllocateInfo textureDescriptorSetAllocateInfo = {};
        textureDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        textureDescriptorSetAllocateInfo.descriptorPool = descriptorPool;
        textureDescriptorSetAllocateInfo.descriptorSetCount = textureDescriptorSets.size();
        textureDescriptorSetAllocateInfo.pSetLayouts = setLayouts.data();
        if(vkAllocateDescriptorSets(device, &textureDescriptorSetAllocateInfo, textureDescriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate texture descriptor sets!");
        }
    }

    for (auto i = 0; i < uniformDescriptorSets.size(); i++) {
        VkDescriptorBufferInfo uniformBufferInfo = {};
        uniformBufferInfo.buffer = uniformBuffers[i];
        uniformBufferInfo.offset = 0;
        uniformBufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet uniformBufferWriteInfo = {};
        uniformBufferWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uniformBufferWriteInfo.dstSet = uniformDescriptorSets[i];
        uniformBufferWriteInfo.dstBinding = 0;
        uniformBufferWriteInfo.descriptorCount = 1;
        uniformBufferWriteInfo.dstArrayElement = 0;
        uniformBufferWriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformBufferWriteInfo.pBufferInfo = &uniformBufferInfo;
        vkUpdateDescriptorSets(device, 1,
            &uniformBufferWriteInfo, 0, nullptr);
    }
    for (auto i = 0; i < textureDescriptorSets.size(); i++) {
        VkDescriptorImageInfo textureImageInfo = {};
        textureImageInfo.imageView = textureImageViews[i];
        textureImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        textureImageInfo.sampler =textureSampler;

        VkWriteDescriptorSet textureImageWriteInfo = {};
        textureImageWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        textureImageWriteInfo.dstSet = textureDescriptorSets[i];
        textureImageWriteInfo.dstBinding = 0;
        textureImageWriteInfo.descriptorCount = 1;
        textureImageWriteInfo.dstArrayElement = 0;
        textureImageWriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureImageWriteInfo.pImageInfo = &textureImageInfo;
        vkUpdateDescriptorSets(device, 1,
            &textureImageWriteInfo, 0, nullptr);
    }
}

void VK::Instances::DescriptorManager::createSetLayouts() {
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;//这里代表资源的数量
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerLayoutBinding;
    if(vkCreateDescriptorSetLayout(device, &layoutInfo,
        nullptr, &textureDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture descriptor set layout!");
    }

    VkDescriptorSetLayoutBinding uboLayoutBinding{}; //描述绑定信息
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;

    uboLayoutBinding.pImmutableSamplers = nullptr; // 与图像采样相关的描述符
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    //指的是这里应用顶点着色器的描述符号;

    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerLayoutBinding;
    if(vkCreateDescriptorSetLayout(device, &layoutInfo,
        nullptr, &uniformDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create uniform descriptor set layout!");
        }

}



void VK::Instances::DescriptorManager::createPool() {
    uniformDescriptorSets.resize(uniformBuffers.size());
    textureDescriptorSets.resize(textureImageViews.size());

    std::vector<VkDescriptorPoolSize> poolSizes = {};
    if (!uniformDescriptorSets.empty() && !textureDescriptorSets.empty()) {
        poolSizes.resize(2);
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(uniformDescriptorSets.size());
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(textureDescriptorSets.size());

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());//包含描述符类型的数量
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(maxSets);
        //确保每一createDescriptorSets帧都是不同的描述符集合
        if(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    } else if (uniformDescriptorSets.empty() && textureDescriptorSets.empty()) {
        throw std::runtime_error("NO descriptorSets!");
    } else {
        poolSizes.resize(1);
        if ( ! uniformDescriptorSets.empty()) {
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(uniformDescriptorSets.size());
        }
        if ( ! textureDescriptorSets.empty()) {
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(textureDescriptorSets.size());
        }
        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());//包含描述符类型的数量
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(maxSets);
        //确保每一createDescriptorSets帧都是不同的描述符集合
        if(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }
}


