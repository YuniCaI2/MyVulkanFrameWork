//
// Created by 51092 on 25-3-6.
//

#include "Sample.h"

void VK::Instances::Sample::createSampler(const VK::Device &device) {
    this->device = device.device;
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;//类似于OpenGL
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;//超出图像时重复纹理
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    //上面指的是是否使用各向异性，这里选择是
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device.physicalDevice, &properties);
    samplerCreateInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    //限制各向异性所支持的纹理的样本数量
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    //使用[0,1)的纹理坐标的范围进行采样
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    //比较结果永远返回TRUE
    samplerCreateInfo.compareEnable = VK_FALSE;
    //禁用比较

    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 1000.0f;
    //这里上面的四个设置适用于mipmap
    if (vkCreateSampler(device.device, &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

}

void VK::Instances::Sample::destroySampler(){
    vkDestroySampler(device, sampler, nullptr);
}
