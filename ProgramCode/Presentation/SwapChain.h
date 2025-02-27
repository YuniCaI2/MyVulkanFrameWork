//
// Created by 51092 on 25-2-26.
//

#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H
#include "../Core/Device.h"

namespace VK {

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class SwapChain {
    public:
        VkSwapchainKHR swapChain;
        VkDevice device;
        VkExtent2D extent;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        void createSwapChain(VkPhysicalDevice physicalDevice, const VK::Device& device,GLFWwindow* window, VkSurfaceKHR surface);

        static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        void DestroySwapChain() const;
    private:
        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
        void createImageViews();
    };
}



#endif //SWAPCHAIN_H
