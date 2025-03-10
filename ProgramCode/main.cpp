#include "RenderInstance.h"
#include "GUI/imguiDraw.h"

class AppInstance {
public:
    RenderInstance* renderInstance = new RenderInstance();
    GUI::imguiDraw* imguiDrawInstance = new GUI::imguiDraw(renderInstance);
    ~AppInstance() {
        delete imguiDrawInstance;
        renderInstance->cleanup();
        delete renderInstance;
    }
public:
    void run() const{
        renderInstance->initVulkan();
        mainLoop();
    }
    void mainLoop() const{
        double lastFrame = glfwGetTime();
        double timeForComputeFrame = glfwGetTime();
        int frameCount = 0;
        while (!glfwWindowShouldClose(renderInstance->window)) {
            glfwPollEvents();

            imguiDrawInstance->BeginRender();
            imguiDrawInstance->DrawUI();
            auto uiRenderPass = imguiDrawInstance->EndRender();;
            renderInstance->drawFrame(uiRenderPass, imguiDrawInstance->uiFrameBuffers);
            frameCount++;
            double currentFrame = glfwGetTime();
            double deltaTime = currentFrame - lastFrame;
            double addTime = currentFrame - timeForComputeFrame;
            RenderInstance::processInput(renderInstance->window, deltaTime, renderInstance->myCamera, renderInstance->mouseFlag);
            if (addTime >= 1.0f) {
                double fps = frameCount / addTime;
                std::cout << "FPS: " << fps << std::endl;
                frameCount = 0;
                timeForComputeFrame = currentFrame;
            }
            lastFrame = currentFrame;
            renderInstance->currentFrame = (renderInstance->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }
        vkDeviceWaitIdle(renderInstance->device.vkDevice); //保证所有资源同步可以正确删除

    }



};


int main() {
    AppInstance appInstance;
    appInstance.run();
}
