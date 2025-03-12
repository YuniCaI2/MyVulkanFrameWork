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
        void createColorResources(const VK::Device& device, const VkExtent2D& extent,const VkFormat& colorFormat,
            const VkSampleCountFlagBits& msaaCount,const uint32_t& colorBufferNum);
        void destroyColorResources() const;
        std::vector<VkImageView> getImageViews() const;

    private:
        VkDevice device{};
    };
}



#endif //COLORRESOURCE_H
