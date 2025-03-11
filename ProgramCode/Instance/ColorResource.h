//
// Created by cai on 25-3-11.
//

#ifndef COLORRESOURCE_H
#define COLORRESOURCE_H
#include "Image.h"

namespace VK::Instances {
    class ColorResource {
    public:
        std::vector<Image> colorImages = {};
        void createColorResources(const VK::Device& device, const VkExtent2D& extent,const uint32_t& depthBufferNum);
        void destroyColorResources() const;
        std::vector<VkImageView> getImageViews() const;
    };
}



#endif //COLORRESOURCE_H
