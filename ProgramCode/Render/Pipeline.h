//
// Created by cai on 25-2-27.
//

#ifndef PIPELINE_H
#define PIPELINE_H
#include <vector>
#include <vulkan/vulkan_core.h>
#include <string>
#include "./Shader.h"
#include "../Presentation/SwapChain.h"

enum class ShaderStage {
    VERT,
    FRAG,
};

namespace VK::Render {
        class Pipeline {
        public:
            VkPipeline m_pipeline;
            void createPipeline(VkDevice device, const SwapChain& swapChain);
            Pipeline setShader(const std::string& path, ShaderStage stage);
        private:
            VkDevice device{};
            VK::SwapChain swapChain{};
            std::vector<Shader> shaders;
            std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
            VkPipelineDynamicStateCreateInfo dynamicState{};
            VkPipelineVertexInputStateCreateInfo vertexInput{};

            static VkPipelineDynamicStateCreateInfo setDynamicState();
        };
    }



#endif //PIPELINE_H
