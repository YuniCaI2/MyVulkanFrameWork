//
// Created by cai on 25-2-27.
//

#ifndef PIPELINE_H
#define PIPELINE_H
#include <vulkan/vulkan_core.h>


namespace VK::Render {
        class Pipeline {
        public:
            VkPipeline m_pipeline;

            void bindVertex();


            void createPipeline(VkDevice device);
        private:
            VkShaderModule vertexShader;
            VkShaderModule fragmentShader;


            static VkShaderModule createShaderModule(VkDevice device, const char *pShaderFile);

        };
    }



#endif //PIPELINE_H
