//
// Created by cai on 25-2-27.
//

#ifndef PIPELINE_H
#define PIPELINE_H
#include <vector>
#include <vulkan/vulkan_core.h>
#include "./Shader.h"
#include "../Instance/DescriptorManager.h"
#include "../Presentation/SwapChain.h"

enum class ShaderStage {
    VERT,
    FRAG,
};

namespace VK::Render {
        class Pipeline {
        public:
            VkPipelineLayout pipelineLayout{};

            Pipeline& initial(const VkDevice& device);
            VkPipeline m_pipeline{};
            void createPipeline( const SwapChain& swapChain,
                const VkRenderPass& renderPass);
            Pipeline& setShader(const std::string& path, ShaderStage stage);
            Pipeline& setRasterizerState(
                const VkBool32& rasterizerDiscardEnable = VK_FALSE,
                const VkPolygonMode& polygonMode = VK_POLYGON_MODE_FILL,
                const VkCullModeFlags& cullMode = VK_CULL_MODE_BACK_BIT,
                const VkFrontFace& frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,

                const float& lineWidth = 1.0f,
                const VkBool32& depthBiasEnable = VK_FALSE,
                const float& depthBiasConstantFactor = 0.0f,
                const float& depthBiasClamp = 0.0f,
                const float& depthBiasSlopeFactor = 0.0f,
                const VkBool32& depthClampEnable = VK_FALSE
                );
            Pipeline& setMultisampleState(
            const VkSampleCountFlagBits& sampleCount = VK_SAMPLE_COUNT_1_BIT,
            const VkBool32& sampleShadingEnable = VK_FALSE, const float& minSampleShading = 1.0f,
            const VkSampleMask* pSamplerMask = nullptr,
            const VkBool32& alphaToCoverageEnable = VK_FALSE,
            const VkBool32& alphaToOneEnable = VK_FALSE
            );
            Pipeline& createPipelineLayout(
            const std::vector<VkDescriptorSetLayout>& pSetLayouts,
            const VkShaderStageFlags& setShaderStages,
            const uint32_t& size
                );
            Pipeline& setDepthStencilState();
            Pipeline& setColorBlendState();
            void Destroy();
        private:
            VkDevice device{};
            VK::SwapChain swapChain{};
            std::vector<Shader> shaders;
            std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
            VkPipelineDynamicStateCreateInfo dynamicState{};
            VkPipelineVertexInputStateCreateInfo vertexInput{};

            VkPipelineRasterizationStateCreateInfo rasterizerInfo{};//光栅
            VkPipelineMultisampleStateCreateInfo multisamplingInfo{};//多重采样
            VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo{};
            VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
            VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};

            static VkPipelineDynamicStateCreateInfo setDynamicState();
        };
    }



#endif //PIPELINE_H
