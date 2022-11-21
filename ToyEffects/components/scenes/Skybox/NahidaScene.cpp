#include <ToyEffects/scenes/Skybox/NahidaScene.h>
#include <ToyEffects/scenes/Skybox/shared.h>
#include <iostream>
using namespace std;

void NahidaScene::cursorPosCallback(double xPos, double yPos) {
    __nahidaPaimonSharedCursorPosCallback(xPos, yPos);
}

void NahidaScene::activeKeyInputProcessor(GLFWwindow* window, float deltaTime) {
    __nahidaPaimonSharedActiveKeyInputProcessor(window, deltaTime);
}


NahidaScene::~NahidaScene() {
    if (this->pSkybox) {
        delete this->pSkybox;
    }

    if (this->nahidaModel) {
        delete this->nahidaModel;
    }
}

void NahidaScene::tick(float deltaT) {
    auto nahida = this->actors[0];
    nahida->setYaw(nahida->getYaw() + deltaT * 20);
}


void NahidaScene::render() {
    auto& runtime = AppRuntime::getInstance();

    nahidaShader.use();
    
    auto view = camera->getViewMatrix();
    auto projection = glm::perspective(
        glm::radians(camera->getFov()), 
        1.0f * runtime.getWindowWidth() / runtime.getWindowHeight(), 
        0.1f, 
        100.0f
    );
    
    
    nahidaShader.setMatrix4fv("projection", projection)
        .setMatrix4fv("view", view);


    for (auto it : this->actors) {
        glm::mat4 t= it.second->getModelMatrix();
        nahidaShader.setMatrix4fv("model", it.second->getModelMatrix());
        it.second->render(&nahidaShader);
    }

    pSkybox->render();
}


NahidaScene::NahidaScene() {
    vector<string> skyboxFaces({
        "assets/SpaceboxCollection/Spacebox3/LightGreen_right1.png",
        "assets/SpaceboxCollection/Spacebox3/LightGreen_left2.png",
        "assets/SpaceboxCollection/Spacebox3/LightGreen_top3.png",
        "assets/SpaceboxCollection/Spacebox3/LightGreen_bottom4.png",
        "assets/SpaceboxCollection/Spacebox3/LightGreen_front5.png",
        "assets/SpaceboxCollection/Spacebox3/LightGreen_back6.png"
    });

    pSkybox = new Skybox(skyboxFaces);
    this->camera = new Camera;
    camera->setPosition(glm::vec3(-10, 0, 0));

    Actor* nahida = new Actor;
    nahida->setScale(glm::vec3(0.2));
    this->addActor(nahida);

    nahidaModel = new Model("assets/genshin-impact/nahida/nahida.pmx");
    nahida->bindModel(nahidaModel);

    if (nahidaShader.errcode != ShaderError::SHADER_OK) {
        cout << "nahida shader err: " << nahidaShader.errmsg << endl;
    }

}




