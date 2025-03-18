//
// Created by 51092 on 25-3-6.
//

#ifndef UNIFORMBUFFER_H
#define UNIFORMBUFFER_H
#include "Buffer.h"
#include "Light.h"
#include "../Camera.h"


namespace VK::Instances {
    class UniformBuffer {
    public:
        Buffer buffer;
        static void update(const UniformBuffer& uniformBuffer,const VkExtent2D& extent, const Camera& camera, const std::vector<Light>& light);
    };
}


#endif //UNIFORMBUFFER_H
