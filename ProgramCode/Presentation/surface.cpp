//
// Created by 51092 on 25-2-25.
//

#include "surface.h"

#include <vulkan/vulkan_core.h>

void VK::Surface::createSurface(const VK::Instance &instance, GLFWwindow *window) {
    m_instance = instance.instance;
    if (glfwCreateWindowSurface(instance.instance,window, nullptr, &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void VK::Surface::DestroySurface() const{
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}
