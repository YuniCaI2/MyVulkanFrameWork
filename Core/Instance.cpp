//
// Created by 51092 on 25-2-23.
//

#include "Instance.h"
#include<iostream>

VK::Instance::Instance(bool enable) {
    enableValidationLayers = enable;
    createInstance();
    setupDebugMessenger();
}

std::vector<const char *> VK::Instance::getRequiredExtensions() const {
    uint32_t glfwExtensionCount {0};
    const char** glfwExtension = glfwGetRequiredInstanceExtensions (&glfwExtensionCount);
    std::vector<const char *> extensions(glfwExtension, glfwExtension + glfwExtensionCount);
    if (enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    vk::DebugUtilsMessageTypeFlagsEXT messageType,
    const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
    ) {
    if (messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    }
    return VK_FALSE;
}

static bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vk::Result result = vk::enumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<vk::LayerProperties> availableLayers;
    availableLayers.resize(layerCount);
    result = vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to enumerate instance layers");
    }

    bool layersFound = false;
    for (auto& layerProperties : availableLayers) {
        if(strcmp("VK_LAYER_KHRONOS_validation", layerProperties.layerName) == 0) {
            layersFound = true;
            break;
        }
    }
    return layersFound;

}

void VK::Instance::createInstance() {
    try {

        if(enableValidationLayers && !checkValidationLayerSupport()) {
            //这里就是在调试模式下，确保验证层可用
            throw std::runtime_error("Check validation layers requested");
        }

        vk::ApplicationInfo appInfo;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 290);
        appInfo.pEngineName = "No Engine";
        appInfo.pApplicationName = "Vulkan Application";
        appInfo.engineVersion = VK_MAKE_VERSION(1,3,290);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        vk::InstanceCreateInfo createInfo{{}, &appInfo};

        auto glfwExtension = getRequiredExtensions();
        std::vector<const char*> requireExtensions;
        requireExtensions.reserve(glfwExtension.size());
for (const auto& extension : glfwExtension) {
            requireExtensions.push_back(extension);
        }
        createInfo.enabledExtensionCount = requireExtensions.size();
        createInfo.ppEnabledExtensionNames = requireExtensions.data();

        vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;
        if(enableValidationLayers) {
            createInfo.enabledLayerCount = 1;

            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugMessengerCreateInfo);
            createInfo.pNext = &debugMessengerCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
            createInfo.pNext = nullptr;
        }
        instance = vk::createInstance(createInfo);
        dispatchLoaderInstance = vk::detail::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);

    }
    catch ( vk::SystemError & err )
    {
        std::cout << "vk::SystemError: " << err.what() << std::endl;
        exit( -1 );
    }
    catch ( std::exception & err )
    {
        std::cout << "std::exception: " << err.what() << std::endl;
        exit( -1 );
    }
    catch ( ... )
    {
        std::cout << "unknown error\n";
        exit( -1 );
    }
}

void VK::Instance::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
        .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance|
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding)
        .setPfnUserCallback(debugCallback)
        .setPUserData(nullptr);
}



void VK::Instance::setupDebugMessenger() {
    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;
    populateDebugMessengerCreateInfo(debugMessengerCreateInfo);
    if(instance.createDebugUtilsMessengerEXT(&debugMessengerCreateInfo, nullptr, &debugMessenger, dispatchLoaderInstance) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to set up debug messenger");
    }//上述代码等价于手动获取扩展函数地址
}


void VK::Instance::DestroyDebugUtils() const{
    instance.destroyDebugUtilsMessengerEXT(debugMessenger,nullptr, dispatchLoaderInstance );
}

void VK::Instance::DestroyInstance() const {
    if (enableValidationLayers) {
        DestroyDebugUtils();
    }
    instance.destroy(nullptr);
}




