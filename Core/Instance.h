//
// Created by 51092 on 25-2-23.
//
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#ifndef INSTANCE_H
#define INSTANCE_H
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

namespace VK {
    class Instance {
    public:
        Instance(bool enable);
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        bool enableValidationLayers;
        std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        [[nodiscard]] std::vector<const char *> getRequiredExtensions() const;

        void createInstance();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void setupDebugMessenger();
        void DestroyDebugUtils() const;
        void DestroyInstance() const;
    };
}




#endif //INSTANCE_H
