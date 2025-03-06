//
// Created by cai on 25-2-27.
//

#include "Pipeline.h"
#include "../Utils/utils.h"
#include "../Instance/Vertex.h"

std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

VK::Render::Pipeline & VK::Render::Pipeline::initial(const VkDevice& device) {
    this->device = device;
    return *this;
}

void VK::Render::Pipeline::createPipeline( const SwapChain& swapChain,
                                          const Instances::DescriptorManager& descriptorManager, const VkRenderPass& renderPass) {
    this->swapChain = swapChain;
    VkPipelineDynamicStateCreateInfo dynamicState = setDynamicState();
    //设置顶点布局
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescription = Vertex::getAttributeDescriptions();

    //顶点输入 Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1; // 表示顶点绑定描述符的数量
    //  因为顶点被硬编码在顶点着色器的文件中了，所以此处不需要加载顶点数据
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional 是一个指向顶点绑定描述符数组的指针
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size()); // 表示顶点属性描述符的数量(可以对应openGL中的顶点布局)
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data(); // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    //这里标注绘制的是三角形，每三个顶点一个三角形，且不能重复
    inputAssembly.primitiveRestartEnable = VK_FALSE;//这里指的是是否使用原始重启，这里是FALSE

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain.extent.width);
    viewport.height = static_cast<float>(swapChain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    //需要保证大于视口
    scissor.offset = {0, 0};
    scissor.extent = swapChain.extent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {descriptorManager.uniformDescriptorSetLayout,
        descriptorManager.textureDescriptorSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pushConstantRangeCount = 0;//先不使用常量推送
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    // pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;


    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizerInfo;
    pipelineInfo.pMultisampleState = &multisamplingInfo;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.pColorBlendState = &colorBlendStateCreateInfo;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;//没有派生管线

    if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }



    for (const auto& shader : shaders) {
        shader.DestroyShaderModule();
    }
}

VK::Render::Pipeline& VK::Render::Pipeline::setShader(const std::string& path, ShaderStage stage) {
    if (stage == ShaderStage::VERT) {
        Shader shader{device, path, VK_SHADER_STAGE_VERTEX_BIT};
        shaders.emplace_back(shader);
        shaderStages.push_back(shader.getPipelineShaderStageCreateInfo());
        return *this;
    }
    else if (stage == ShaderStage::FRAG) {
        Shader shader{device, path, VK_SHADER_STAGE_FRAGMENT_BIT};
        shaders.emplace_back(shader);
        shaderStages.push_back(shader.getPipelineShaderStageCreateInfo());
        return *this;
    }
    else {
        throw std::runtime_error("Invalid shader stage");
    }
}

VK::Render::Pipeline& VK::Render::Pipeline::setRasterizerState(const VkBool32 &rasterizerDiscardEnable,
    const VkPolygonMode &polygonMode, const VkCullModeFlags &cullMode, const VkFrontFace &frontFace,
    const float &lineWidth, const VkBool32 &depthBiasEnable, const float &depthBiasConstantFactor,
    const float &depthBiasClamp, const float &depthBiasSlopeFactor, const VkBool32 &depthClampEnable) {
    rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerInfo.depthClampEnable = depthClampEnable;
    rasterizerInfo.rasterizerDiscardEnable = rasterizerDiscardEnable;
    rasterizerInfo.polygonMode = polygonMode;
    rasterizerInfo.cullMode = cullMode;
    rasterizerInfo.frontFace = frontFace;
    rasterizerInfo.depthBiasEnable = depthBiasEnable;
    rasterizerInfo.depthBiasConstantFactor = depthBiasConstantFactor;
    rasterizerInfo.depthBiasClamp = depthBiasClamp;
    rasterizerInfo.depthBiasSlopeFactor = depthBiasSlopeFactor;
    rasterizerInfo.lineWidth = lineWidth;
    return *this;
}

VK::Render::Pipeline& VK::Render::Pipeline::setMultisampleState(const VkSampleCountFlagBits &sampleCount,
    const VkBool32 &sampleShadingEnable, const float &minSampleShading, const VkSampleMask *pSamplerMask,
    const VkBool32 &alphaToCoverageEnable, const VkBool32 &alphaToOneEnable) {
    multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingInfo.sampleShadingEnable = sampleShadingEnable;
    multisamplingInfo.rasterizationSamples = sampleCount;
    multisamplingInfo.alphaToCoverageEnable = alphaToCoverageEnable;
    multisamplingInfo.alphaToOneEnable = alphaToOneEnable;
    multisamplingInfo.pSampleMask = pSamplerMask;
    multisamplingInfo.minSampleShading = minSampleShading;
    return *this;
}

VK::Render::Pipeline& VK::Render::Pipeline::setDepthStencilState() {
    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
    //这里设置更近的保留
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.minDepthBounds = 0.0f;
    depthStencilState.maxDepthBounds = 1.0f;
    //深度范围
    depthStencilState.stencilTestEnable = VK_FALSE;
    //不使用模版缓冲
    depthStencilState.front = {};
    depthStencilState.back = {};
    //关于模版缓冲的一些设置

    depthStencilInfo = depthStencilState;

    return *this;


}

VK::Render::Pipeline& VK::Render::Pipeline::setColorBlendState() {
    //2025.02.28 中此处设置默认不调用
    //颜色混合
    VkPipelineColorBlendAttachmentState  colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    //这里设置保留所有分量
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentInfo = colorBlendAttachment;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachmentInfo;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    colorBlendStateCreateInfo = colorBlending;
    return *this;


}

void VK::Render::Pipeline::Destroy() {
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
    vkDestroyPipeline(device, m_pipeline, nullptr);
}

VkPipelineDynamicStateCreateInfo VK::Render::Pipeline::setDynamicState(){
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    return dynamicState;
    //动态设置
}

