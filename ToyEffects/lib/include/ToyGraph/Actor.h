/*
 * ��Ա�ࡣ
 * 
 * ����ʱ�䣺2022��9��20�� ���Ϻ��мζ�����ͤ��
 */

#pragma once
#include "ToyGraphCommon/EngineCompileOptions.h"

// glad & glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// stl
#include <functional>
#include <cmath>
#include <map>
#include <vector>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ToyGraph/Shader.h>


/**
 * ��Ա�ࡣ
 * 
 * ���п��Լ��뵽�����ڵ���Ʒ��Ӧ�̳��Ա��ࣨ������ӣ���
 * �����������ڣ�
 *   ģ�͡��ƹ⡢��Ƶ
 */
class Actor {

public: // �����������

    /**
     * ��Ա�๹�캯������ɳ�ʼ��������
     */
    Actor();

    /**
     * ��Ա����������������ڴ��ͷŵ���������
     */
    virtual ~Actor();

public: // ������

    /**
     * ÿ֡�����á����ڴ��� Actor �ı仯��
     * 
     * @param deltaSeconds ֡���ʱ�䡣��λΪ�롣
     */
    virtual void tick(float deltaSeconds) {
        for (auto it : children) {
            it->tick(deltaSeconds);
        }
    };
    
    virtual void render(class Shader* pShader = nullptr);

public: // setters and getters.

    /* -------- setters and getters. -------- */

    // position
    virtual Actor& setPosition(const glm::vec3& position);
    virtual glm::vec3& getPositionRef();
    virtual const glm::vec3& getPosition();

    // yaw
    virtual Actor& setYaw(const float yaw);
    virtual float getYaw();

    // pitch
    virtual Actor& setPitch(const float pitch);
    virtual float getPitch();

    // roll
    virtual Actor& setRoll(const float roll);
    virtual float getRoll();

    
    virtual const glm::vec3& getDirectionVectorUp();

    virtual const glm::vec3& getDirectionVectorRight();
   
    virtual const glm::vec3& getDirectionVectorFront();

    virtual Actor& setScale(const glm::vec3& scale);
    virtual const glm::vec3& getScale();

    void addChild(Actor* actor);
    void bindModel(class Model* model);
    void setShader(int shaderId);

public: // һ�㷽����
    virtual void move(float distance, const glm::vec3& direction);
    glm::mat4 getModelMatrix();

public: 
    int id = 0;

protected: // �Զ����߳�Ա���ⲿһ�㲻ֱ�Ӳ�����
    glm::vec3 directionVectorUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 directionVectorRight = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 directionVectorFront = glm::vec3(1.0f, 0.0f, 0.0f);

    bool directionVectorsIsLatest = false;

protected: // �Զ����߷������ⲿһ�㲻ֱ�ӵ��á�
    void updateDirectionVectors();

protected: // �ڲ���Ա��

    /*
        ��Ա�����Ǳ�Ҫ��Ҫֱ�Ӷ�д��
        �Ƽ�ʹ�� setter �� getter ��ɶ�д������
        setter �� getter �ڻ����У�鲽�衣

        ���������⣬����ֱ�Ӳ�����Ա�������ܺ�Σ�ա�
    */

    /**
     * �������ꡣ
     */
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

    /* ------------ ŷ���Ƕȡ� ------------ */

    /**
     * �����ǡ�
     */
    float pitch = 0.0f;

    /**
     * ƫ���ǡ�
     */
    float yaw = 0.0f;

    /**
     * �����ǡ�
     */
    float roll = 0.0f;

    /**
     * ���š�
     */
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

    std::vector<Actor*> children;
    Actor* parent = nullptr;
    class Model* pModel = nullptr;

    class Shader shader;

private: // ��ֹ���ݡ�

    /**
     * ���ƹ��졣��ֹʹ�á�
     */
    Actor(const Actor& actor) {};

};
