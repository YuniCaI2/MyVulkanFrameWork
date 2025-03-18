//
// Created by 51092 on 25-3-18.
//

#include "CubeMap.h"
#include<opencv2/opencv.hpp>
#include "Buffer.h"
void VK::Instances::CubeMap::createCubeMap(const VK::Device& device, const std::string &cubeMapPath) {
    cv::Mat image = cv::imread(cubeMapPath);  // 使用 OpenCV 加载纹理图像
    if (image.empty()) {
        throw std::runtime_error("无法加载纹理图像: " + cubeMapPath);  // 加载失败抛出异常
    }
    if (image.depth() == CV_16U || image.depth() == CV_16S) {

    } else if (image.depth() == CV_32F) {

    }
    cv::Mat resizedImage;
    cv::cvtColor(image, resizedImage, cv::COLOR_BGRA2RGBA);  // 转换图像格式为 RGBA
    VkDeviceSize imageSize = resizedImage.rows * resizedImage.cols * 8;  // 计算图像数据大小//HDRI为16位图像
    auto* imageData = reinterpret_cast<uint16_t*>(resizedImage.data);  // 获取图像数据指针
    this->image.width = resizedImage.cols;  // 设置纹理宽度
    this->image.height = resizedImage.rows;  // 设置纹理高度

    Buffer stagingBuffer;  // 创建暂存缓冲区
    stagingBuffer.createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.Map();  // 映射内存
    memcpy(stagingBuffer.data, imageData, static_cast<size_t>(imageSize));  // 复制图像数据
    stagingBuffer.UnMap();  // 取消映射
}

void VK::Instances::CubeMap::destroy() {
    image.destroyImage();
}