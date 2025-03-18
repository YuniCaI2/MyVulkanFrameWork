#ifndef LIGHT_H
#define LIGHT_H
#include <glm/glm.hpp>

enum class LightType {
    POINT,      // 点光源 (Dot/Point Light)
    DIRECTIONAL,// 方向光 (Directional Light)
    SPOT        // 聚光灯 (Spot Light)
};
namespace VK::Instances {
    class Light {
    public:
        // 构造函数
        Light() :
            position(0.0f, 0.0f, 0.0f),
            color(1.0f, 1.0f, 1.0f),
            type(LightType::POINT),
            direction(0.0f, 5.0f, 0.0f),
            spotAngle(glm::radians(45.0f)),
            spotFalloff(0.1f) {}

        // 公共属性
        glm::vec3 position;     // 光源位置 (所有光源类型都需要)
        glm::vec3 color;        // 光源颜色 (RGB)
        LightType type;         // 光源类型

        // 方向光和聚光灯专用属性
        glm::vec3 direction;    // 光照方向 (方向光和聚光灯使用)

        // 聚光灯专用属性
        float spotAngle;        // 聚光灯的锥角 (弧度)
        float spotFalloff;      // 聚光灯边缘衰减因子

        // 设置方法
        void setPosition(const glm::vec3& pos) { position = pos; }
        void setColor(const glm::vec3& col) { color = col; }
        void setType(LightType lightType) { type = lightType; }
        void setDirection(const glm::vec3& dir) { direction = glm::normalize(dir); }
        void setSpotAngle(float angle) { spotAngle = glm::radians(angle); }
        void setSpotFalloff(float falloff) { spotFalloff = falloff; }

        // 获取方法
        glm::vec3 getPosition() const { return position; }
        glm::vec3 getColor() const { return color; }
        LightType getType() const { return type; }
        glm::vec3 getDirection() const { return direction; }
        float getSpotAngle() const { return spotAngle; }
        float getSpotFalloff() const { return spotFalloff; }
    };
}

#endif // LIGHT_H