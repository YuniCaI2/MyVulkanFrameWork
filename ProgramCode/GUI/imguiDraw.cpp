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
    for (const auto& framebuffer : uiFrameBuffers) {
        framebuffer.destroyFrameBuffers();
    }
    this->renderPass.DestroyRenderPass();
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
        pool_info.poolSizeCount = (sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize));
        pool_info.maxSets = 1000;
        VkResult err = vkCreateDescriptorPool(device.vkDevice, &pool_info, VK_NULL_HANDLE, &descriptorPool);
        Utils::checkVkResult(err);
    }
    //RenderPass
    {
        renderPass.createRenderPass(device.physicalDevice, device.vkDevice, swapChain.format, RenderPassType::GUI);
    }
    {
        for (size_t i = 0; i < swapChain.swapChainImages.size(); i++) {
            VK::Render::FrameBuffer framebuffer{};
            std::vector<VkImageView> attachments{};
            attachments.push_back(swapChain.swapChainImageViews[i]);
            framebuffer.createFrameBuffers(device, renderPass, swapChain, attachments);
            uiFrameBuffers.push_back(framebuffer);
        }
    }


    //渲染完成的信号量
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
        ImGui_ImplVulkan_CreateFontsTexture();
    }
}

void GUI::imguiDraw::BeginRender() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::imguiDraw::DrawUI() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));          // 固定在左上角
    ImGui::SetNextWindowSize(ImVec2(300, static_cast<float>(vulkanInstance->swapChain.extent.height))); // 宽度 200，高度填满
    ImGui::Begin("Settings");

    // 添加 UI 控件...
    ImGui::End();
}

VkRenderPass GUI::imguiDraw::EndRender() {
    ImGui::Render();
    auto rp = FrameRender();
    return rp;
}

VkRenderPass GUI::imguiDraw::FrameRender() {
    return renderPass.m_renderPass;

    // 提交CommandBuffer

}
//Recreate后续增加