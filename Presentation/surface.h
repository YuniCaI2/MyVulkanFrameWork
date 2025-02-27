//
// Created by 51092 on 25-2-25.
//

#ifndef SURFACE_H
#define SURFACE_H
#include <vulkan/vulkan_core.h>

#include "../Core/Instance.h"


namespace VK {
    class Surface {
    public:
        VkSurfaceKHR m_surface{};
        VkInstance m_instance;
        void createSurface(const VK::Instance& instance, GLFWwindow* window);
        void DestroySurface() const;
    };
}



#endif //SURFACE_H
