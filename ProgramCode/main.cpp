#include <iostream>
#include <vulkan/vulkan.hpp>
#include<opencv2/opencv.hpp>
#include<GLFW/glfw3.h>
#include "Core/Instance.h"
#include "Core/physicalDevice.h"
#include "Presentation/surface.h"
#include "Queue/QueueFamily.h"
#include "Core/Device.h"
#include "Presentation/SwapChain.h"
#include "Render/Pipeline.h"
#include "Render/RenderPass.h"
#include "Instance/DescriptorManager.h"
#include "Render/FrameBuffers.h"
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
const bool constexpr  enableValidationLayers = true;
#endif

class MyVulkan {
public:
    //Core
    GLFWwindow* window{};
    VK::Instance instance{};
    VK::PhysicalDevice physicalDevice{};
    VK::Surface surface{};
    VK::QueueFamily queueFamilies{};
    VK::Device device{};
    VK::SwapChain swapChain{};
    VK::Render::RenderPass renderPass{};
    VK::Render::Pipeline pipeline{};
    VK::Instances::DescriptorManager descriptorManager{};
    VK::Render::FrameBuffers frameBuffers{};




public:
    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }
    void cleanup() {
        frameBuffers.destroyFrameBuffers();
        pipeline.Destroy();
        renderPass.DestroyRenderPass();
        swapChain.DestroySwapChain();
        surface.DestroySurface();
        device.Destroy();
        instance.DestroyInstance();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    void initVulkan() {
        //HardWare
        createWindow();
        instance.createInstance(enableValidationLayers);
        physicalDevice.createPhysicalDevice(instance.instance);
        surface.createSurface(instance, window);
        queueFamilies.createQueueFamily(physicalDevice.m_physicalDevice, surface.m_surface);
        device.createDevice(queueFamilies, physicalDevice, enableValidationLayers);
        swapChain.createSwapChain(physicalDevice.Device(),device, window, surface.m_surface);
        renderPass.createRenderPass(physicalDevice.Device(), device.device, swapChain.format);
        pipeline.initial(device.device).
        setShader("D:/学习/cpp_program/Vulkan/Shaders/spv/vert.spv",ShaderStage::VERT)
        .setShader("D:/学习/cpp_program/Vulkan/Shaders/spv/frag.spv",ShaderStage::FRAG)
        .setRasterizerState()
        .setMultisampleState()
        .setColorBlendState().setDepthStencilState()
        .createPipeline( swapChain,descriptorManager
            , renderPass.m_renderPass );
        frameBuffers.createFrameBuffers(device, renderPass, swapChain);
    }
    void createWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(640 * 2, 480 * 2, "Vulkan", nullptr, nullptr);
    }
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }
};


int main() {
    MyVulkan vulkan;
    vulkan.run();
}
