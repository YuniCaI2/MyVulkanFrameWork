//
// Created by 51092 on 25-3-6.
//

#ifndef MODEL_H
#define MODEL_H
#include <vector>
#include <string>

#include "IndexBuffer.h"
#include "Sample.h"
#include "TextureImage.h"
#include "Vertex.h"
#include "VertexBuffer.h"

enum class ModelType {
    OBJ
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
        Sample sampler;
        std::vector<Mesh> meshes;
        glm::mat4 modelMatrix;
        void LoadModel(const VK::Device& device,const std::string& path, ModelType type);
        void createSampler(const VK::Device& device);
        void createModelVertexBuffer(const VK::Device& device, const VkCommandPool& commandPool);
        void createModelIndexBuffer(const VK::Device& device, const VkCommandPool& commandPool);
        void createModelTextureImage(const VK::Device& device, const VkCommandPool& commandPool);
        void destroy() const;
    };
}


#endif //MODEL_H
