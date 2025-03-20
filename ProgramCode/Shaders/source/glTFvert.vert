#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 9) out vec3 fragPos;

struct Light {
    vec3 position;
    vec3 color;
    uint type;
    vec3 direction;
    float spotAngle;
    float spotFalloff;
};

layout(std140, set = 0, binding = 0) uniform UBO {
    vec3 viewPos;
    mat4 view;
    mat4 proj;
    int lightCount;
    Light lights; // 保持一致，假设单个灯光
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
} model;

void main() {
    fragPos = vec3(model.modelMatrix * vec4(inPosition, 1.0)); // 世界空间位置
    gl_Position = ubo.proj * ubo.view * model.modelMatrix * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = vec2(inTexCoord.x, inTexCoord.y);

    fragNormal = normalize(vec3(model.modelMatrix * vec4(normal, 0.0)));

}