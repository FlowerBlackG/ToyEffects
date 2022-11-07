
#pragma once

#include <ToyGraph/Scene/Scene.h>
#include <ToyGraph/Skybox.h>
#include <ToyGraph/Camera.h>
#include <ToyGraph/Actor.h>

class MainScene : public Scene {
public:
    ~MainScene();

    static MainScene* constructor() {
        return new MainScene;
    }

    virtual void render() override;

    MainScene();

    void cursorPosCallback(double xPos, double yPos) override;

    void activeKeyInputProcessor(GLFWwindow* window, float deltaT) override;

    Skybox* pSkybox = nullptr;

};
