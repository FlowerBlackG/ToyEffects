
#pragma once

#include <ToyGraph/Scene/Scene.h>
#include <ToyGraph/Skybox.h>
#include <ToyGraph/Camera.h>
#include <ToyGraph/Actor.h>

class NahidaScene : public Scene {
public:
    ~NahidaScene();

    static NahidaScene* constructor() {
        return new NahidaScene;
    }

    virtual void render() override;
    virtual void tick(float deltaT) override;

    NahidaScene();

    void cursorPosCallback(double xPos, double yPos) override;

    void activeKeyInputProcessor(GLFWwindow* window, float deltaT) override;

    Skybox* pSkybox = nullptr;
    Model* nahidaModel = nullptr;
    Shader nahidaShader{
        "shaders/shader.vert",
        "shaders/shader.frag"
    };

    Shader screenShader{
        "shaders/postprocess-screen-shader.vert",
        "shaders/postprocess-screen-shader.frag"
    };

    GLuint fbo = 0;
    GLuint scrQuadVao;
    GLuint scrQuadVbo;
    GLuint textureColorbuffer;
    
    int postProcessEffect = 0;
    const int N_EFFECTS = 6;
};


