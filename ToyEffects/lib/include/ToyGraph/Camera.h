/*
 * �����װ�ࡣ
 *
 * ����ʱ�䣺2022��9��20�� ���Ϻ��мζ�����ͤ��
 */

#pragma once
#include "ToyGraphCommon/EngineCompileOptions.h"
#include <ToyGraph/Actor.h>


/**
 * �����
 */
class Camera : public Actor {
public:


public: // setters and getters.
    Camera& setFov(float fov);
    float getFov();

    Camera& setMovementSpeed(float movementSpeed);
    float getMovementSpeed();

public: // ���߷�����

    /**
     * ����ŷ���Ƕȣ������ȡ view ����
     * 
     * @return ����õ����ӽǾ���
     */
    glm::mat4 getViewMatrix();

protected: // �������ݳ�Ա��

    /**
     * field of view. ����ӽǡ�
     */
    float fov = 45.0f;

    float movementSpeed = 1.0f;

};
