//
// Created by 51092 on 25-2-23.
//

#ifndef INSTANCE_H
#define INSTANCE_H
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
extern std::vector<const char*> validationLayers;
namespace VK {
    class Instance {
    public:
        void createInstance(bool enable);
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        bool enableValidationLayers;
        [[nodiscard]] std::vector<const char *> getRequiredExtensions() const;

        void createInstance();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void setupDebugMessenger();
        void DestroyDebugUtils() const;
        void DestroyInstance() const;
    };
}




#endif //INSTANCE_H
