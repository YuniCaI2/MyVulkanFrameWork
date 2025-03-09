//
// Created by 51092 on 25-3-9.
//

#include "imguiDraw.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

static void check_vk_result(VkResult err) {
    if (err == 0)
        return;
    fprintf(stderr, "ImGui Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}


GUI::imguiDraw::imguiDraw(RenderInstance* myinstance){//主管线的同步信号
    vulkanInstance = myinstance;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    initVulkanResource(vulkanInstance->instance, vulkanInstance->window, vulkanInstance->device, vulkanInstance->swapChain);
}

GUI::imguiDraw::~imguiDraw() {
    vkDeviceWaitIdle(vulkanInstance->device.vkDevice);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(vulkanInstance->device.vkDevice, descriptorPool, nullptr);
}


void GUI::imguiDraw::initVulkanResource(const VK::Instance &instance, GLFWwindow *window, VK::Device &device,
                                        const VK::SwapChain &swapChain) {
    //初始化描述符
    {
        VkDescriptorPoolSize pool_sizes[] =
        {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.pPoolSizes = pool_sizes;
        VkResult err = vkCreateDescriptorPool(device.vkDevice, &pool_info, VK_NULL_HANDLE, &descriptorPool);
        Utils::checkVkResult(err);
    }
    //RenderPass
    {
        renderPass.createRenderPass(device.physicalDevice, device.vkDevice, swapChain.format, RenderPassType::GUI);
    }
    //FrameBuffer
    {
        for (auto i = 0; i < swapChain.swapChainImageViews.size(); ++i) {
            std::vector<VkImageView> attachments(0);
            attachments.push_back(swapChain.swapChainImageViews[i]);
            frameBuffers[i].createFrameBuffers(device, renderPass, swapChain, attachments);
        }
    }
    //CommandPool
    {
        commandBufferManager.createCommandBuffers(device, MAX_FRAMES_IN_FLIGHT);
    }
    //渲染完成的信号量
    {
        syncManager.createSyncObjects(device, 0, MAX_FRAMES_IN_FLIGHT);
    }
    //初始化ImGUI
    {
        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = instance.instance;
        init_info.PhysicalDevice = device.physicalDevice;
        init_info.Device = device.vkDevice;
        init_info.QueueFamily = device.graphicsQueueFamilyIndex;
        init_info.Queue = device.graphicsQueue;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = descriptorPool;
        init_info.RenderPass = renderPass.m_renderPass;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = swapChain.swapChainImageViews.size();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = VK_NULL_HANDLE;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info);
    }
}

void GUI::imguiDraw::BeginRender() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::imguiDraw::DrawUI() {
    ImGui::Begin("Settings");
    ImGui::Text("Application profiler");
    ImGui::End();
}

void GUI::imguiDraw::EndRender() {
    FrameRender();
}

void GUI::imguiDraw::FrameRender() {
    uint32_t currentFrame = vulkanInstance->currentFrame;
    auto commandBuffer = commandBufferManager.commandBuffers[currentFrame];
    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo commandInfo = {};
    commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VkResult err = vkBeginCommandBuffer(commandBuffer, &commandInfo);
    check_vk_result(err);

    VkRenderPassBeginInfo renderPassinfo = {};
    renderPassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassinfo.renderPass = renderPass.m_renderPass;
    renderPassinfo.framebuffer = frameBuffers[currentFrame].Buffer;
    renderPassinfo.renderArea.extent.width = vulkanInstance->swapChain.extent.width;
    renderPassinfo.renderArea.extent.height = vulkanInstance->swapChain.extent.height;
    renderPassinfo.clearValueCount = 0;
    vkCmdBeginRenderPass(commandBuffer, &renderPassinfo, VK_SUBPASS_CONTENTS_INLINE);

    // Record Imgui Draw Data and draw funcs into command buffer
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    vkCmdEndRenderPass(commandBuffer);
    err = vkEndCommandBuffer(commandBuffer);
    check_vk_result(err);

    // 提交CommandBuffer
    std::vector<VkPipelineStageFlags> waitStages = {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
    std::vector<VkSemaphore> waitSemaphores = {vulkanInstance->syncManager.Semaphores[static_cast<size_t>(vulkanInstance->syncManager.Semaphores.size() * MAX_FRAMES_IN_FLIGHT + 1)]};

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.commandBufferCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    submitInfo.pSignalSemaphores = &syncManager.Semaphores[currentFrame];
    submitInfo.signalSemaphoreCount = 1;

    VkFence fence = vulkanInstance->syncManager.Fences[currentFrame];
    if (vkQueueSubmit(vulkanInstance->device.graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    vulkanInstance->syncManager.Semaphores[vulkanInstance->syncManager.Semaphores.size()* MAX_FRAMES_IN_FLIGHT +1] = syncManager.Semaphores[currentFrame];
}
//Recreate后续增加