
#pragma once

#include <ToyGraph/Scene/Scene.h>
#include <ToyGraph/Skybox.h>
#include <ToyGraph/Camera.h>
#include <ToyGraph/Actor.h>

class PaimonScene : public Scene {
public:
    ~PaimonScene();

    static PaimonScene* constructor() {
        return new PaimonScene;
    }

    virtual void render() override;
    virtual void tick(float deltaT) override;

    PaimonScene();

    void cursorPosCallback(double xPos, double yPos) override;

    void activeKeyInputProcessor(GLFWwindow* window, float deltaT) override;

    Skybox* pSkybox = nullptr;
    Model* paimonModel = nullptr;
    Shader paimonShader{
        "shaders/shader.vert",
        "shaders/shader.frag"
    };
    
};
