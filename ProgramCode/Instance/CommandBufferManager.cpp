//
// Created by 51092 on 25-3-4.
//

#include "CommandBufferManager.h"

#include <stdexcept>

void VK::Instances::CommandBufferManager::createCommandBuffers(const VK::Device &device,
    const VK::QueueFamily &queueFamily, uint32_t size) {
    this->device = device.device;
    createCommandPool(queueFamily);
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = commandPool,
        .commandBufferCount = size
    };
    if (vkAllocateCommandBuffers(device.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VK::Instances::CommandBufferManager::recordCommandBuffer(const VkCommandBuffer &commandBuffer,
    const VK::Render::RenderPass &renderPass,
    const VK::Render::Pipeline &graphicsPipeline,
    const VK::Render::FrameBuffers &frameBuffers,
    const VK::SwapChain &swapChain,
    const VK::Instances::DescriptorManager &descriptorManager,
    uint32_t imageIndex
    ) {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;//这个参数指定我们如何使用帧缓冲区
    beginInfo.pInheritanceInfo = nullptr; //这个参数与辅助缓冲区有关；
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    std::array<VkClearValue, 2> clearValues = {
    };
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .framebuffer =  frameBuffers.frameBuffers[imageIndex],
        .renderArea.offset = {0, 0},
        .renderArea.extent = swapChain.extent,
        .renderPass = renderPass.m_renderPass,
        .clearValueCount = 2,
        .pClearValues = clearValues.data()
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.m_pipeline);

    VkViewport viewport = {
        .height = static_cast<float>(swapChain.extent.width),
        .width = static_cast<float>(swapChain.extent.width),
        .x = 0.0f,
        .y = 0.0f,
        .maxDepth = 1.0f,
        .minDepth = 0.0f,
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .extent = swapChain.extent,
        .offset = {0, 0}
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

}

void VK::Instances::CommandBufferManager::destroyCommandBuffers() {
    vkDestroyCommandPool(device, commandPool, nullptr);
}

void VK::Instances::CommandBufferManager::createCommandPool(const VK::QueueFamily &queueFamily) {
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamily.graphicsFamily.value()
    };
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}
