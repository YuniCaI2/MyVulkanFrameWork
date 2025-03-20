//
// Created by 51092 on 25-3-18.
//

#include "CubeMap.h"
#include<opencv2/opencv.hpp>

void VK::Instances::CubeMap::createCubeMap(const VK::Device &device, const VkCommandPool &commandPool,
                                           const VkSampler &sampler,
                                           const std::string &cubeMapPath) {
    m_Device = device.vkDevice;
    LoadEXRRawImage(device, commandPool, cubeMapPath);
    LoadShader("../ProgramCode/Shaders/spv/computerForCubeMap.spv");
    createDescriptor(device, sampler);
    LoadPipeline(device, commandPool);

    VkCommandBuffer commandBuffer = Utils::beginSingleTimeCommands(device, commandPool);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout,
        0, 1,&descriptorSet, 0, nullptr);
    uint32_t groupCountX = 1024 / 32;
    uint32_t groupCountY = 1024 / 32;
    uint32_t groupCountZ = 6; // 立方体贴图的 6 个面
    vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    Utils::endSingleTimeCommands(device, commandPool, commandBuffer);
    Utils::transitionImageLayout(device, commandPool, this->image.image, this->image.format,
                             VK_IMAGE_LAYOUT_GENERAL,
                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 6);
}

void VK::Instances::CubeMap::Destroy() const {
    vkDestroyPipeline(m_Device, computePipeline, nullptr);
    vkDestroyPipelineLayout(m_Device, computePipelineLayout, nullptr);
    rawImage.destroyImage();
    vkDestroyDescriptorSetLayout(m_Device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(m_Device, descriptorPool, nullptr);
    image.destroyImage();
}


void VK::Instances::CubeMap::LoadEXRRawImage(const VK::Device &device, const VkCommandPool &commandPool,
                                             const std::string &Path) {
    Buffer rawImageData{};
    cv::Mat image = cv::imread(Path, cv::IMREAD_UNCHANGED);
    std::cout << "Image depth:" << image.depth() << std::endl;
    // cv::imshow(Path, image);
    if (image.empty()) {
        throw std::runtime_error("无法加载纹理图像: " + Path); // 加载失败抛出异常
    }

    if (image.depth() == CV_16U || image.depth() == CV_16F) {
        auto imageData = reinterpret_cast<uint16_t *>(image.data);
        VkDeviceSize imageSize = image.rows * image.cols * 8;
        rawImageData.createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        rawImageData.Map();
        memcpy(rawImageData.data, imageData, static_cast<size_t>(imageSize));

        rawImage.createImage(device, image.cols, image.rows, 1, 1,
                             VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        Utils::transitionImageLayout(device, commandPool, rawImage.image, VK_FORMAT_R16G16B16A16_SFLOAT,
                             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1);
        rawImage.copy(rawImageData.buffer, commandPool);
        rawImage.imageView = Utils::createImageView(device.vkDevice, rawImage.image, VK_FORMAT_R16G16B16A16_SFLOAT,
                                                    VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);
        Utils::transitionImageLayout(device, commandPool, rawImage.image, VK_FORMAT_R16G16B16A16_SFLOAT,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);
        this->image.createImage(device, 1024, 1024, 1, 6,
                                VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        this->image.imageView = Utils::createImageView(device.vkDevice, this->image.image,
                                                       VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 1, 6);
        Utils::transitionImageLayout(device, commandPool, this->image.image, VK_FORMAT_R16G16B16A16_SFLOAT,
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_GENERAL, 1, 6);
    }
    if (image.depth() == CV_32F) {
        auto imageData = reinterpret_cast<uint32_t *>(image.data);
        VkDeviceSize imageSize = image.rows * image.cols * 16;
        rawImageData.createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        rawImageData.Map();
        memcpy(rawImageData.data, imageData, static_cast<size_t>(imageSize));

        rawImage.createImage(device, image.cols, image.rows, 1, 1,
                             VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        Utils::transitionImageLayout(device, commandPool, rawImage.image, VK_FORMAT_R32G32B32A32_SFLOAT,
                             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1);
        rawImage.copy(rawImageData.buffer, commandPool);
        rawImage.imageView = Utils::createImageView(device.vkDevice, rawImage.image, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                    VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);
        Utils::transitionImageLayout(device, commandPool, rawImage.image, VK_FORMAT_R32G32B32A32_SFLOAT,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);
        this->image.createImage(device, 1024, 1024, 1, 6,
                                VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        this->image.imageView = Utils::createImageView(device.vkDevice, this->image.image,
                                                       VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 1, 6);
        Utils::transitionImageLayout(device, commandPool, this->image.image, VK_FORMAT_R32G32B32A32_SFLOAT,
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_GENERAL, 1, 6);
    }
    rawImageData.destroyBuffer();
}

void VK::Instances::CubeMap::createDescriptor(const VK::Device &device, const VkSampler &sampler) {
    VkDescriptorSetLayoutBinding bindings[2] = {};
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(device.vkDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout for CubeMap!");
    }
    std::vector< VkDescriptorPoolSize> poolSizes;
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    VkDescriptorPoolSize descriptorPoolSize{};
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSize.descriptorCount = 1;
    poolSizes.push_back(descriptorPoolSize);
    descriptorPoolSize.descriptorCount = 1;
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes.push_back(descriptorPoolSize);

    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;
    if (vkCreateDescriptorPool(device.vkDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool for CubeMap!");
    }
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

    if (vkAllocateDescriptorSets(device.vkDevice, &descriptorSetAllocateInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets for CubeMap!");
    }
    VkDescriptorImageInfo descriptorRawImageInfo{};
    descriptorRawImageInfo.sampler = sampler;
    descriptorRawImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptorRawImageInfo.imageView = rawImage.imageView;

    VkDescriptorImageInfo descriptorImageInfo{};
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    descriptorImageInfo.imageView = image.imageView;

    VkWriteDescriptorSet writes[2] = {};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = descriptorSet;
    writes[0].dstBinding = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[0].pImageInfo = &descriptorRawImageInfo;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = descriptorSet;
    writes[1].dstBinding = 1;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writes[1].pImageInfo = &descriptorImageInfo;

    vkUpdateDescriptorSets(device.vkDevice, 2, writes, 0, nullptr);

    // 创建布局、池和描述符集（省略细节）
}

void VK::Instances::CubeMap::LoadShader(
    const std::string &Path) {
    VK::Render::Shader shader{m_Device, Path, VK_SHADER_STAGE_COMPUTE_BIT};
    computeShader = shader;
}

void VK::Instances::CubeMap::LoadPipeline(const VK::Device &device, const VkCommandPool &commandPool) {

    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descriptorSetLayout;
    layoutInfo.pushConstantRangeCount = 0;

    vkCreatePipelineLayout(device.vkDevice, &layoutInfo, nullptr, &computePipelineLayout);

    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = computeShader.getPipelineShaderStageCreateInfo();
    pipelineInfo.layout = computePipelineLayout;
    vkCreateComputePipelines(device.vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline);
    computeShader.DestroyShaderModule();
}
