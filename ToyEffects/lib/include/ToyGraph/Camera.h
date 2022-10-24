/*
 * 相机封装类。
 *
 * 创建时间：2022年9月20日 于上海市嘉定区安亭镇
 */

#pragma once
#include "ToyGraphCommon/EngineCompileOptions.h"
#include <ToyGraph/Actor.h>


/**
 * 相机。
 */
class Camera : public Actor {
public:


public: // setters and getters.
    Camera& setFov(float fov);
    float getFov();

    Camera& setMovementSpeed(float movementSpeed);
    float getMovementSpeed();

public: // 工具方法。

    /**
     * 基于欧拉角度，计算获取 view 矩阵。
     * 
     * @return 计算得到的视角矩阵。
     */
    glm::mat4 getViewMatrix();

protected: // 基本数据成员。

    /**
     * field of view. 相机视角。
     */
    float fov = 45.0f;

    float movementSpeed = 1.0f;

};
