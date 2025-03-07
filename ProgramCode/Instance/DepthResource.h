//
// Created by cai on 25-3-7.
//

#ifndef DEPTHRESOURCE_H
#define DEPTHRESOURCE_H
#include <vector>
#include "../Core/Device.h"
#include "./Image.h"

namespace VK::Instances {
    class DepthResource {
    public:
        std::vector<Image> depthImages = {};
        void createDepthResources(const VK::Device& device, const VkExtent2D& extent,const uint32_t& depthBufferNum);
        void destroyDepthResources() const;
        std::vector<VkImageView> getImageViews() const;
    private:
        VkDevice device{};
    };

}



#endif //DEPTHRESOURCE_H
