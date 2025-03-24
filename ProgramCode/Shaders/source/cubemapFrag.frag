#version 450
layout(location = 0) in vec3 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform samplerCube cubemap;

vec3 hdrTosdr(vec3 envColor){
    // HDR -> LDR
    envColor = envColor / (envColor + vec3(1.0));
    // Gamma校正（只在颜色为线性空间的渲染管线才需要）

    return envColor;
}

void main() {
    // 使用顶点位置作为纹理坐标采样 Cubemap
    outColor = vec4(hdrTosdr(texture(cubemap, inTexCoord).rgb), 1.0);
}