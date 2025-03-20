#version 450
layout(location = 0) out vec3 outTexCoord;

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

void main() {
    // 硬编码立方体的 36 个顶点（6 个面，每个面 2 个三角形）
    vec3 positions[36] = vec3[](
    // 前面
    vec3(-1.0, -1.0,  1.0), vec3( 1.0, -1.0,  1.0), vec3( 1.0,  1.0,  1.0),
    vec3(-1.0, -1.0,  1.0), vec3( 1.0,  1.0,  1.0), vec3(-1.0,  1.0,  1.0),
    // 后面
    vec3(-1.0, -1.0, -1.0), vec3(-1.0,  1.0, -1.0), vec3( 1.0,  1.0, -1.0),
    vec3(-1.0, -1.0, -1.0), vec3( 1.0,  1.0, -1.0), vec3( 1.0, -1.0, -1.0),
    // 左面
    vec3(-1.0, -1.0, -1.0), vec3(-1.0, -1.0,  1.0), vec3(-1.0,  1.0,  1.0),
    vec3(-1.0, -1.0, -1.0), vec3(-1.0,  1.0,  1.0), vec3(-1.0,  1.0, -1.0),
    // 右面
    vec3( 1.0, -1.0, -1.0), vec3( 1.0,  1.0, -1.0), vec3( 1.0,  1.0,  1.0),
    vec3( 1.0, -1.0, -1.0), vec3( 1.0,  1.0,  1.0), vec3( 1.0, -1.0,  1.0),
    // 顶面
    vec3(-1.0,  1.0, -1.0), vec3(-1.0,  1.0,  1.0), vec3( 1.0,  1.0,  1.0),
    vec3(-1.0,  1.0, -1.0), vec3( 1.0,  1.0,  1.0), vec3( 1.0,  1.0, -1.0),
    // 底面
    vec3(-1.0, -1.0, -1.0), vec3( 1.0, -1.0, -1.0), vec3( 1.0, -1.0,  1.0),
    vec3(-1.0, -1.0, -1.0), vec3( 1.0, -1.0,  1.0), vec3(-1.0, -1.0,  1.0)
    );

    // 根据 gl_VertexIndex 获取顶点位置
    vec3 pos = positions[gl_VertexIndex];

    // 输出纹理坐标（直接使用顶点位置作为 Cubemap 的采样方向）
    outTexCoord = pos;

    // 计算最终顶点位置
    // 移除视图矩阵的平移部分（天空盒应始终围绕相机）
    mat4 viewNoTranslation = mat4(mat3(ubo.view));
    gl_Position = ubo.proj * viewNoTranslation * vec4(pos, 1.0);

    // 确保天空盒在深度测试中位于最远（并轻微调整深度）
    gl_Position.z = gl_Position.w * 0.9999; // 关键修改： 深度值略微减小
}
