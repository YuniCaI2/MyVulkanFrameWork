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
        VK::Instances::CommandBufferManager commandBufferManager{};
        VK::Render::RenderPass renderPass{};
        VK::Instances::SyncManager syncManager{};
        std::vector<VK::Render::FrameBuffer> frameBuffers{};

        imguiDraw(RenderInstance* instance);
        ~imguiDraw();
        void initVulkanResource(const VK::Instance& instance,GLFWwindow* window, VK::Device& device, const VK::SwapChain& swapChain);
        void BeginRender();
        void DrawUI();
        void EndRender();
        void FrameRender();
    private:
        RenderInstance* vulkanInstance{nullptr};
    };
}



#endif //IMGUIDRAW_H
