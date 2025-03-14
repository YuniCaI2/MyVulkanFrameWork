//
// Created by 51092 on 25-3-6.
//

#include "Model.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <opencv2/opencv.hpp>
#include <tiny_gltf.h>
#include "../OBJ_Loader.h"
#include "../Utils/utils.h"


void VK::Instances::Model::LoadModel(const VK::Device &device, const std::string &path, ModelType type, const VkCommandPool &commandPool) {
    descriptorManager.initialManager(device.vkDevice);
    if (type == ModelType::OBJ) {
        objl::Loader loader;
        bool loadSuccess = loader.LoadFile(path);
        if (!loadSuccess) {
            throw std::runtime_error("Failed to load OBJ file: " + path);
        }

        if (!loader.LoadedMeshes.empty()) {
            for (const auto &mesh: loader.LoadedMeshes) {
                Mesh _mesh;
                for (const auto &vertex: mesh.Vertices) {
                    Vertex vertexData;
                    vertexData.pos.x = vertex.Position.X;
                    vertexData.pos.y = vertex.Position.Y;
                    vertexData.pos.z = vertex.Position.Z;
                    vertexData.texCoord.x = vertex.TextureCoordinate.X;
                    vertexData.texCoord.y = vertex.TextureCoordinate.Y;
                    vertexData.color = vertexData.pos;
                    vertexData.normal.x = vertex.Normal.X;
                    vertexData.normal.y = vertex.Normal.Y;
                    vertexData.normal.z = vertex.Normal.Z;
                    _mesh.vertices.push_back(vertexData);
                }
                size_t p = path.find_last_of("/ \\");
                _mesh.texPath = path.substr(0, p + 1) + mesh.MeshMaterial.map_Kd; //拼接纹理坐标
                _mesh.indices = mesh.Indices;
                std::cout << _mesh.texPath << std::endl;
                meshes.push_back(_mesh);
            }
        }
        this->createSampler(device);
        this->createModelIndexBuffer(device, commandPool);
        this->createModelTextureImage(device, commandPool);
        this->createModelVertexBuffer(device, commandPool);
        descriptorManager.setSampler(sampler.sampler);
        descriptorManager.createSets();
    }
    if (type == ModelType::glTF) {
        // 初始化模型加载器
        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string err, warn;

        // 加载.glb文件（二进制格式）
        bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, "model.glb");

        // 错误处理
        if (!warn.empty()) std::cout << "警告: " << warn << std::endl;
        if (!err.empty()) std::cerr << "错误: " << err << std::endl;
        if (!ret) throw std::runtime_error("加载GLB失败");
    }
}



void VK::Instances::Model::createSampler(const VK::Device &device) {
    sampler.createSampler(device);
}

void VK::Instances::Model::createModelVertexBuffer(const VK::Device &device, const VkCommandPool &commandPool) {
    for (auto &mesh: meshes) {
        std::cout << "Loaded mesh with " << mesh.vertices.size()
                << " vertices and " << mesh.indices.size() << " indices.\n";


        VkDeviceSize size = sizeof(mesh.vertices[0]) * mesh.vertices.size();
        Buffer stagingBuffer{};
        stagingBuffer.createBuffer(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.Map();
        memcpy(stagingBuffer.data, mesh.vertices.data(), static_cast<size_t>(size));
        stagingBuffer.UnMap();

        mesh.vertexBuffer.buffer.createBuffer(device, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        mesh.vertexBuffer.buffer.copy(stagingBuffer.buffer, commandPool);
        stagingBuffer.destroyBuffer();
    }
}

void VK::Instances::Model::createModelIndexBuffer(const VK::Device &device, const VkCommandPool &commandPool) {
    for (auto &mesh: meshes) {
        VkDeviceSize size = sizeof(mesh.indices[0]) * mesh.indices.size();
        Buffer stagingBuffer{};
        stagingBuffer.createBuffer(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.Map();
        memcpy(stagingBuffer.data, mesh.indices.data(), static_cast<size_t>(size));
        stagingBuffer.UnMap();

        mesh.indexBuffer.buffer.createBuffer(device, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        mesh.indexBuffer.buffer.copy(stagingBuffer.buffer, commandPool);
        stagingBuffer.destroyBuffer();
    }
}

void VK::Instances::Model::createModelTextureImage(const VK::Device &device, const VkCommandPool &commandPool) {
    for (auto &mesh: meshes) {
        cv::Mat image = cv::imread(mesh.texPath);
        if (image.empty()) {
            throw std::runtime_error("Failed to load image!");
        }
        cv::Mat resizedImage;
        cv::cvtColor(image, resizedImage, cv::COLOR_BGRA2RGBA);
        VkDeviceSize imageSize = resizedImage.rows * resizedImage.cols * 4;
        auto *imageData2 = reinterpret_cast<uint8_t *>(resizedImage.data);
        mesh.texture.image.width = resizedImage.cols;
        mesh.texture.image.height = resizedImage.rows;

        Buffer stagingBuffer;
        stagingBuffer.createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.Map();
        memcpy(stagingBuffer.data, imageData2, static_cast<size_t>(imageSize));
        stagingBuffer.UnMap();
        //这里先不做mipmap
        mesh.texture.image.createImage(device, resizedImage.cols, resizedImage.rows, 1,1,
                                       VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB,
                                       VK_IMAGE_TILING_OPTIMAL,
                                       VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        Utils::transitionImageLayout(device, commandPool, mesh.texture.image.image,
                                     VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
        mesh.texture.image.copy(stagingBuffer.buffer, commandPool);
        Utils::transitionImageLayout(device, commandPool, mesh.texture.image.image,
                                     VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
        stagingBuffer.destroyBuffer();
        mesh.texture.image.imageView = Utils::createImageView(device.vkDevice, mesh.texture.image.image,
                                                              VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        descriptorManager.setImageView(mesh.texture.image.imageView);
    }
    descriptorManager.setMaxSets(meshes.size());
}




void VK::Instances::Model::destroy() {
    descriptorManager.destroy();
    for (auto &mesh: meshes) {
        mesh.indexBuffer.buffer.destroyBuffer();
        mesh.vertexBuffer.buffer.destroyBuffer();
        mesh.texture.image.destroyImage();
    }
    sampler.destroySampler();
}
