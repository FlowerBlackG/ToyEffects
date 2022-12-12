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

    float near_distance = 10.0; // ��ƽ���ģ��˥����Χ
    float far_distance = 10.0; // Զƽ���ģ��˥����Χ

    float near_plane = -12.0; // ��ƽ��
    float far_plane = -20.0; // Զƽ��

    int postProcessEffect = 0;
    const int N_EFFECTS = 6;

    //GUI���
    GUI* vcgui = nullptr;
    virtual void setGUI() override;
    void initGUI();

};