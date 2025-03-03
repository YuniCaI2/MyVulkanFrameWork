//
// Created by 51092 on 25-3-3.
//

#include "RenderPass.h"
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


void VK::Render::RenderPass::createRenderPass(const VkPhysicalDevice& physicalDevice,const VkDevice &device, const VkFormat &format) {
    this->device = device;
    this->swapchainImageFormat = format;
    this->physicalDevice = physicalDevice;
    setAttachmentDescription({
        .format = swapchainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
    });
    setAttachmentReference({
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    });
    setAttachmentDescription({
        .format = Utils::findDepthFormat(physicalDevice),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
    });
    setAttachmentReference({
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    });
    setSubpassDescription({
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentReferences[0],
        .pDepthStencilAttachment = &attachmentReferences[1]
    });
    setSubpassDependency({
       .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    });
}

void VK::Render::RenderPass::setSubpassDependency(const VkSubpassDependency &subpassDependency) {
    this->subpassDependencies.push_back(subpassDependency);
}
