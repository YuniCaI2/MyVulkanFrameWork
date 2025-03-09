//
// Created by 51092 on 25-3-3.
//

#include "RenderPass.h"

#include <stdexcept>

#include "../Utils/utils.h"

void VK::Render::RenderPass::setAttachmentDescription(const VkAttachmentDescription &attachmentDescription) {
    attachments.push_back(attachmentDescription);
}

void VK::Render::RenderPass::setSubpassDescription(const VkSubpassDescription &subpassDescription) {
    subpass.push_back(subpassDescription);
}

void VK::Render::RenderPass::setAttachmentReference(const VkAttachmentReference &attachmentReference) {
    attachmentReferences.push_back(attachmentReference);
}


void VK::Render::RenderPass::createRenderPass(const VkPhysicalDevice &physicalDevice, const VkDevice &device,
                                              const VkFormat &format, RenderPassType renderPassType) {
    this->device = device;
    this->swapchainImageFormat = format;
    this->physicalDevice = physicalDevice;
    if (renderPassType == RenderPassType::FORWARD) {
        setAttachmentDescription({
            .flags = 0, // 显式初始化可选字段
            .format = swapchainImageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // 正确顺序位置
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // 调整到正确位置
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        });

        setAttachmentReference({
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        });

        // 修正后的深度附件描述
        setAttachmentDescription({
            .format = Utils::findDepthFormat(physicalDevice),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // 调整到正确位置
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        });

        setAttachmentReference({
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        });

        // 子通道描述（保持原有正确顺序）
        setSubpassDescription({
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachmentReferences[0],
            .pDepthStencilAttachment = &attachmentReferences[1]
        });

        // 修正后的子通道依赖关系（严格按照VkSubpassDependency成员顺序）
        setSubpassDependency({
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // StageMask在前
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = 0 // 显式初始化可选字段
        });

        VkRenderPassCreateInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr, // 显式初始化可选字段
            .flags = 0, // 显式初始化可选字段
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = subpass.data(),
            .dependencyCount = static_cast<uint32_t>(subpassDependencies.size()),
            .pDependencies = subpassDependencies.data()
        };

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!");
        }
    }
    if (renderPassType == RenderPassType::GUI) {

        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = swapchainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        setAttachmentDescription(colorAttachment);

        setAttachmentReference({
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        });

        setSubpassDescription({
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachmentReferences[0],
        });

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        setSubpassDependency(dependency);

        VkRenderPassCreateInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr, // 显式初始化可选字段
            .flags = 0, // 显式初始化可选字段
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = subpass.data(),
            .dependencyCount = static_cast<uint32_t>(subpassDependencies.size()),
            .pDependencies = subpassDependencies.data()
        };

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!");
        }
    }
}

void VK::Render::RenderPass::setSubpassDependency(const VkSubpassDependency &subpassDependency) {
    this->subpassDependencies.push_back(subpassDependency);
}

void VK::Render::RenderPass::DestroyRenderPass() const {
    vkDestroyRenderPass(device, m_renderPass, nullptr);
}
