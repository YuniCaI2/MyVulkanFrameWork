//
// Created by 51092 on 25-2-24.
//

#ifndef PHYSICALDEVICE_H
#define PHYSICALDEVICE_H
#include "Device.h"


namespace VK {
    class PhysicalDevice {
    public:
        vk::PhysicalDevice m_physicalDevice;
        std::vector<const char*> m_deviceExtensions;
    public:
        PhysicalDevice setDeviceExtension(const char * extension);
        vk::PhysicalDevice Device() const;
        PhysicalDevice(const vk::Instance& instance);
    private:
        bool isDeviceSuitable(const vk::PhysicalDevice& physicalDevice);
        void pickPhysicalDevice(const vk::Instance& instance);
    };
}



#endif //PHYSICALDEVICE_H
