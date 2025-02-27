//
// Created by 51092 on 25-2-24.
//

#ifndef PHYSICALDEVICE_H
#define PHYSICALDEVICE_H
#include "Instance.h"


namespace VK {
    class PhysicalDevice {
    public:
        VkPhysicalDevice m_physicalDevice;
        std::vector<const char*> m_deviceExtensions;
    public:
        PhysicalDevice setDeviceExtension(const char * extension);
        VkPhysicalDevice Device() const;
        void createPhysicalDevice(const VkInstance& instance);
    private:
        bool isDeviceSuitable(const VkPhysicalDevice& physicalDevice);
        void pickPhysicalDevice(const VkInstance& instance);
    };
}



#endif //PHYSICALDEVICE_H
