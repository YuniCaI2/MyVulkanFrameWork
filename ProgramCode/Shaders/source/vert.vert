#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;


layout(set = 0,binding = 0) uniform UniformBufferObject {
    vec3 viewPos;
    mat4 view;
    mat4 proj;
} ubo;
layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
} model;


void main() {
    gl_Position = ubo.proj * ubo.view * model.modelMatrix * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = vec2(inTexCoord.x , 1.0f - inTexCoord.y);
//    fragTexIndex = ubo.texIndex;
}