//
// Created by 51092 on 25-3-3.
//

#ifndef BUFFER_H
#define BUFFER_H
#include "CommandBufferManager.h"

//先使用静态的实现，不使用动态内存

namespace VK::Instances {
    class Buffer {
    public:
        VkBuffer buffer{};
        VkDeviceMemory memory{};
        void* data{nullptr};
        void createBuffer(const VK::Device& device,const VkDeviceSize& size,
            const VkBufferUsageFlags& usage, const VkMemoryPropertyFlags& properties);
        void Map() const;
        void UnMap() const;
    private:
        VK::Device device{};
    };
}



#endif //BUFFER_H
