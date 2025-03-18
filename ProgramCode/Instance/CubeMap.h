//
// Created by 51092 on 25-3-18.
//

#ifndef CUBEMAP_H
#define CUBEMAP_H
#include "Image.h"


namespace VK::Instances {
    class CubeMap {
    public:
        CubeMap() = default;
        void createCubeMap(const VK::Device& device,const std::string& cubeMapPath);
        void destroy();
        Image image{};
    private:
        VkDevice m_Device;
    };
}



#endif //CUBEMAP_H
