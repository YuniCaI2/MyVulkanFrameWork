#version 450

const float PI = 3.14159265359;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 9) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2DArray textureArray;
layout(set = 2, binding = 0) uniform samplerCube cubeMap;
layout(set = 3, binding = 0) uniform sampler2D LUT;
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
    Light lights[2]; // 直接在片段着色器中使用 UBO 的灯光数据
} ubo;



vec3 hdrTosdr(vec3 envColor){
    // HDR -> LDR
    envColor = envColor / (envColor + vec3(1.0));
//    // Gamma校正（只在颜色为线性空间的渲染管线才需要）
//    envColor = pow(envColor, vec3(1.0/2.2));

    return envColor;
}

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

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}


vec3 getNormal(vec3 texNormal)
{
    vec3 tangentNormal = texNormal;

    vec3 q1 = dFdx(fragPos);
    vec3 q2 = dFdy(fragPos);
    vec2 st1 = dFdx(fragTexCoord);
    vec2 st2 = dFdy(fragTexCoord);

    float det = st1.x * st2.y - st1.y * st2.x;

    if (abs(det) < 0.0001) {
        return normalize(fragNormal);
    }

    vec3 T = normalize((q1 * st2.y - q2 * st1.y) / det);
    vec3 N = normalize(fragNormal);
    vec3 B = normalize(cross(N, T));

    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float linearToSrgb(float value) {
    if (value <= 0.0031308) {
        return value * 12.92;
    } else {
        return 1.055 * pow(value, 1.0 / 2.4) - 0.055;
    }
}

void main() {
    float maxMipLevel = float(textureQueryLevels(cubeMap)) - 1.0;

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
    float ao = linearToSrgb(texture(textureArray, vec3(fragTexCoord, 4.0f)).r);

    vec3 N = getNormal(tangentNormal);
    vec3 V = normalize(fragViewPos - fragPos);

    vec3 F0 = vec3(0.01);
    F0 = mix(F0, albedo, metallic);

    // 初始化直接光照贡献
    vec3 Lo = vec3(0.0);


    //IBL的diffuse Light （ambient）
    vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD = 1.0 - kS;
    vec3 irradiance = hdrTosdr(textureLod(cubeMap, N, maxMipLevel * 0.6).rgb);
    vec3 diffuse = (1 - metallic ) * irradiance * albedo;

    //Spec
    float lod = roughness * maxMipLevel;
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 prefilteredCoolor = hdrTosdr(textureLod(cubeMap, reflect(N,V), lod).rgb);
    vec2 envBRDF = vec2(linearToSrgb(texture(LUT, vec2(dot(N, V), roughness)).r),linearToSrgb(texture(LUT, vec2(dot(N, V), roughness)).g));
//    vec2 envBRDF = vec2(texture(LUT, vec2(0.5, 0.5)).r,texture(LUT, vec2(0.5, 0.5)).g);
    vec3 indirectSpec = prefilteredCoolor * (F * envBRDF.x + envBRDF.y);
    vec3 ambient = (kD * diffuse + indirectSpec) * ao;


    for (int i = 0; i < ubo.lightCount; i++) {
        vec3 L = normalize(ubo.lights[i].position - fragPos); // 使用数组索引访问灯光位置
        vec3 H = normalize(V + L);

        float distance = length(ubo.lights[i].position - fragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 lightColor = ubo.lights[i].color; // 使用数组索引访问灯光颜色，去掉 2.0 系数
        vec3 radiance = lightColor * attenuation;

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
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 color = ambient + emissive + Lo;

    outColor = vec4(color, 1.0);
//    outColor = vec4(envBRDF, 0.0,1.0);
}