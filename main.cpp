#include <iostream>
#include <vulkan/vulkan.hpp>
#include<opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include<GLFW/glfw3.h>
#include "Core/Instance.h"
#include "Core/physicalDevice.h"
#include "Presentation/surface.h"
#include "Queue/QueueFamily.h"
#include "Core/Device.h"
#include "Presentation/SwapChain.h"
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




public:
    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }
    void cleanup() {
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
