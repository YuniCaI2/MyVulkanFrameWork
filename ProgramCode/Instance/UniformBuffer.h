//
// Created by 51092 on 25-3-6.
//

#ifndef UNIFORMBUFFER_H
#define UNIFORMBUFFER_H
#include "Buffer.h"
#include "../Camera.h"


namespace VK::Instances {
    class UniformBuffer {
    public:
        Buffer buffer;
        static void update(const UniformBuffer& uniformBuffer,const VkExtent2D& extent, const Camera& camera);
    };
}


#endif //UNIFORMBUFFER_H
