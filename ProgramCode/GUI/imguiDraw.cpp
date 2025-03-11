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


GUI::imguiDraw::imguiDraw(){//主管线的同步信号
    vulkanInstance = RenderInstance::getInstance();
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
    commandBufferManager.destroyCommandBuffers();
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
    {
        commandBufferManager.createCommandBuffers(device, 2);
        //创建
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
    ImGui::Text("FPS: %.1f", vulkanInstance->currentFPS);

    // 1. 按钮
    if (ImGui::Button("Click Me")) {
        // 按钮被点击时的回调
        std::cout << "Button clicked!" << std::endl;
    }

    // 2. 文本输入框
    static char textInput[128] = "Hello";
    ImGui::InputText("Input Text", textInput, IM_ARRAYSIZE(textInput));

    // 3. 滑动条（浮点数）
    static float sliderValue = 0.5f;
    ImGui::SliderFloat("Slider", &sliderValue, 0.0f, 1.0f);

    // 4. 复选框
    static bool checkBoxValue = false;
    ImGui::Checkbox("Checkbox", &checkBoxValue);

    ImGui::Separator();


    // 5. 单选框
    static int radioValue = 0;
    ImGui::RadioButton("Option A", &radioValue, 0); ImGui::SameLine();
    ImGui::RadioButton("Option B", &radioValue, 1);

    ImGui::Separator();


    static const char* items[] = { "Apple", "Banana", "Cherry" };
    static int itemCurrent = 0;
    ImGui::Combo("Combo Box", &itemCurrent, items, IM_ARRAYSIZE(items));

    ImGui::Separator();


    ImGui::ListBox("List", &itemCurrent, items, IM_ARRAYSIZE(items));

    ImGui::Separator();

    if (ImGui::TreeNode("Tree Node")) {
        ImGui::Text("Child Content");
        ImGui::TreePop();
    }

    if (ImGui::BeginTable("Table", 3, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Value");
        ImGui::TableSetupColumn("Status");
        ImGui::TableHeadersRow();

        for (int row = 0; row < 3; row++) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Item %d", row);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.2f", 100.0f * row);
            ImGui::TableSetColumnIndex(2);
            ImGui::Checkbox("##check", &checkBoxValue);
        }
        ImGui::EndTable();
    }



    // 添加 UI 控件...
    ImGui::End();
}

VkCommandBuffer GUI::imguiDraw::EndRender() {
    ImGui::Render();
    return SubmitFrameRender();
}

VkCommandBuffer GUI::imguiDraw::SubmitFrameRender() {
    vkWaitForFences(vulkanInstance->device.vkDevice, 1, &vulkanInstance->syncManager.Fences[vulkanInstance->currentFrame],
        VK_TRUE, UINT64_MAX); //后面参数是超时参数，VK_TRUE指的是要等待所有的Fence
    // 提交CommandBuffer
    vkResetCommandBuffer(commandBufferManager.commandBuffers[vulkanInstance->currentFrame], 0);
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //这个参数指定我们如何使用帧缓冲区
    beginInfo.pInheritanceInfo = nullptr; //这个参数与辅助缓冲区有关；
    if (vkBeginCommandBuffer(commandBufferManager.commandBuffers[vulkanInstance->currentFrame], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    VkRenderPassBeginInfo uiRenderPassinfo = {};
    uiRenderPassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    uiRenderPassinfo.renderPass = renderPass.m_renderPass;
    uiRenderPassinfo.framebuffer = uiFrameBuffers[vulkanInstance->imageIndex].Buffer;
    uiRenderPassinfo.renderArea.extent.width = vulkanInstance->swapChain.extent.width;
    uiRenderPassinfo.renderArea.extent.height = vulkanInstance->swapChain.extent.height;
    uiRenderPassinfo.clearValueCount = 0;

    vkCmdBeginRenderPass(commandBufferManager.commandBuffers[vulkanInstance->currentFrame], &uiRenderPassinfo, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBufferManager.commandBuffers[vulkanInstance->currentFrame]);
    vkCmdEndRenderPass(commandBufferManager.commandBuffers[vulkanInstance->currentFrame]);

    if (vkEndCommandBuffer(commandBufferManager.commandBuffers[vulkanInstance->currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
    return commandBufferManager.commandBuffers[vulkanInstance->currentFrame];
}

void GUI::imguiDraw::recreateFrameBuffers() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(vulkanInstance->window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(vulkanInstance->window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(vulkanInstance->device.vkDevice);
    destroyFrameBuffers();
    for (size_t i = 0; i < vulkanInstance->swapChain.swapChainImages.size(); i++) {
        std::vector<VkImageView> attachments{};
        attachments.push_back(vulkanInstance->swapChain.swapChainImageViews[i]);
        uiFrameBuffers[i].createFrameBuffers(vulkanInstance->device, renderPass, vulkanInstance->swapChain, attachments);
    }
}

void GUI::imguiDraw::destroyFrameBuffers() const {
    for (const auto& framebuffer : uiFrameBuffers) {
        framebuffer.destroyFrameBuffers();
    }
}

//Recreate后续增加
