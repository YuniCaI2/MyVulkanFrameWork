#include <iostream>
#include <vulkan/vulkan.hpp>
#include<opencv2/opencv.hpp>
#include<GLFW/glfw3.h>
#include "Core/Instance.h"
#include "Core/physicalDevice.h"
#include "Presentation/surface.h"
#include "Queue/QueueFamily.h"
#include "Core/Device.h"
#include "Instance/DepthResource.h"
#include "Presentation/SwapChain.h"
#include "Render/Pipeline.h"
#include "Render/RenderPass.h"
#include "Instance/DescriptorManager.h"
#include "Instance/Model.h"
#include "Instance/SyncManager.h"
#include "Instance/UniformBuffer.h"
#include "Instance/UniformBufferObject.h"
#include "Instance/VertexBuffer.h"
#include "Render/FrameBuffers.h"
#include "Instance/UniformBufferObject.h"
#include "Camera.h"

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
const bool constexpr  enableValidationLayers = true;
#endif

class MyVulkan {
public:
    //Core
    GLFWwindow* window{};
    VK::Instance instance{};
    VK::PhysicalDevice physicalDevice{};
    VK::Surface surface{};
    VK::QueueFamily queueFamilies{};
    VK::Device device{};
    VK::SwapChain swapChain{};
    VK::Render::RenderPass renderPass{};
    VK::Render::Pipeline pipeline{};
    VK::Instances::DescriptorManager descriptorManager{};
    VK::Render::FrameBuffers frameBuffers{};
    VK::Instances::DepthResource depthResource{};
    VK::Instances::CommandBufferManager commandBufferManager{};
    VK::Instances::Model model{};
    std::vector<VK::Instances::UniformBuffer> uniformBuffers{};
    VK::Instances::SyncManager syncManager{};

private:
    uint32_t currentFrame{0};
    Camera myCamera{};
public:

    void recordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;//这个参数指定我们如何使用帧缓冲区
        beginInfo.pInheritanceInfo = nullptr; //这个参数与辅助缓冲区有关；
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        //starting a render pass
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass.m_renderPass;
        renderPassInfo.framebuffer = frameBuffers.frameBuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChain.extent;

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        //注意clearValue是Union
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,VK_SUBPASS_CONTENTS_INLINE);
        //渲染通道命令将嵌入主命令缓冲区本身，并且不会执行辅助命令缓冲区
        //VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS ：渲染通道命令将从辅助命令缓冲区执行。
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.m_pipeline);
        //第二个参数说明管线是图形管线还是计算管线

        //因为视口和剪切是动态的，所以要在发出前进行设置

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChain.extent.width);
        viewport.height = static_cast<float>(swapChain.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.extent = swapChain.extent;
        scissor.offset = {0, 0};
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline.m_pipelineLayout, 0, 1,
        &descriptorManager.uniformDescriptorSets[currentFrame], 0, nullptr);
        for (auto i = 0; i < model.meshes.size(); i++) {
            VkBuffer vertexBuffers[] = {model.meshes[i].vertexBuffer.buffer.buffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, model.meshes[i].indexBuffer.buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
            //加载模型中的顶点
            VK::Instances::UniformBuffer::update(uniformBuffers[currentFrame], swapChain.extent, myCamera);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline.m_pipelineLayout, 0, 1,
                &descriptorManager.textureDescriptorSets[i], 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.meshes[i].indices.size()), 1, 0, 0, 0);
            //后面的参数用来对齐索引和顶点
        }
        //加载模型中的顶点
        // updateUniformBuffer(currentFrame, i);
        // vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        //     pipelineLayout, 0, 1,
        //     &descriptorSets[currentFrame], 0, nullptr);
        // vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(_indices.size()), 1, 0, 0, 0);
        //后面的参数用来对齐索引和顶点

        vkCmdEndRenderPass(commandBuffer);
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void drawFrame() {

    }


    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }

    void cleanup() {
        syncManager.destroySyncObjects();
        for (auto i = 0; i < uniformBuffers.size(); i++) {
            uniformBuffers[i].buffer.destroyBuffer();
        }
        model.destroy();
        commandBufferManager.destroyCommandBuffers();
        depthResource.destroyDepthResources();
        descriptorManager.destroy();
        frameBuffers.destroyFrameBuffers();
        pipeline.Destroy();
        renderPass.DestroyRenderPass();
        swapChain.DestroySwapChain();
        surface.DestroySurface();
        device.Destroy();
        instance.DestroyInstance();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    void initVulkan() {
        //HardWare
        createWindow();
        instance.createInstance(enableValidationLayers);
        physicalDevice.createPhysicalDevice(instance.instance);
        surface.createSurface(instance, window);
        queueFamilies.createQueueFamily(physicalDevice.m_physicalDevice, surface.m_surface);
        device.createDevice(queueFamilies, physicalDevice, enableValidationLayers);
        swapChain.createSwapChain(physicalDevice.Device(),device, window, surface.m_surface);
        renderPass.createRenderPass(physicalDevice.Device(), device.device, swapChain.format);
        pipeline.initial(device.device).
        setShader("D:/学习/cpp_program/Vulkan/Shaders/spv/vert.spv",ShaderStage::VERT)
        .setShader("D:/学习/cpp_program/Vulkan/Shaders/spv/frag.spv",ShaderStage::FRAG)
        .setRasterizerState()
        .setMultisampleState()
        .setColorBlendState().setDepthStencilState()
        .createPipeline( swapChain,descriptorManager
            , renderPass.m_renderPass );
        depthResource.createDepthResources(device,swapChain.extent,swapChain.swapChainImages.size());
        frameBuffers.createFrameBuffers(device, renderPass, swapChain, depthResource.getImageViews());

        commandBufferManager.createCommandBuffers(device, queueFamilies,1);

        model.LoadModel(device, "/Users/yunicai/Model/blue-archive-sunohara-kokona/cocona.obj", ModelType::OBJ);
        model.createSampler(device);
        model.createModelIndexBuffer(device,commandBufferManager.commandPool);
        model.createModelVertexBuffer(device,commandBufferManager.commandPool);
        model.createModelTextureImage(device,commandBufferManager.commandPool);
        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for (auto& uniformBuffer : uniformBuffers) {
            uniformBuffer.buffer.createBuffer(device,sizeof(UniformBufferObject),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }
        syncManager.createSyncObjects(device, MAX_FRAMES_IN_FLIGHT * 1, MAX_FRAMES_IN_FLIGHT * 2 );
        descriptorManager.initialManager(device.device);
        descriptorManager.setSampler(model.sampler.sampler);
        for (const auto& uniformBuffer : uniformBuffers) {
            descriptorManager.setUniformBuffer(uniformBuffer.buffer.buffer);
        }
        for (const auto& mesh : model.meshes) {
            descriptorManager.setImageView(mesh.texture.image.imageView);
        }
        descriptorManager.setMaxSets(model.meshes.size() + uniformBuffers.size());
        descriptorManager.createSets();




    }
    void createWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(640 * 2, 480 * 2, "Vulkan", nullptr, nullptr);
    }
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }
};


int main() {
    MyVulkan vulkan;
    vulkan.run();
}
