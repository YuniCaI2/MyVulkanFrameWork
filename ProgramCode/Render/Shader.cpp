//
// Created by 51092 on 25-2-27.
//

#include "Shader.h"

#include <iostream>
#include <ostream>

#include "../Utils/utils.h"

VK::Render::Shader::Shader(VkDevice device, const std::string &filepath, VkShaderStageFlagBits stage) {
    this->device = device;
    this->stage = stage;
    shaderModule = createShaderModule(filepath);

}

VkPipelineShaderStageCreateInfo VK::Render::Shader::getPipelineShaderStageCreateInfo() {
    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = stage;
    shaderStageInfo.module = shaderModule;
    shaderStageInfo.pName = "main";
    return shaderStageInfo;
}

void VK::Render::Shader::DestroyShaderModule() const{
    vkDestroyShaderModule(this->device, shaderModule, nullptr);
}


VkShaderModule VK::Render::Shader::createShaderModule(const std::string& codeFilePath) const{
    auto shaderCode = Utils::readFile(codeFilePath);
    std::cout << shaderCode.size() << std::endl;
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }
    return shaderModule;
}
