//
// Created by 51092 on 25-3-4.
//

#include "FrameBuffer.h"
#include <vector>


void VK::Render::FrameBuffer::createFrameBuffers(const VK::Device &device, const RenderPass &renderPass,const VK::SwapChain &swapChain,
    const std::vector<VkImageView> &attachments) {
    m_Device = device.vkDevice;
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass =renderPass.m_renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swapChain.extent.width;
    framebufferInfo.height = swapChain.extent.height;
    framebufferInfo.layers = 1;
    if(vkCreateFramebuffer(device.vkDevice, &framebufferInfo, nullptr, &Buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer");
    }

}

void VK::Render::FrameBuffer::destroyFrameBuffers() const{
    vkDestroyFramebuffer(m_Device, Buffer, nullptr);
}
