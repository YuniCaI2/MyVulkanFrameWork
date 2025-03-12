#include "RenderInstance.h"
#include "GUI/imguiDraw.h"

class AppInstance {
public:
    RenderInstance *renderInstance = RenderInstance::getInstance();
    GUI::imguiDraw *imguiDrawInstance = new GUI::imguiDraw();


    ~AppInstance() {
        delete imguiDrawInstance;
        renderInstance->cleanup();
        delete renderInstance;
    }

public:
    void run() const {
        renderInstance->initVulkan();
        mainLoop();
    }

    void mainLoop() const {
        double lastFrame = glfwGetTime();
        double timeForComputeFrame = glfwGetTime();
        int frameCount = 0;
        RenderType lastRenderType = renderInstance->renderType;
        while (!glfwWindowShouldClose(renderInstance->window)) {
            glfwPollEvents();
            RenderType currentRenderType = renderInstance->renderType;
            if (currentRenderType != lastRenderType) {
                renderInstance->recreateRenderResource(currentRenderType);
                imguiDrawInstance->recreateFrameBuffers();
                lastRenderType = currentRenderType;
                continue;
            }
            imguiDrawInstance->BeginRender();
            imguiDrawInstance->DrawUI();
            auto result = renderInstance->getAvaliableImageIndex();
            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                renderInstance->recreateSwapChain();
                imguiDrawInstance->recreateFrameBuffers();
                continue;
            } else if (result != VK_SUBOPTIMAL_KHR && result != VK_SUCCESS) {
                throw std::runtime_error("failed to acquire swap chain image!");
            }
            //Fence导致必须其要放到前面
            auto uiCommandBuffer = imguiDrawInstance->EndRender();
            renderInstance->submitCommandBuffer(uiCommandBuffer);
            result = renderInstance->presentFrame();
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                //交换链已经与表面不兼容，无法再用于渲染。通常与窗口大小发生调整有关。
                //交换链仍然可以成功呈现到表面，但是表面属性不再完全匹配
                renderInstance->recreateSwapChain();
                imguiDrawInstance->recreateFrameBuffers();
                continue;
            } else if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to present swap chain image!");
            }
            frameCount++;
            double currentFrame = glfwGetTime();
            double deltaTime = currentFrame - lastFrame;
            double addTime = currentFrame - timeForComputeFrame;
            RenderInstance::processInput(renderInstance->window, deltaTime, renderInstance->myCamera,
                                         renderInstance->mouseFlag);
            if (addTime >= 1.0f) {
                renderInstance->currentFPS = frameCount / addTime;
                frameCount = 0;
                timeForComputeFrame = currentFrame;
            }
            lastFrame = currentFrame;
            renderInstance->currentFrame = (renderInstance->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
            lastRenderType = currentRenderType;
        }
            vkDeviceWaitIdle(renderInstance->device.vkDevice); //保证所有资源同步可以正确删除
    }
};


int main() {
    AppInstance appInstance;
    appInstance.run();
}
