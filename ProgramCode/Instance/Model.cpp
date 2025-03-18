#include "Model.h"
#include <opencv2/opencv.hpp>  // 包含 OpenCV，用于加载 OBJ 模型的纹理图像
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>  // 包含 tiny_gltf.h，用于解析 glTF 文件
#include <glm/ext/matrix_transform.hpp>

#include "../OBJ_Loader.h"     // OBJ 文件加载器
#include "../Utils/utils.h"    // 工具函数，包含 Vulkan 图像布局转换等功能

void VK::Instances::Model::computeTangents(Mesh &mesh) {
    // 临时存储切线，初始化为零向量
    std::vector<glm::vec3> tangents(mesh.vertices.size(), glm::vec3(0.0f));

    // 遍历每个三角形（每 3 个索引）
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        // 获取三角形的三个顶点
        Vertex& v0 = mesh.vertices[mesh.indices[i]];
        Vertex& v1 = mesh.vertices[mesh.indices[i + 1]];
        Vertex& v2 = mesh.vertices[mesh.indices[i + 2]];

        // 计算边向量
        glm::vec3 E1 = v1.pos - v0.pos;
        glm::vec3 E2 = v2.pos - v0.pos;

        // 计算 UV 差异
        glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
        glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

        // 计算分母，避免除以零
        float r = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
        if (r == 0.0f) continue; // 如果 UV 重叠，跳过
        r = 1.0f / r;

        // 计算切线
        glm::vec3 tangent = (E1 * deltaUV2.y - E2 * deltaUV1.y) * r;

        // 累加切线到每个顶点（平滑处理）
        tangents[mesh.indices[i]] += tangent;
        tangents[mesh.indices[i + 1]] += tangent;
        tangents[mesh.indices[i + 2]] += tangent;
    }

    // 归一化和正交化
    for (size_t i = 0; i < mesh.vertices.size(); i++) {
        glm::vec3& T = tangents[i];
        glm::vec3& N = mesh.vertices[i].normal;

        if (glm::length(T) > 0.0f) { // 确保切线非零
            T = glm::normalize(T);
            // Gram-Schmidt 正交化，确保 T 与 N 垂直
            T = glm::normalize(T - N * glm::dot(N, T));
            mesh.vertices[i].tangent = T;
        } else {
            // 如果切线计算失败，使用默认值（例如法线的某个正交方向）
            mesh.vertices[i].tangent = glm::normalize(glm::cross(N, glm::vec3(0.0f, 1.0f, 0.0f)));
        }
    }
}

// 构造函数，初始化模型对象
VK::Instances::Model::Model() {
    glTFModel = new tinygltf::Model();  // 分配 tinygltf::Model 对象，用于存储 glTF 数据
}

// 析构函数，清理模型资源
VK::Instances::Model::~Model() {
    delete glTFModel;  // 释放 tinygltf::Model 对象，避免内存泄漏
}

// 加载模型文件，支持 OBJ 和 glTF 格式
void VK::Instances::Model::LoadModel(const VK::Device& device, const std::string& path, ModelType type, const VkCommandPool& commandPool) {
    this->device = device;  // 保存 Vulkan 设备引用
    descriptorManager.initialManager(device.vkDevice);  // 初始化描述符管理器，用于管理纹理描述符
    modelMatrix = glm::mat4(1.0f);  // 初始化模型变换矩阵为单位矩阵，表示无变换
    modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // 根据模型类型加载文件
    if (type == ModelType::OBJ) {
        modelType = ModelType::OBJ;  // 设置模型类型为 OBJ
        objl::Loader loader;  // 创建 OBJ 文件加载器
        bool loadSuccess = loader.LoadFile(path);  // 加载 OBJ 文件
        if (!loadSuccess) {
            throw std::runtime_error("无法加载 OBJ 文件: " + path);  // 加载失败时抛出异常
        }

        // 遍历加载的每个网格
        for (const auto& mesh : loader.LoadedMeshes) {
            Mesh _mesh;  // 创建新的网格对象
            // 转换 OBJ 顶点数据到自定义 Vertex 结构体
            for (const auto& vertex : mesh.Vertices) {
                Vertex vertexData;
                vertexData.pos = glm::vec3(vertex.Position.X, vertex.Position.Y, vertex.Position.Z);  // 顶点位置
                vertexData.texCoord = glm::vec2(vertex.TextureCoordinate.X, vertex.TextureCoordinate.Y);  // 纹理坐标
                vertexData.color = vertexData.pos;  // 使用位置作为颜色（简单着色）
                vertexData.normal = glm::vec3(vertex.Normal.X, vertex.Normal.Y, vertex.Normal.Z);  // 法线向量
                _mesh.vertices.push_back(vertexData);  // 添加顶点到网格
            }
            // 获取纹理路径（从 OBJ 文件的材质中提取）
            size_t p = path.find_last_of("/\\");  // 找到路径中的最后一个分隔符
            _mesh.texPath[0] = path.substr(0, p + 1) + mesh.MeshMaterial.map_Kd;  // 拼接纹理文件路径
            _mesh.indices = mesh.Indices;  // 设置索引数据
            meshes.push_back(_mesh);  // 将网格添加到模型的网格数组
        }
    } else if (type == ModelType::glTF) {
        modelType = ModelType::glTF;  // 设置模型类型为 glTF
        tinygltf::TinyGLTF loader;  // 创建 glTF 文件加载器
        std::string err, warn;  // 用于存储加载时的错误和警告信息
        // 判断文件扩展名，选择加载方式
        bool isBinary = (path.substr(path.find_last_of('.') + 1) == "glb");
        bool ret;
        if (isBinary) {
            ret = loader.LoadBinaryFromFile(glTFModel, &err, &warn, path);  // 加载 .glb 文件
        } else {
            ret = loader.LoadASCIIFromFile(glTFModel, &err, &warn, path);  // 加载 .gltf 文件
        }
        if (!ret) {
            throw std::runtime_error("无法加载 glTF 文件: " + path + " 错误: " + err);
        }

        // 遍历 glTF 文件中的每个网格
        for (const auto& mesh : glTFModel->meshes) {
            Mesh _mesh;  // 创建新的网格对象
            // 遍历网格中的每个图元（Primitive）
            for (const auto& primitive : mesh.primitives) {
                // 获取位置数据
                const auto& posAccessor = glTFModel->accessors[primitive.attributes.at("POSITION")];
                const auto& posBufferView = glTFModel->bufferViews[posAccessor.bufferView];
                const auto& posBuffer = glTFModel->buffers[posBufferView.buffer];
                const auto* positions = reinterpret_cast<const float*>(
                    &posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);

                // 获取法线数据
                const auto& normAccessor = glTFModel->accessors[primitive.attributes.at("NORMAL")];
                const auto& normBufferView = glTFModel->bufferViews[normAccessor.bufferView];
                const auto& normBuffer = glTFModel->buffers[normBufferView.buffer];
                const auto* normals = reinterpret_cast<const float*>(
                    &normBuffer.data[normBufferView.byteOffset + normAccessor.byteOffset]);

                // 获取纹理坐标（如果存在）
                const float* texCoords = nullptr;
                if (primitive.attributes.count("TEXCOORD_0")) {
                    const auto& texAccessor = glTFModel->accessors[primitive.attributes.at("TEXCOORD_0")];
                    const auto& texBufferView = glTFModel->bufferViews[texAccessor.bufferView];
                    const auto& texBuffer = glTFModel->buffers[texBufferView.buffer];
                    texCoords = reinterpret_cast<const float*>(
                        &texBuffer.data[texBufferView.byteOffset + texAccessor.byteOffset]);
                }

                // 转换顶点数据
                for (size_t i = 0; i < posAccessor.count; i++) {
                    Vertex vertex;
                    vertex.pos = glm::vec3(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);  // 位置
                    vertex.normal = glm::vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);  // 法线
                    vertex.texCoord = texCoords ? glm::vec2(texCoords[i * 2], texCoords[i * 2 + 1]) : glm::vec2(0.0f);  // 纹理坐标
                    vertex.color = vertex.pos;  // 使用位置作为颜色
                    _mesh.vertices.push_back(vertex);  // 添加顶点到网格
                }

                // 获取索引数据
                const auto& indexAccessor = glTFModel->accessors[primitive.indices];
                const auto& indexBufferView = glTFModel->bufferViews[indexAccessor.bufferView];
                const auto& indexBuffer = glTFModel->buffers[indexBufferView.buffer];
                const uint8_t* indexData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];

                // 根据索引数据类型（uint16 或 uint32）解析索引
                if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const auto* indices = reinterpret_cast<const uint16_t*>(indexData);
                    for (size_t i = 0; i < indexAccessor.count; i++) {
                        _mesh.indices.push_back(static_cast<uint32_t>(indices[i]));  // 转换为 uint32
                    }
                } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    const auto* indices = reinterpret_cast<const uint32_t*>(indexData);
                    for (size_t i = 0; i < indexAccessor.count; i++) {
                        _mesh.indices.push_back(indices[i]);  // 直接使用 uint32
                    }
                }

                // 获取纹理信息（如果存在）
                if (primitive.material >= 0) {
                    const auto& material = glTFModel->materials[primitive.material];
                    if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
                        _mesh.texPath[0] = std::to_string(material.pbrMetallicRoughness.baseColorTexture.index);  // 存储纹理索引
                        _mesh.texPath[1] = std::to_string(material.normalTexture.index);
                        _mesh.texPath[2]  = std::to_string(material.pbrMetallicRoughness.metallicRoughnessTexture.index);
                       _mesh.texPath[3]  = std::to_string(material.emissiveTexture.index);
                        _mesh.texPath[4]  = std::to_string(material.occlusionTexture.index);
                    }
                }
            }
            // computeTangents(_mesh);  // 为每个网格计算切线
            meshes.push_back(_mesh);  // 将网格添加到模型的网格数组
        }
    }

    // 创建 Vulkan 资源
    createSampler(device);  // 创建纹理采样器
    createModelIndexBuffer(device, commandPool);  // 创建索引缓冲区
    createModelTextureImage(device, commandPool);  // 创建纹理图像
    createModelVertexBuffer(device, commandPool);  // 创建顶点缓冲区
    descriptorManager.setSampler(sampler.sampler);  // 设置采样器到描述符管理器
    descriptorManager.createSets();  // 创建描述符集
}

// 创建纹理采样器
void VK::Instances::Model::createSampler(const VK::Device& device) {
    sampler.createSampler(device);  // 调用采样器对象的创建方法，配置 Vulkan 采样器
}

// 创建顶点缓冲区
void VK::Instances::Model::createModelVertexBuffer(const VK::Device& device, const VkCommandPool& commandPool) {
    for (auto& mesh : meshes) {  // 遍历每个网格
        VkDeviceSize size = sizeof(mesh.vertices[0]) * mesh.vertices.size();  // 计算顶点数据大小
        Buffer stagingBuffer;  // 创建暂存缓冲区，用于将数据从 CPU 传输到 GPU
        stagingBuffer.createBuffer(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);  // 配置暂存缓冲区
        stagingBuffer.Map();  // 映射内存以写入数据
        memcpy(stagingBuffer.data, mesh.vertices.data(), static_cast<size_t>(size));  // 复制顶点数据到暂存缓冲区
        stagingBuffer.UnMap();  // 取消映射

        // 创建顶点缓冲区（GPU 专用）
        mesh.vertexBuffer.buffer.createBuffer(device, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        mesh.vertexBuffer.buffer.copy(stagingBuffer.buffer, commandPool);  // 将数据从暂存缓冲区复制到顶点缓冲区
        stagingBuffer.destroyBuffer();  // 销毁暂存缓冲区
    }
}

// 创建索引缓冲区
void VK::Instances::Model::createModelIndexBuffer(const VK::Device& device, const VkCommandPool& commandPool) {
    for (auto& mesh : meshes) {  // 遍历每个网格
        VkDeviceSize size = sizeof(mesh.indices[0]) * mesh.indices.size();  // 计算索引数据大小
        Buffer stagingBuffer;  // 创建暂存缓冲区
        stagingBuffer.createBuffer(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);  // 配置暂存缓冲区
        stagingBuffer.Map();  // 映射内存
        memcpy(stagingBuffer.data, mesh.indices.data(), static_cast<size_t>(size));  // 复制索引数据
        stagingBuffer.UnMap();  // 取消映射

        // 创建索引缓冲区（GPU 专用）
        mesh.indexBuffer.buffer.createBuffer(device, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        mesh.indexBuffer.buffer.copy(stagingBuffer.buffer, commandPool);  // 将数据从暂存缓冲区复制到索引缓冲区
        stagingBuffer.destroyBuffer();  // 销毁暂存缓冲区
    }
}

// 创建纹理图像
void VK::Instances::Model::createModelTextureImage(const VK::Device& device, const VkCommandPool& commandPool) {
    if (modelType == ModelType::OBJ) {  // 处理 OBJ 模型的纹理
        for (auto& mesh : meshes) {
            cv::Mat image = cv::imread(mesh.texPath[0]);  // 使用 OpenCV 加载纹理图像
            if (image.empty()) {
                throw std::runtime_error("无法加载纹理图像: " + mesh.texPath[0]);  // 加载失败抛出异常
            }
            cv::Mat resizedImage;
            cv::cvtColor(image, resizedImage, cv::COLOR_BGRA2RGBA);  // 转换图像格式为 RGBA
            VkDeviceSize imageSize = resizedImage.rows * resizedImage.cols * 4;  // 计算图像数据大小
            auto* imageData = reinterpret_cast<uint8_t*>(resizedImage.data);  // 获取图像数据指针
            mesh.texture.image.width = resizedImage.cols;  // 设置纹理宽度
            mesh.texture.image.height = resizedImage.rows;  // 设置纹理高度

            Buffer stagingBuffer;  // 创建暂存缓冲区
            stagingBuffer.createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            stagingBuffer.Map();  // 映射内存
            memcpy(stagingBuffer.data, imageData, static_cast<size_t>(imageSize));  // 复制图像数据
            stagingBuffer.UnMap();  // 取消映射

            // 创建 Vulkan 图像
            mesh.texture.image.createImage(device, resizedImage.cols, resizedImage.rows, 1, 1,
                                           VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB,
                                           VK_IMAGE_TILING_OPTIMAL,
                                           VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            Utils::transitionImageLayout(device, commandPool, mesh.texture.image.image,
                                         VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1);  // 转换图像布局为传输目标
            mesh.texture.image.copy(stagingBuffer.buffer, commandPool);  // 复制数据到图像
            Utils::transitionImageLayout(device, commandPool, mesh.texture.image.image,
                                         VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);  // 转换图像布局为着色器可读
            stagingBuffer.destroyBuffer();  // 销毁暂存缓冲区
            mesh.texture.image.imageView = Utils::createImageView(device.vkDevice, mesh.texture.image.image,
                                                                  VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1,1);  // 创建图像视图
            descriptorManager.setImageView(mesh.texture.image.imageView);  // 设置图像视图到描述符管理器
        }
    } else if (modelType == ModelType::glTF) {  // 处理 glTF 模型的纹理
        for (auto& mesh : meshes) {
            if (!mesh.texPath[0].empty()) {  // 如果网格有纹理//此处无鲁棒性，后续增加
                std::vector<uint8_t> rgbaData(0);
                for (const auto & i : mesh.texPath) {
                    int textureIndex = std::stoi(i);  // 将纹理路径转换为索引
                    const auto& image = glTFModel->images[glTFModel->textures[textureIndex].source];  // 获取纹理图像数据

                    rgbaData.insert(rgbaData.end(), image.image.begin(), image.image.end());

                    mesh.texture.image.width = image.width;  // 设置纹理宽度
                    mesh.texture.image.height = image.height;  // 设置纹理高度
                }
                VkDeviceSize imageSize = rgbaData.size();
                Buffer stagingBuffer;  // 创建暂存缓冲区
                stagingBuffer.createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                stagingBuffer.Map();  // 映射内存
                memcpy(stagingBuffer.data, rgbaData.data(), static_cast<size_t>(imageSize));  // 复制图像数据
                stagingBuffer.UnMap();  // 取消映射

                // 创建 Vulkan 图像
                mesh.texture.image.createImage(device,  mesh.texture.image.width,  mesh.texture.image.height, 1, 5,
                                               VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB,
                                               VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                Utils::transitionImageLayout(device, commandPool, mesh.texture.image.image,
                                             VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 5);  // 转换图像布局
                mesh.texture.image.copy(stagingBuffer.buffer, commandPool);  // 复制数据到图像
                Utils::transitionImageLayout(device, commandPool, mesh.texture.image.image,
                                             VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 5);  // 转换图像布局
                stagingBuffer.destroyBuffer();  // 销毁暂存缓冲区
                mesh.texture.image.imageView = Utils::createImageView(device.vkDevice, mesh.texture.image.image,
                                                                      VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1,5);  // 创建图像视图
                descriptorManager.setImageView(mesh.texture.image.imageView);  // 设置图像视图
            }
        }
    }
    descriptorManager.setMaxSets(meshes.size());  // 设置描述符集的最大数量
}

// 绘制模型
void VK::Instances::Model::draw(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout) {
    // 推送模型变换矩阵到顶点着色器
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &modelMatrix);
    for (size_t i = 0; i < meshes.size(); i++) {  // 遍历每个网格
        VkBuffer vertexBuffers[] = {meshes[i].vertexBuffer.buffer.buffer};  // 获取顶点缓冲区
        VkDeviceSize offsets[] = {0};  // 缓冲区偏移量
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);  // 绑定顶点缓冲区
        vkCmdBindIndexBuffer(commandBuffer, meshes[i].indexBuffer.buffer.buffer, 0, VK_INDEX_TYPE_UINT32);  // 绑定索引缓冲区
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1,
                                &descriptorManager.textureDescriptorSets[i], 0, nullptr);  // 绑定描述符集
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(meshes[i].indices.size()), 1, 0, 0, 0);  // 记录绘制命令
    }
}

// 销毁模型资源
void VK::Instances::Model::destroy() {
    descriptorManager.destroy();  // 销毁描述符管理器
    for (auto& mesh : meshes) {  // 遍历每个网格
        mesh.indexBuffer.buffer.destroyBuffer();  // 销毁索引缓冲区
        mesh.vertexBuffer.buffer.destroyBuffer();  // 销毁顶点缓冲区
        mesh.texture.image.destroyImage();  // 销毁纹理图像
    }
    sampler.destroySampler();  // 销毁纹理采样器
}