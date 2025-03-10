//
// Created by 51092 on 25-3-9.
//

#ifndef MYVULKAN_H
#define MYVULKAN_H

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
const bool constexpr enableValidationLayers = true;
#endif


#include "./GUI/imgui/imgui_impl_glfw.h"
#include "./GUI/imgui/imgui_impl_vulkan.h"
#include "./GUI/imgui/imgui.h"

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


class RenderInstance {
public:
    uint32_t currentFrame{0};
    //Core
    GLFWwindow *window{};
    VK::Instance instance{};
    VK::PhysicalDevice physicalDevice{};
    VK::Surface surface{};
    VK::QueueFamily queueFamilies{};
    VK::Device device{};
    VK::SwapChain swapChain{};
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

    RenderInstance() {
        //HardWare Core
        createWindow();
        instance.createInstance(enableValidationLayers);
        physicalDevice.createPhysicalDevice(instance.instance);
        surface.createSurface(instance, window);
        queueFamilies.createQueueFamily(physicalDevice.m_physicalDevice, surface.m_surface);
        device.createDevice(queueFamilies, physicalDevice, enableValidationLayers);
        swapChain.createSwapChain(physicalDevice.Device(), device, window, surface.m_surface);
    }

    //视角移动变量
    bool firstMouse{true};
    float lastX{};
    float lastY{};
    Camera myCamera{};

public:
    RenderInstance *getPtr() {
        return this;
    }

    void recordCommandBuffer(const VkCommandBuffer &commandBuffer, uint32_t imageIndex,
                             const VkRenderPass &uiRenderPass, const std::vector<VK::Render::FrameBuffer> &uiFrameBuffers) {
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
                                pipeline.m_pipelineLayout, 0, 1,
                                &descriptorManager.uniformDescriptorSets[currentFrame], 0, nullptr);
        VK::Instances::UniformBuffer::update(uniformBuffers[currentFrame], swapChain.extent, myCamera);
        for (auto i = 0; i < model.meshes.size(); i++) {
            VkBuffer vertexBuffers[] = {model.meshes[i].vertexBuffer.buffer.buffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, model.meshes[i].indexBuffer.buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
            //加载模型中的顶点
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline.m_pipelineLayout, 1, 1,
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

        VkRenderPassBeginInfo uiRenderPassinfo = {};
        uiRenderPassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        uiRenderPassinfo.renderPass = uiRenderPass;
        uiRenderPassinfo.framebuffer = uiFrameBuffers[imageIndex].Buffer;
        uiRenderPassinfo.renderArea.extent.width = swapChain.extent.width;
        uiRenderPassinfo.renderArea.extent.height = swapChain.extent.height;
        uiRenderPassinfo.clearValueCount = 0;

        vkCmdBeginRenderPass(commandBuffer, &uiRenderPassinfo, VK_SUBPASS_CONTENTS_INLINE);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
        vkCmdEndRenderPass(commandBuffer);


        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void drawFrame(const VkRenderPass &uiRenderPass, const std::vector<VK::Render::FrameBuffer> &uiFrameBuffers) {
        vkWaitForFences(device.vkDevice, 1, &syncManager.Fences[currentFrame],
                        VK_TRUE, UINT64_MAX); //后面参数是超时参数，VK_TRUE指的是要等待所有的Fence
        VkResult result = vkAcquireNextImageKHR(device.vkDevice, swapChain.swapChain,
                                                UINT64_MAX, syncManager.Semaphores[currentFrame * 2],
                                                VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            // recreateSwapChain();
            return;
        } else if (result != VK_SUBOPTIMAL_KHR && result != VK_SUCCESS) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        vkResetFences(device.vkDevice, 1, &syncManager.Fences[currentFrame]); //手动设置为无信号

        //这里是以纳秒为单位的
        vkResetCommandBuffer(commandBufferManager.commandBuffers[currentFrame], 0);
        recordCommandBuffer(commandBufferManager.commandBuffers[currentFrame], imageIndex, uiRenderPass, uiFrameBuffers);
        //这里来记录我们想要的命令,录制命令
        //有了完整的命令，之后就要提交它
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphore[] = {syncManager.Semaphores[currentFrame * 2]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBufferManager.commandBuffers[currentFrame];
        VkSemaphore signalSemaphores[] = {syncManager.Semaphores[currentFrame * 2 + 1]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        if (vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, syncManager.Fences[currentFrame]) != VK_SUCCESS) {
            //Fence由UI托管
            throw std::runtime_error("failed to submit command buffer command buffer submission!");
        }
        // VkSubmitInfo uiSubmitInfo = {};
        // uiSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        // uiSubmitInfo.waitSemaphoreCount = 1;
        // uiSubmitInfo.pWaitSemaphores = signalSemaphores;
        // uiSubmitInfo.pWaitDstStageMask = waitStages;
        // uiSubmitInfo.commandBufferCount = 1;
        // uiSubmitInfo.pCommandBuffers = &GUICommandBuffer;
        // uiSubmitInfo.signalSemaphoreCount = 1;
        // uiSubmitInfo.pSignalSemaphores = &syncManager.Semaphores[currentFrame * 3 + 2];
        // if (vkQueueSubmit(device.graphicsQueue, 1, &uiSubmitInfo, syncManager.Fences[currentFrame]) != VK_SUCCESS) {
        //     throw std::runtime_error("failed to submit command buffer ui command buffer submission!");
        // }


        //当命令缓冲区完成时将发出信号

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
        result = vkQueuePresentKHR(device.presentQueue, &presentInfo);

        // if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        //     //交换链已经与表面不兼容，无法再用于渲染。通常与窗口大小发生调整有关。
        //     //交换链仍然可以成功呈现到表面，但是表面属性不再完全匹配
        //     recreateSwapChain();
        //     framebufferResized = false;
        // } else
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }

    void cleanup() {
        model.destroy();
        syncManager.destroySyncObjects();
        for (auto i = 0; i < uniformBuffers.size(); i++) {
            uniformBuffers[i].buffer.destroyBuffer();
        }
        commandBufferManager.destroyCommandBuffers();
        depthResource.destroyDepthResources();
        descriptorManager.destroy();
        for (const auto &framebuffer: presentFrameBuffers) {
            framebuffer.destroyFrameBuffers();
        }
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
        renderPass.createRenderPass(physicalDevice.Device(), device.vkDevice, swapChain.format,
                                    RenderPassType::FORWARD);
        depthResource.createDepthResources(device, swapChain.extent, swapChain.swapChainImages.size());
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

        model.LoadModel(device, MODEL_PATH, ModelType::OBJ);
        model.createSampler(device);
        model.createModelIndexBuffer(device, commandBufferManager.commandPool);
        model.createModelVertexBuffer(device, commandBufferManager.commandPool);
        model.createModelTextureImage(device, commandBufferManager.commandPool);
        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for (auto &uniformBuffer: uniformBuffers) {
            uniformBuffer.buffer.createBuffer(device, sizeof(UniformBufferObject),
                                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            uniformBuffer.buffer.Map();
        }
        syncManager.createSyncObjects(device, MAX_FRAMES_IN_FLIGHT * 1, MAX_FRAMES_IN_FLIGHT * 2);
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
                .setColorBlendState().setDepthStencilState()
                .createPipeline(swapChain, descriptorManager
                                , renderPass.m_renderPass);

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
        if (!mouseFlag) {
            float xpos = static_cast<float>(xposIn);
            float ypos = static_cast<float>(yposIn);
            auto app = static_cast<RenderInstance *>(glfwGetWindowUserPointer(window));

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
        }
    }

    static void processInput(GLFWwindow *window, double deltaTime, Camera &camera, bool &mouseFlag) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            if (!mouseFlag) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            mouseFlag = !mouseFlag;
        }

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
