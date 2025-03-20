#version 450
layout(location = 0) in vec3 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform samplerCube cubemap;

void main() {
    // 使用顶点位置作为纹理坐标采样 Cubemap
    outColor = texture(cubemap, inTexCoord);
}