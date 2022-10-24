/*
 * ToyGraph ���档
 *
 * ������2022��9��21��
 * �������Ϻ��мζ�����ͤ��
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

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ToyGraph �����
#include "ToyGraph/Camera.h"
#include "ToyGraph/Actor.h"
#include "ToyGraph/Shader.h"
#include "ToyGraph/AppRuntime.h"

#include "ToyGraph/Model/Texture.h"
#include "ToyGraph/Model/Mesh.h"
#include "ToyGraph/Model/Model.h"


/**
 * ToyGraph ͼ�������ܿ�������
 * ����ʽ����ģʽ��
 */
class Engine {

public:
    static Engine& getInstance();

public:
    Texture* getLoadedTexture(const std::string& filepath);

public:
    std::map<const std::string, Texture> texturesLoaded;

protected:
    static Engine engineInstance;

private:
    Engine() = default;
    Engine(const Engine&) = default;
    Engine& operator = (const Engine&) {};

};
