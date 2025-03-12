//
// Created by cai on 25-3-11.
//

#include "ColorResource.h"
#include "../Utils/utils.h"
void VK::Instances::ColorResource::createColorResources(const VK::Device &device, const VkExtent2D &extent,
                                                        const VkFormat& colorFormat,
                                                        const VkSampleCountFlagBits &msaaCount,
                                                        const uint32_t &colorBufferNum) {
    this->device = device.vkDevice;
    colorImages.resize(0);
    colorImages.reserve(colorBufferNum);
    for (uint32_t i = 0; i < colorBufferNum; i++) {
        VK::Instances::Image colorImage;
        colorImage.createImage(device, extent.width, extent.height, 1,
                               msaaCount, colorFormat,
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        colorImage.imageView = Utils::createImageView(device.vkDevice, colorImage.image, colorFormat,
                                                      VK_IMAGE_ASPECT_COLOR_BIT, 1);
        colorImages.push_back(colorImage);
    }
}

void VK::Instances::ColorResource::destroyColorResources() const {
    for(const auto &image : colorImages) {
        image.destroyImage();
    }
}

std::vector<VkImageView> VK::Instances::ColorResource::getImageViews() const {
    std::vector<VkImageView> imageViews;
    imageViews.reserve(colorImages.size());
    for (const auto& colorImage : colorImages) {
        imageViews.push_back(colorImage.imageView);
    }
    return imageViews;
}
