//
// Created by 51092 on 25-3-6.
//

#ifndef MODEL_H
#define MODEL_H
#include <vector>
#include <string>

#include "DescriptorManager.h"
#include "IndexBuffer.h"
#include "Sampler.h"
#include "TextureImage.h"
#include "Vertex.h"
#include "VertexBuffer.h"

enum class ModelType {
    OBJ,
    glTF
};


namespace VK::Instances {


    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        //-- Vulkan 资源 --
        IndexBuffer indexBuffer;
        VertexBuffer vertexBuffer;
        std::string texPath;
        TextureImage texture;
    };

    class Model {
    public:
        ModelType modelType{};
        Sampler sampler{};
        std::vector<Mesh> meshes{};
        VK::Instances::DescriptorManager descriptorManager{};
    private:

        VK::Device device{};

    public:

        void LoadModel(const VK::Device& device,const std::string& path, ModelType type , const VkCommandPool &commandPool);
        void createSampler(const VK::Device& device);
        void createModelVertexBuffer(const VK::Device& device, const VkCommandPool& commandPool);
        void createModelIndexBuffer(const VK::Device& device, const VkCommandPool& commandPool);
        void createModelTextureImage(const VK::Device& device, const VkCommandPool& commandPool);
        void draw(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout);
        void destroy();
    };
}


#endif //MODEL_H
