//
// Created by 51092 on 25-3-6.
//

#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>

// 前向声明 tinygltf::Model，避免直接包含 tiny_gltf.h，减少头文件依赖
namespace tinygltf {
    class Model;
}

#include "DescriptorManager.h"  // 描述符管理器类，用于管理纹理等资源的描述符集
#include "IndexBuffer.h"       // 索引缓冲区类，封装 Vulkan 索引缓冲区
#include "Sampler.h"           // 纹理采样器类，封装 Vulkan 采样器
#include "TextureImage.h"      // 纹理图像类，封装 Vulkan 图像和图像视图
#include "Vertex.h"            // 顶点结构体，定义顶点数据（位置、法线、纹理坐标等）
#include "VertexBuffer.h"      // 顶点缓冲区类，封装 Vulkan 顶点缓冲区

// 模型类型枚举，用于指定加载的模型格式
enum class ModelType {
    OBJ,  // OBJ 文件格式，一种基于文本的简单 3D 模型格式
    glTF  // glTF 文件格式，支持 .gltf（JSON 格式）和 .glb（二进制格式）
};

namespace VK::Instances {

    // 网格结构，存储顶点数据、索引数据以及对应的 Vulkan 渲染资源
    struct Mesh {
        std::vector<Vertex> vertices;  // 顶点数据数组，包含位置、法线、纹理坐标等信息
        std::vector<uint32_t> indices; // 索引数据数组，定义顶点的绘制顺序，用于优化渲染性能

        // Vulkan 相关的资源
        IndexBuffer indexBuffer;       // 索引缓冲区对象，将索引数据存储到 GPU 内存中
        VertexBuffer vertexBuffer;     // 顶点缓冲区对象，将顶点数据存储到 GPU 内存中
        std::string texPath[5];           // 纹理路径，OBJ 使用文件路径，glTF 使用纹理索引的字符串形式
        std::string normalPath;
        std::string metallicRoughnessPath;
        std::string aoPath;
        std::string emissivePath;
        TextureImage texture;          // 纹理图像对象，包含 Vulkan 图像、图像视图和相关内存
    };

    // 模型类，负责加载、渲染和管理 3D 模型及其资源
    class Model {
    public:
        ModelType modelType{};                     // 模型类型，标识当前加载的是 OBJ 还是 glTF 模型
        Sampler sampler{};                         // 纹理采样器对象，控制纹理采样方式（如线性插值）
        std::vector<Mesh> meshes{};                // 网格数组，一个模型可能包含多个网格
        glm::mat4 modelMatrix{};                   // 模型变换矩阵，用于平移、旋转、缩放模型
        VK::Instances::DescriptorManager descriptorManager{}; // 描述符管理器，管理纹理的描述符集

    private:
        // 使用前向声明的 tinygltf::Model 指针，避免直接包含完整定义
        tinygltf::Model* glTFModel = nullptr;      // glTF 模型数据，仅用于 glTF 格式，存储加载的 glTF 数据
        VK::Device device{};                       // Vulkan 设备引用，用于创建和管理 Vulkan 资源
        void computeTangents(Mesh& mesh);
    public:
        // 构造函数，初始化模型对象
        Model();

        // 析构函数，清理模型资源
        ~Model();

        // 加载模型文件
        // 参数：
        // - device: Vulkan 设备对象，提供 Vulkan 上下文
        // - path: 模型文件的路径（如 .obj 或 .glb 文件）
        // - type: 模型类型（OBJ 或 glTF）
        // - commandPool: Vulkan 命令池，用于执行资源创建命令
        void LoadModel(const VK::Device& device, const std::string& path, ModelType type, const VkCommandPool& commandPool);

        // 创建纹理采样器
        // 参数：
        // - device: Vulkan 设备对象
        void createSampler(const VK::Device& device);

        // 创建顶点缓冲区，将顶点数据上传至 GPU
        // 参数：
        // - device: Vulkan 设备对象
        // - commandPool: Vulkan 命令池
        void createModelVertexBuffer(const VK::Device& device, const VkCommandPool& commandPool);

        // 创建索引缓冲区，将索引数据上传至 GPU
        // 参数：
        // - device: Vulkan 设备对象
        // - commandPool: Vulkan 命令池
        void createModelIndexBuffer(const VK::Device& device, const VkCommandPool& commandPool);

        // 创建纹理图像，将纹理数据上传至 GPU
        // 参数：
        // - device: Vulkan 设备对象
        // - commandPool: Vulkan 命令池
        void createModelTextureImage(const VK::Device& device, const VkCommandPool& commandPool);

        // 绘制模型，记录绘制命令到命令缓冲区
        // 参数：
        // - commandBuffer: Vulkan 命令缓冲区
        // - pipelineLayout: Vulkan 管线布局，定义着色器资源绑定
        void draw(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout);

        // 销毁模型资源，释放所有 Vulkan 对象
        void destroy();
    };
}

#endif //MODEL_H