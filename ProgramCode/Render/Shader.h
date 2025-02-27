//
// Created by 51092 on 25-2-27.
//

#ifndef SHADER_H
#define SHADER_H
#include "Pipeline.h"
#include <string>

#include "../Core/Device.h"

namespace VK::Render {
    class Shader {
    public:
        Shader(VkDevice device, const std::string& filepath, VkShaderStageFlagBits stage);
        VkPipelineShaderStageCreateInfo getPipelineShaderStageCreateInfo();
        void DestroyShaderModule() const;

    private:
        VkDevice device{};
        VkShaderModule shaderModule{};
        VkShaderStageFlagBits stage{};
        VkShaderModule createShaderModule(const std::string& codeFilePath) const;
    };
}


#endif //SHADER_H
