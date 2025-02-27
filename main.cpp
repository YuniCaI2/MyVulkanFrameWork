#include <iostream>
#include <vulkan/vulkan.hpp>
#include<opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include<GLFW/glfw3.h>
#include "Core/Instance.h"
#include "Core/physicalDevice.h"
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class MyVulkan {
public:
    GLFWwindow* window;
    VK::Instance instance{enableValidationLayers};
    VK::PhysicalDevice physicalDevice{instance.instance};




public:
    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }
    void cleanup() {
        instance.DestroyInstance();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    void initVulkan() {
        createWindow();
    }
    void createWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(640, 480, "Vulkan", NULL, NULL);
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
