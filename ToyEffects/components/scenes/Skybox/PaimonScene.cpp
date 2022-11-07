
#include <ToyEffects/scenes/Skybox/PaimonScene.h>
#include <ToyEffects/scenes/Skybox/shared.h>
#include <iostream>
using namespace std;

void PaimonScene::cursorPosCallback(double xPos, double yPos) {
    __nahidaPaimonSharedCursorPosCallback(xPos, yPos);
}

void PaimonScene::activeKeyInputProcessor(GLFWwindow* window, float deltaTime) {
    __nahidaPaimonSharedActiveKeyInputProcessor(window, deltaTime);
}


PaimonScene::~PaimonScene() {
    if (this->pSkybox) {
        delete this->pSkybox;
    }

    if (this->paimonModel) {
        delete this->paimonModel;
    }
}

void PaimonScene::tick(float deltaT) {
    auto paimon = this->actors[0];
    paimon->setYaw(paimon->getYaw() + deltaT * 20); // 让派蒙转起来~
}

void PaimonScene::render() {
    auto& runtime = AppRuntime::getInstance();

    // 绘制派蒙。
    paimonShader.use();
    
    auto view = camera->getViewMatrix();
    auto projection = glm::perspective(
        glm::radians(camera->getFov()), 
        1.0f * runtime.getWindowWidth() / runtime.getWindowHeight(), 
        0.1f, 
        100.0f
    );
    
    
    paimonShader.setMatrix4fv("projection", projection)
        .setMatrix4fv("view", view);


    for (auto it : this->actors) { // 实际只有一个 actor，即派蒙。
        
        paimonShader.setMatrix4fv("model", it.second->getModelMatrix());
        it.second->render(&paimonShader);
    }

    // 绘制天空盒。
    pSkybox->render();
}

PaimonScene::PaimonScene() {
    // 准备天空盒。
    vector<string> skyboxFaces({
        "assets/SpaceboxCollection/Spacebox6/SkyBlue2_right1.png",
        "assets/SpaceboxCollection/Spacebox6/SkyBlue2_left2.png",
        "assets/SpaceboxCollection/Spacebox6/SkyBlue2_top3.png",
        "assets/SpaceboxCollection/Spacebox6/SkyBlue2_bottom4.png",
        "assets/SpaceboxCollection/Spacebox6/SkyBlue2_front5.png",
        "assets/SpaceboxCollection/Spacebox6/SkyBlue2_back6.png"
    });

    pSkybox = new Skybox(skyboxFaces);
    this->camera = new Camera;
    camera->setPosition(glm::vec3(-10, 0, 0));

    // 准备派蒙。
    Actor* paimon = new Actor;
    paimon->setScale(glm::vec3(0.2));
    this->addActor(paimon);

    paimonModel = new Model("assets/genshin-impact/paimon/paimon.pmx");
    paimon->bindModel(paimonModel);

    if (paimonShader.errcode != ShaderError::SHADER_OK) {
        cout << "paimon shader err: " << paimonShader.errmsg << endl;
    }

}
