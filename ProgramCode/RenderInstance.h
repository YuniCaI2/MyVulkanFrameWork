//
// Created by 51092 on 25-3-9.
//

#ifndef MYVULKAN_H
#define MYVULKAN_H
#include "Instance/ColorResource.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool constexpr enableValidationLayers = true;
#endif


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
#include "Render/FrameBuffer.h"
#include "Camera.h"

#if defined(_WIN32) || defined(_WIN64)
#define MODEL_PATH "D:/Model/blue-archive-sunohara-kokona/cocona.obj"
#elif defined(__APPLE__) && defined(__MACH__)
#define MODEL_PATH "/Users/yunicai/Model/blue-archive-sunohara-kokona/cocona.obj"
#endif

enum class RenderType {
    NORMAL,
    MSAA
};


class RenderInstance {
    RenderInstance() {
        //核心
        createWindow();
        instance.createInstance(enableValidationLayers);
        physicalDevice.createPhysicalDevice(instance.instance);
        surface.createSurface(instance, window);
        queueFamilies.createQueueFamily(physicalDevice.m_physicalDevice, surface.m_surface);
        device.createDevice(queueFamilies, physicalDevice, enableValidationLayers);
        swapChain.createSwapChain(physicalDevice.Device(), device, window, surface.m_surface);
    } // 🚫必须私有构造函数
public:
    RenderInstance(const RenderInstance &) = delete; // 🔒禁止拷贝
    RenderInstance &operator=(const RenderInstance &) = delete; // 🔐禁止赋值
public:
    static RenderInstance *getInstance() {
        static auto *instance = new RenderInstance();
        return instance;
    };
    //Core
    GLFWwindow *window{};
    VK::Instance instance{};
    VK::PhysicalDevice physicalDevice{};
    VK::Surface surface{};
    VK::QueueFamily queueFamilies{};
    VK::Device device{};
    VK::SwapChain swapChain{};
    //Render
    VK::Render::RenderPass renderPass{};
    VK::Render::Pipeline pipeline{};
    VK::Instances::DescriptorManager descriptorManager{};
    std::vector<VK::Render::FrameBuffer> presentFrameBuffers{};
    VK::Instances::DepthResource depthResource{};
    VK::Instances::CommandBufferManager commandBufferManager{};
    VK::Instances::Model model{};
    std::vector<VK::Instances::UniformBuffer> uniformBuffers{};
    VK::Instances::SyncManager syncManager{};
    uint32_t imageIndex{};
    std::vector<VK::Render::FrameBuffer> uiFrameBuffers{};
    inline static bool mouseFlag{false};

    //RenderInstance 所对应的RenderType
    RenderType renderType{RenderType::NORMAL};


    //视角移动变量
    bool firstMouse{true};
    float lastX{};
    float lastY{};
    Camera myCamera{};

    //屏幕信息
    uint32_t currentFrame{0};
    float currentFPS;

    //前向渲染实例——MSAA设置
    VkSampleCountFlagBits msaaSamples{VK_SAMPLE_COUNT_1_BIT};
    VkSampleCountFlagBits maxMsaaSamples{VK_SAMPLE_COUNT_1_BIT};
    std::vector<VK::Render::FrameBuffer> massFrameBuffers{};
    VK::Instances::ColorResource colorResource{};

public:
    void recordCommandBuffer(const VkCommandBuffer &commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //这个参数指定我们如何使用帧缓冲区
        beginInfo.pInheritanceInfo = nullptr; //这个参数与辅助缓冲区有关；
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        //starting a render pass
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass.m_renderPass;
        renderPassInfo.framebuffer = presentFrameBuffers[imageIndex].Buffer;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChain.extent;

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        //注意clearValue是Union
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
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
                                pipeline.pipelineLayout, 0, 1,
                                &descriptorManager.uniformDescriptorSets[currentFrame], 0, nullptr);
        VK::Instances::UniformBuffer::update(uniformBuffers[currentFrame], swapChain.extent, myCamera);
        model.draw(commandBuffer, pipeline.pipelineLayout);
        // for (auto i = 0; i < model.meshes.size(); i++) {
        //     VkBuffer vertexBuffers[] = {model.meshes[i].vertexBuffer.buffer.buffer};
        //     VkDeviceSize offsets[] = {0};
        //     vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        //     vkCmdBindIndexBuffer(commandBuffer, model.meshes[i].indexBuffer.buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        //     //加载模型中的顶点
        //     vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        //                             pipeline.pipelineLayout, 1, 1,
        //                             &descriptorManager.textureDescriptorSets[i], 0, nullptr);
        //     vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.meshes[i].indices.size()), 1, 0, 0, 0);
        //     //后面的参数用来对齐索引和顶点
        // }
        vkCmdEndRenderPass(commandBuffer);
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    //分解drawFrame，便于加载UI
    auto getAvaliableImageIndex() {
        vkWaitForFences(device.vkDevice, 1, &syncManager.Fences[currentFrame],
                        VK_TRUE, UINT64_MAX); //后面参数是超时参数，VK_TRUE指的是要等待所有的Fence
        VkResult result = vkAcquireNextImageKHR(device.vkDevice, swapChain.swapChain,
                                                UINT64_MAX, syncManager.Semaphores[currentFrame * 2],
                                                VK_NULL_HANDLE, &imageIndex);
        return result;
    }

    void submitCommandBuffer(const VkCommandBuffer &addCommandBuffer) {
        vkResetFences(device.vkDevice, 1, &syncManager.Fences[currentFrame]); //手动设置为无信号,上一轮

        //这里是以纳秒为单位的
        vkResetCommandBuffer(commandBufferManager.commandBuffers[currentFrame], 0);
        recordCommandBuffer(commandBufferManager.commandBuffers[currentFrame], imageIndex);
        //这里来记录我们想要的命令,录制命令
        //有了完整的命令，之后就要提交它
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphore[] = {syncManager.Semaphores[currentFrame * 2]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        std::array<VkCommandBuffer, 2> commandBuffers = {
            commandBufferManager.commandBuffers[currentFrame],
            addCommandBuffer
        };
        submitInfo.commandBufferCount = commandBuffers.size();
        submitInfo.pCommandBuffers = commandBuffers.data();
        VkSemaphore signalSemaphores[] = {syncManager.Semaphores[currentFrame * 2 + 1]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        if (vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, syncManager.Fences[currentFrame]) != VK_SUCCESS) {
            //Fence由UI托管
            throw std::runtime_error("failed to submit command buffer command buffer submission!");
        }
    }

    auto presentFrame() {
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        VkSemaphore waitSemaphoreForPresent[] = {syncManager.Semaphores[currentFrame * 2 + 1]};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = waitSemaphoreForPresent;
        //等待信号量实现后，再进行执行

        VkSwapchainKHR swapChains[] = {swapChain.swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;
        /*它允许您指定VkResult值数组来检查每个单独的交换链是否呈现成功。
         如果您只使用单个交换链，则没有必要，因为您可以简单地使用当前函数的返回值。
         */
        auto result = vkQueuePresentKHR(device.presentQueue, &presentInfo);
        return result;
    }

    //重建交换链和帧缓冲
    void recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }
        //destroy
        vkDeviceWaitIdle(device.vkDevice);
        cleanupSwapChain();
        //recreate
        swapChain.createSwapChain(device.physicalDevice, device, window, surface.m_surface);
        depthResource.createDepthResources(device, swapChain.extent, msaaSamples, swapChain.swapChainImages.size());
        if (renderType == RenderType::MSAA) {
            colorResource.createColorResources(device, swapChain.extent, swapChain.format, msaaSamples,
                                               swapChain.swapChainImages.size());
            for (auto i = 0; i < presentFrameBuffers.size(); ++i) {
                std::vector<VkImageView> attachments{};
                attachments.push_back(colorResource.getImageViews()[i]);
                attachments.push_back(swapChain.swapChainImageViews[i]);
                attachments.push_back(depthResource.getImageViews()[i]);
                presentFrameBuffers[i].createFrameBuffers(device, renderPass, swapChain, attachments);
            }
        } else {
            for (auto i = 0; i < presentFrameBuffers.size(); ++i) {
                std::vector<VkImageView> attachments{};
                attachments.push_back(swapChain.swapChainImageViews[i]);
                attachments.push_back(depthResource.getImageViews()[i]);
                presentFrameBuffers[i].createFrameBuffers(device, renderPass, swapChain, attachments);
            }
        }
    }


    void cleanupSwapChain() {
        for (const auto &framebuffer: presentFrameBuffers) {
            framebuffer.destroyFrameBuffers();
        }
        if (renderType == RenderType::MSAA) {
            colorResource.destroyColorResources();
        }
        depthResource.destroyDepthResources();
        swapChain.DestroySwapChain();
    }


    void cleanup() {
        model.destroy();
        syncManager.destroySyncObjects();
        for (auto i = 0; i < uniformBuffers.size(); i++) {
            uniformBuffers[i].buffer.destroyBuffer();
        }
        commandBufferManager.destroyCommandBuffers();
        descriptorManager.destroy();
        cleanupSwapChain();

        if (!(RenderType::MSAA == renderType)) {
            colorResource.destroyColorResources();
            //保证删除color Resource
        }

        pipeline.Destroy();
        renderPass.DestroyRenderPass();
        surface.DestroySurface();
        device.Destroy();
        instance.DestroyInstance();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void initVulkan() {
        renderPass.createRenderPass(physicalDevice.Device(), device.vkDevice, swapChain.format, msaaSamples,
                                    RenderPassType::FORWARD);
        depthResource.createDepthResources(device, swapChain.extent, msaaSamples, swapChain.swapChainImages.size());
        //创建交换链的帧缓冲
        for (size_t i = 0; i < swapChain.swapChainImages.size(); i++) {
            VK::Render::FrameBuffer framebuffer{};
            std::vector<VkImageView> attachments{};
            attachments.push_back(swapChain.swapChainImageViews[i]);
            attachments.push_back(depthResource.getImageViews()[i]);
            framebuffer.createFrameBuffers(device, renderPass, swapChain, attachments);
            presentFrameBuffers.push_back(framebuffer);
        }

        commandBufferManager.createCommandBuffers(device, 2);

        model.LoadModel(device, MODEL_PATH, ModelType::OBJ, commandBufferManager.commandPool);
        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for (auto &uniformBuffer: uniformBuffers) {
            uniformBuffer.buffer.createBuffer(device, sizeof(UniformBufferObject),
                                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            uniformBuffer.buffer.Map();
        }
        syncManager.createSyncObjects(device, MAX_FRAMES_IN_FLIGHT * 1, MAX_FRAMES_IN_FLIGHT * 2
        );
        descriptorManager.initialManager(device.vkDevice);
        descriptorManager.setSampler(model.sampler.sampler);
        for (const auto &uniformBuffer: uniformBuffers) {
            descriptorManager.setUniformBuffer(uniformBuffer.buffer.buffer);
        }
        for (const auto &mesh: model.meshes) {
            descriptorManager.setImageView(mesh.texture.image.imageView);
        }
        descriptorManager.setMaxSets(model.meshes.size() + uniformBuffers.size());
        descriptorManager.createSets();
        pipeline.initial(device.vkDevice).
                setShader("../ProgramCode/Shaders/spv/vert.spv", ShaderStage::VERT)
                .setShader("../ProgramCode/Shaders/spv/frag.spv", ShaderStage::FRAG)
                .setRasterizerState()
                .setMultisampleState()
                .setColorBlendState().setDepthStencilState().createPipelineLayout(
                    {descriptorManager.uniformDescriptorSetLayout, descriptorManager.textureDescriptorSetLayout},
                    VK_SHADER_STAGE_VERTEX_BIT,sizeof(glm::mat4)
                )
                .createPipeline(swapChain
                                , renderPass.m_renderPass);
        //创建颜色附件用于MSAA
        colorResource.createColorResources(device, swapChain.extent, swapChain.format, msaaSamples,
                                           swapChain.swapChainImages.size());
        //MSAA参数：
        maxMsaaSamples = Utils::getMaxUsableSampleCount(device.physicalDevice);
    }

    void recreateRenderResource(RenderType currentRenderType) {
        if (renderType == RenderType::MSAA) {
            msaaSamples = maxMsaaSamples;
            vkDeviceWaitIdle(device.vkDevice);
            cleanupSwapChain();
            renderPass.DestroyRenderPass();
            pipeline.Destroy();
            swapChain.createSwapChain(device.physicalDevice, device, window, surface.m_surface);
            renderPass.createRenderPass(physicalDevice.Device(), device.vkDevice, swapChain.format, msaaSamples,
                                        RenderPassType::MSAA);
            depthResource.createDepthResources(device, swapChain.extent, msaaSamples,
                                               swapChain.swapChainImages.size());
            colorResource.createColorResources(device, swapChain.extent, swapChain.format, msaaSamples,
                                               swapChain.swapChainImages.size());
            for (auto i = 0; i < presentFrameBuffers.size(); ++i) {
                std::vector<VkImageView> attachments{};
                attachments.push_back(colorResource.getImageViews()[i]);
                attachments.push_back(depthResource.getImageViews()[i]);
                attachments.push_back(swapChain.swapChainImageViews[i]);
                presentFrameBuffers[i].createFrameBuffers(device, renderPass, swapChain, attachments);
            }
            pipeline.initial(device.vkDevice).
                    setShader("../ProgramCode/Shaders/spv/vert.spv", ShaderStage::VERT)
                    .setShader("../ProgramCode/Shaders/spv/frag.spv", ShaderStage::FRAG)
                    .setRasterizerState()
                    .setMultisampleState(msaaSamples)
                    .setColorBlendState().setDepthStencilState().createPipelineLayout(
                        {descriptorManager.uniformDescriptorSetLayout, descriptorManager.textureDescriptorSetLayout},
                        VK_SHADER_STAGE_VERTEX_BIT,sizeof(glm::mat4)
                    )
                    .createPipeline(swapChain, renderPass.m_renderPass);
        }
        if (renderType == RenderType::NORMAL) {
            msaaSamples = VK_SAMPLE_COUNT_1_BIT;
            vkDeviceWaitIdle(device.vkDevice);
            cleanupSwapChain();
            renderPass.DestroyRenderPass();
            pipeline.Destroy();

            swapChain.createSwapChain(device.physicalDevice, device, window, surface.m_surface);
            renderPass.createRenderPass(physicalDevice.Device(), device.vkDevice, swapChain.format, msaaSamples,
                                        RenderPassType::FORWARD);
            depthResource.createDepthResources(device, swapChain.extent, msaaSamples,
                                               swapChain.swapChainImages.size());
            for (auto i = 0; i < presentFrameBuffers.size(); ++i) {
                std::vector<VkImageView> attachments{};
                attachments.push_back(swapChain.swapChainImageViews[i]);
                attachments.push_back(depthResource.getImageViews()[i]);
                presentFrameBuffers[i].createFrameBuffers(device, renderPass, swapChain, attachments);
            }
            pipeline.initial(device.vkDevice).
                    setShader("../ProgramCode/Shaders/spv/vert.spv", ShaderStage::VERT)
                    .setShader("../ProgramCode/Shaders/spv/frag.spv", ShaderStage::FRAG)
                    .setRasterizerState()
                    .setMultisampleState(msaaSamples)
                    .setColorBlendState().setDepthStencilState()
                    .createPipelineLayout(
                        {descriptorManager.uniformDescriptorSetLayout, descriptorManager.textureDescriptorSetLayout},
                        VK_SHADER_STAGE_VERTEX_BIT,sizeof(glm::mat4)
                    )
                    .createPipeline(swapChain, renderPass.m_renderPass);
        }
    }


    void createWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(640 * 2, 480 * 2, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetCursorPosCallback(window, mouseCallBack);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    static void mouseCallBack(GLFWwindow *window, double xposIn, double yposIn) {
        auto app = static_cast<RenderInstance *>(glfwGetWindowUserPointer(window));
        if (!mouseFlag) {
            float xpos = static_cast<float>(xposIn);
            float ypos = static_cast<float>(yposIn);

            if (app->firstMouse) {
                app->lastX = xpos;
                app->lastY = ypos;
                app->firstMouse = false;
            }
            auto xoffset = xpos - app->lastX;
            auto yoffset = app->lastY - ypos;

            app->lastX = xpos;
            app->lastY = ypos;
            app->myCamera.ProcessMouseMovement(xoffset, yoffset);
        } else {
            app->firstMouse = true;
        }
    }

    static void processInput(GLFWwindow *window, double deltaTime, Camera &camera, bool &mouseFlag) {
        static bool lastPressedEsc = false;
        bool currentPressedEsc = (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);
        if (currentPressedEsc && !lastPressedEsc) {
            if (!mouseFlag) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            mouseFlag = !mouseFlag;
        }
        lastPressedEsc = currentPressedEsc;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }
};

#endif //MYVULKAN_H
