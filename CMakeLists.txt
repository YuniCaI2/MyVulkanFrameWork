cmake_minimum_required(VERSION 3.29)
project(Vulkan_Learn)


set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(OpenCV REQUIRED)
find_package(glfw3 REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
        ProgramCode/main.cpp
        ProgramCode/Queue/QueueFamily.cpp
        ProgramCode/Core/Device.cpp
        ProgramCode/Core/Instance.cpp
        ProgramCode/Core/physicalDevice.cpp
        ProgramCode/Core/Device.h
        ProgramCode/Core/Instance.h
        ProgramCode/Core/physicalDevice.h
        ProgramCode/Presentation/SwapChain.cpp
        ProgramCode/Presentation/SwapChain.h
        ProgramCode/Presentation/surface.cpp
        ProgramCode/Presentation/surface.h
        ProgramCode/Queue/QueueFamily.h
        ProgramCode/Queue/QueueFamily.cpp
        ProgramCode/Utils/utils.cpp
        ProgramCode/Utils/utils.h
        ProgramCode/Render/Pipeline.cpp
        ProgramCode/Render/Pipeline.h
        ProgramCode/Instance/Vertex.h
        ProgramCode/Render/Shader.cpp
        ProgramCode/Render/Shader.h
        ProgramCode/Instance/DescriptorManager.cpp
        ProgramCode/Instance/DescriptorManager.h
        ProgramCode/Instance/UniformBufferObject.h
        ProgramCode/Render/RenderPass.cpp
        ProgramCode/Render/RenderPass.h
        ProgramCode/Instance/Buffer.cpp
        ProgramCode/Instance/Buffer.h
        ProgramCode/Render/FrameBuffer.cpp
        ProgramCode/Render/FrameBuffer.h
        ProgramCode/Instance/CommandBufferManager.cpp
        ProgramCode/Instance/CommandBufferManager.h
        ProgramCode/Instance/Image.cpp
        ProgramCode/Instance/Image.h
        ProgramCode/Instance/VertexBuffer.cpp
        ProgramCode/Instance/VertexBuffer.h
        ProgramCode/Instance/IndexBuffer.cpp
        ProgramCode/Instance/IndexBuffer.h
        ProgramCode/Instance/UniformBuffer.cpp
        ProgramCode/Instance/UniformBuffer.h
        ProgramCode/Camera.h
        ProgramCode/OBJ_Loader.h
        ProgramCode/Instance/Model.cpp
        ProgramCode/Instance/Model.h
        ProgramCode/Instance/TextureImage.cpp
        ProgramCode/Instance/TextureImage.h
        ProgramCode/Instance/Sampler.cpp
        ProgramCode/Instance/Sampler.h
        ProgramCode/Instance/SyncManager.cpp
        ProgramCode/Instance/SyncManager.h
        ProgramCode/Instance/DepthResource.cpp
        ProgramCode/Instance/DepthResource.h
        ProgramCode/Render/FrameBuffer.cpp
        ProgramCode/GUI/imguiDraw.cpp
        ProgramCode/GUI/imguiDraw.h
        ProgramCode/GUI/imgui/imgui_impl_vulkan.cpp
        ProgramCode/GUI/imgui/imgui_impl_vulkan.h
        ProgramCode/GUI/imgui/imgui_impl_glfw.h
        ProgramCode/GUI/imgui/imgui_impl_glfw.cpp
        ProgramCode/GUI/imgui/imconfig.h
        ProgramCode/GUI/imgui/imstb_truetype.h
        ProgramCode/GUI/imgui/imstb_textedit.h
        ProgramCode/GUI/imgui/imstb_rectpack.h
        ProgramCode/GUI/imgui/imgui_widgets.cpp
        ProgramCode/GUI/imgui/imgui_draw.cpp
        ProgramCode/GUI/imgui/imgui_tables.cpp
        ProgramCode/GUI/imgui/imgui_internal.h
        ProgramCode/GUI/imgui/imgui.cpp
        ProgramCode/GUI/imgui/imgui.h
        ProgramCode/RenderInstance.h
        ProgramCode/Instance/ColorResource.h
        ProgramCode/Instance/ColorResource.cpp
)
target_link_libraries(${PROJECT_NAME} glfw)

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})






# 使用示例（注意参数需要包含完整文件名）
