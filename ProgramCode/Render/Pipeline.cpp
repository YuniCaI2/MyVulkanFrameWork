//
// Created by cai on 25-2-27.
//

#include "Pipeline.h"
#include "../Utils/utils.h"
#include "../Instance/Vertex.h"

void VK::Render::Pipeline::createPipeline(VkDevice device, const SwapChain& swapChain) {
    this->device = device;
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


    for (const auto& shader : shaders) {
        shader.DestroyShaderModule();
    }
}

VK::Render::Pipeline VK::Render::Pipeline::setShader(const std::string& path, ShaderStage stage) {
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

VkPipelineDynamicStateCreateInfo VK::Render::Pipeline::setDynamicState(){
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    return dynamicState;
    //动态设置
}

