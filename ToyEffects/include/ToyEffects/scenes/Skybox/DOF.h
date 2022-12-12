#pragma once

#include <ToyGraph/Scene/Scene.h>
#include <ToyGraph/Skybox.h>
#include <ToyGraph/Camera.h>
#include <ToyGraph/Actor.h>
#include <ToyGraph/GUI.h>

class DOF : public Scene {
public:
	~DOF();

	static DOF* constructor() {
		return new DOF;
	}

	virtual void render() override;
	virtual void tick(float deltaT) override;

	DOF();

	void cursorPosCallback(double xPos, double yPos) override;

	void activeKeyInputProcessor(GLFWwindow* window, float deltaT) override;

    Skybox* pSkybox = nullptr;
    Model* nahidaModel = nullptr;
    Model* paimonModel = nullptr;
    Shader characterShader{
        "shaders/DOFshader.vert",
        "shaders/DOFshader.frag"
    };

    Shader screenShader{
        "shaders/DOFshaderSecond.vert",
        "shaders/DOFshaderSecond.frag"
    };

    GLuint fbo = 0;
    GLuint scrQuadVao;
    GLuint scrQuadVbo;
    GLuint textureColorbuffer;

    float near_distance = 10.0; // 近平面的模糊衰减范围
    float far_distance = 10.0; // 远平面的模糊衰减范围

    float near_plane = -12.0; // 近平面
    float far_plane = -20.0; // 远平面

    int postProcessEffect = 0;
    const int N_EFFECTS = 6;

    //GUI相关
    GUI* vcgui = nullptr;
    virtual void setGUI() override;
    void initGUI();

};