//
// Created by 51092 on 25-3-3.
//

#ifndef RENDERPASS_H
#define RENDERPASS_H
#include <vulkan/vulkan_core.h>
#include <vector>
#include "../Utils/utils.h"

namespace VK::Render {
    class RenderPass {
    public:
        VkRenderPass m_renderPass;
        void setAttachmentDescription(const VkAttachmentDescription& attachmentDescription);
        void setAttachmentReference(const VkAttachmentReference& attachmentReference);
        void setSubpassDescription(const VkSubpassDescription& subpassDescription);
        void createRenderPass(const VkPhysicalDevice& physicalDeivce,const VkDevice& device, const VkFormat& format, RenderPassType renderPassType);
        void setSubpassDependency(const VkSubpassDependency& subpassDependency);
        void DestroyRenderPass() const;
    private:
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkAttachmentReference> attachmentReferences;
        std::vector<VkSubpassDescription> subpass;
        std::vector<VkSubpassDependency> subpassDependencies;
        VkDevice device{};
        VkFormat swapchainImageFormat{};
        VkPhysicalDevice physicalDevice{};
    };
}



#endif //RENDERPASS_H
