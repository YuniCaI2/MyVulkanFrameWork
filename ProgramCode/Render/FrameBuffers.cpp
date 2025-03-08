//
// Created by 51092 on 25-3-4.
//

#include "FrameBuffers.h"

#include <iostream>
#include <ostream>
#include <vector>

void VK::Render::FrameBuffers::createFrameBuffers(const VK::Device &device, const RenderPass &renderPass,
    const VK::SwapChain &swapChain, const std::vector<VkImageView>& depthBuffer) {
    m_Device = device.device;
    frameBuffers.resize(swapChain.swapChainImageViews.size());

    if (depthBuffer.size() != swapChain.swapChainImageViews.size()) {
        std::cout << "Depth buffer size does not match swap chain size" << std::endl;
    }

    for (uint32_t i = 0; i < swapChain.swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapChain.swapChainImageViews[i],
            depthBuffer[i]
        };
        // VkFramebufferCreateInfo framebufferInfo = {
        //     .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        //     .renderPass = renderPass.m_renderPass,
        //     .height = swapChain.extent.height,
        //     .width = swapChain.extent.width,
        //     .layers = 1,
        //     .attachmentCount = static_cast<uint32_t>(frameBuffers.size()),
        //     .pAttachments = attachments,
        // };
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass =renderPass.m_renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(2);
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChain.extent.width;
        framebufferInfo.height = swapChain.extent.height;
        framebufferInfo.layers = 1;
        if(vkCreateFramebuffer(device.device, &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer");
        }
    }
}

void VK::Render::FrameBuffers::destroyFrameBuffers() const{
    for (auto frameBuffer : frameBuffers) {
        vkDestroyFramebuffer(m_Device, frameBuffer, nullptr);
    }
}
