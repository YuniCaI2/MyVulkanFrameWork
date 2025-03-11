//
// Created by 51092 on 25-3-9.
//

#ifndef IMGUIDRAW_H
#define IMGUIDRAW_H
#include "../RenderInstance.h"

namespace GUI {
    class imguiDraw {
    public:
        //Resources
        VkDescriptorPool descriptorPool{};
        VK::Render::RenderPass renderPass{};
        std::vector<VK::Render::FrameBuffer> uiFrameBuffers{};
        VK::Instances::CommandBufferManager commandBufferManager{};

        imguiDraw(RenderInstance* instance);
        ~imguiDraw();
        void initVulkanResource(const VK::Instance& instance,GLFWwindow* window, VK::Device& device, const VK::SwapChain& swapChain);
        void BeginRender();
        void DrawUI();
        VkCommandBuffer EndRender();
        VkCommandBuffer SubmitFrameRender();
        void recreateFrameBuffers();
    private:
        void destroyFrameBuffers() const;
        RenderInstance* vulkanInstance{nullptr};
    };
}



#endif //IMGUIDRAW_H
