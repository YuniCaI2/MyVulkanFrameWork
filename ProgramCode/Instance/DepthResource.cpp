//
// Created by cai on 25-3-7.
//

#include "DepthResource.h"
#include "../Utils/utils.h"

void VK::Instances::DepthResource::createDepthResources(const VK::Device &device,
    const VkExtent2D &extent,
    const uint32_t &depthBufferNum) {
    this->device = device.vkDevice;
    VkFormat depthFormat = Utils::findDepthFormat(device.physicalDevice);
    depthImages.resize(0);
    depthImages.reserve(depthBufferNum);
    for (uint32_t i = 0; i < depthBufferNum; i++) {
        VK::Instances::Image depthImage;
        depthImage.createImage(device, extent.width, extent.height, 1,
            VK_SAMPLE_COUNT_1_BIT, depthFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        depthImage.imageView = Utils::createImageView(device.vkDevice, depthImage.image, depthFormat,VK_IMAGE_ASPECT_DEPTH_BIT,1);
        depthImages.push_back(depthImage);
    }
}

void VK::Instances::DepthResource::destroyDepthResources() const{
    for (const auto& depthImage : depthImages) {
        depthImage.destroyImage();
    }
}

std::vector<VkImageView> VK::Instances::DepthResource::getImageViews() const {
    std::vector<VkImageView> imageViews;
    imageViews.reserve(depthImages.size());
    for (const auto& depthImage : depthImages) {
        imageViews.push_back(depthImage.imageView);
    }
    return imageViews;

}
