#version 450

const float PI = 3.14159265359;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 9) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2DArray textureArray;

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
    Light lights; // 直接在片段着色器中使用 UBO 的灯光数据
} ubo;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / max(denom, 0.001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 getNormal(vec3 texNormal)
{
    // 将纹理法线从 [0,1] 转换为 [-1,1]（如果尚未转换）
    vec3 tangentNormal = texNormal ;

    // 计算屏幕空间导数
    vec3 q1 = dFdx(fragPos);        // 位置在 x 方向的偏导数
    vec3 q2 = dFdy(fragPos);        // 位置在 y 方向的偏导数
    vec2 st1 = dFdx(fragTexCoord);  // 纹理坐标在 x 方向的偏导数
    vec2 st2 = dFdy(fragTexCoord);  // 纹理坐标在 y 方向的偏导数

    // 计算雅可比行列式的行列式 (det)
    float det = st1.x * st2.y - st1.y * st2.x;

    // 避免除以零，添加一个小的 epsilon
    if (abs(det) < 0.0001) {
        return normalize(fragNormal); // 如果 det 接近 0，返回顶点法线作为回退
    }

    // 计算切线向量 T，并除以 det 以正确缩放
    vec3 T = normalize((q1 * st2.y - q2 * st1.y) / det);
    vec3 N = normalize(fragNormal);
    vec3 B = normalize(cross(N, T)); // 副切线 B

    // 构造 TBN 矩阵
    mat3 TBN = mat3(T, B, N);

    // 将切线空间法线转换为世界空间
    return normalize(TBN * tangentNormal);
}
//一般的颜色纹理，我们都加载成SRGB格式，因为颜色纹理已经提前被gamma校正了，FORMAT设置成SRGB后，
//API会自动的映射回线形空间，但是我把材质性质的贴图设置为SRGB则也会做这个校正，所以这里要反向校正回去
float linearToSrgb(float value) {
    if (value <= 0.0031308) {
        return value * 12.92;
    } else {
        return 1.055 * pow(value, 1.0 / 2.4) - 0.055;
    }
}

void main() {
    // 计算 TBN 矩阵（世界空间）

    vec3 fragViewPos = ubo.viewPos;
    vec3 albedo = texture(textureArray, vec3(fragTexCoord, 0.0f)).rgb;

    // 法线贴图（加载为 sRGB，需抵消 Vulkan 的自动转换）
    vec4 normalSample = texture(textureArray, vec3(fragTexCoord, 1.0f));
    vec3 normalRaw = vec3(
    linearToSrgb(normalSample.r),
    linearToSrgb(normalSample.g),
    linearToSrgb(normalSample.b)
    );
    vec3 tangentNormal = normalize(normalRaw * 2.0 - 1.0);

    // 金属度粗糙度贴图（假设也加载为 sRGB，需抵消转换）
    vec4 metallicRoughness = texture(textureArray, vec3(fragTexCoord, 2.0f));
    float roughness = max(linearToSrgb(metallicRoughness.g), 0.1); // 粗糙度
    float metallic = linearToSrgb(metallicRoughness.b);           // 金属度

    // 自发光贴图（sRGB 格式，Vulkan 自动转为线性空间）
    vec3 emissive = texture(textureArray, vec3(fragTexCoord, 3.0f)).rgb;

    // AO 贴图（假设也加载为 sRGB，需抵消转换）
    float ao = linearToSrgb(texture(textureArray, vec3(fragTexCoord, 4.0f)).r);//    roughness = 0.2;

//    vec3 N = normalize(fragTBN * tangentNormal);
    vec3 N = getNormal(tangentNormal);
//    N = fragNormal;
    vec3 V = normalize(fragViewPos - fragPos);
    vec3 L = normalize(ubo.lights.position - fragPos); // 使用 UBO 中的灯光位置
    vec3 H = normalize(V + L);

    float distance = length(ubo.lights.position - fragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 lightColor = 2.0 * ubo.lights.color; // 使用 UBO 中的灯光颜色
    vec3 radiance = lightColor * attenuation;

    vec3 F0 = vec3(0.01);
    F0 = mix(F0, albedo, metallic);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD = (1.0 - metallic) * kD;

    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    vec3 ambient = vec3(0.005) * albedo * ao;
//    ambient = vec3(0,0,0);
    vec3 color = (Lo + ambient + emissive);

//    color = color / (color + vec3(1.0)); // 色调映射
//    color = pow(color, vec3(1.0/2.2));   // 伽马校正
//    outColor = vec4(Lo, 1.0);
//    outColor  = vec4(kD, 1.0);
//    outColor = vec4(n,1.0);
//    outColor = vec4(G,G,G,1);
//    outColor = vec4(V,1.0);
//    outColor = vec4(NDF, NDF, NDF,1.0);
//    outColor = vec4(metallic,metallic,metallic,1.0);
//    outColor = vec4(NdotL,NdotL,NdotL, 1.0);
//    outColor = vec4(roughness,roughness,roughness, 1.0);
    outColor = vec4(color,1.0);
//    outColor = vec4(F0,1.0);
//    outColor = vec4(H,1.0);
//    outColor = vec4(specular,1.0);
//    outColor = vec4(N,1.0);
}