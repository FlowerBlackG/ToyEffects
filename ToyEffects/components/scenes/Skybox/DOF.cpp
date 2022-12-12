#include <ToyEffects/scenes/Skybox/DOF.h>
#include <ToyEffects/scenes/Skybox/shared.h>
#include <iostream>
#include "../imgui/imgui.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
using namespace std;

static const float SCR_QUAD_VERTICES[] = {
    -1, 1, 0, 1,
    -1, -1, 0, 0,
    1, -1, 1, 0,

    -1, 1, 0, 1,
    1, -1, 1, 0,
    1, 1, 1, 1
};

void DOF::cursorPosCallback(double xPos, double yPos) {
    __nahidaPaimonSharedCursorPosCallback(xPos, yPos);
}

void DOF::activeKeyInputProcessor(GLFWwindow* window, float deltaTime) {
    __nahidaPaimonSharedActiveKeyInputProcessor(window, deltaTime);

    auto& runtime = AppRuntime::getInstance();

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS
        && runtime.lastFrameKeyStatus[GLFW_KEY_LEFT] == GLFW_RELEASE
        ) {
        this->postProcessEffect--;
        if (this->postProcessEffect < 0) {
            this->postProcessEffect = this->N_EFFECTS - 1;
        }


        cout << "effect: " << this->postProcessEffect << endl;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS
        && runtime.lastFrameKeyStatus[GLFW_KEY_RIGHT] == GLFW_RELEASE
        ) {
        this->postProcessEffect++;
        if (this->postProcessEffect >= this->N_EFFECTS) {
            this->postProcessEffect = 0;
        }


        cout << "effect: " << this->postProcessEffect << endl;
    }


}


DOF::~DOF() {
    if (this->pSkybox) {
        delete this->pSkybox;
        this->pSkybox = nullptr;
    }

    if (this->nahidaModel) {
        delete this->nahidaModel;
        this->nahidaModel = nullptr;
    }

    glDeleteFramebuffers(1, &fbo);
    if (vcgui)
        delete vcgui;

}

void DOF::tick(float deltaT) {

    auto paimon = this->actors[0];
    paimon ->setYaw(paimon->getYaw() + deltaT * 20);

    auto nahida = this->actors[1];
    nahida->setYaw(nahida->getYaw() + deltaT * 20);
}


void DOF::render() {
    auto& runtime = AppRuntime::getInstance();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    // todo: ToyGraph 封装应该调整以适配后处理模式。

    characterShader.use();

    auto view = camera->getViewMatrix();
    auto projection = glm::perspective(
        glm::radians(camera->getFov()),
        1.0f * runtime.getWindowWidth() / runtime.getWindowHeight(),
        0.1f,
        100.0f
    );


    characterShader.setMatrix4fv("projection", projection)
        .setMatrix4fv("view", view);


    for (auto it : this->actors) {
        glm::mat4 t = it.second->getModelMatrix();
        characterShader.setMatrix4fv("model", it.second->getModelMatrix());
        it.second->render(&characterShader);
    }

    characterShader.setFloat("near_distance", this->near_distance);
    characterShader.setFloat("far_distance", this->far_distance);
    characterShader.setFloat("near_plane", this->near_plane);
    characterShader.setFloat("far_plane", this->far_plane);

    pSkybox->render();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);


    screenShader.use();
    glDisable(GL_DEPTH_TEST);
    screenShader.setVector2f("screenSize", runtime.getWindowWidth(), runtime.getWindowHeight());
    //screenShader.setInt("effect", this->postProcessEffect);
    glBindVertexArray(scrQuadVao);
    //glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //render gui
    vcgui->render();

}

void DOF::setGUI()
{
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Scene average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::SliderFloat("near_distance", &near_distance, 0.0f, 15.0f);
    ImGui::SliderFloat("far_distance", &far_distance, 0.0f, 15.0f);
    ImGui::SliderFloat("near_plane", &near_plane, -30.0f, -1.0f);
    ImGui::SliderFloat("far_plane", &far_plane, -35.0f, -7.0f);

}

void DOF::initGUI()
{
    auto& app = AppRuntime::getInstance();
    vcgui = new GUI(this, app.getWindow());
    //enable mouse
    //glfwSetInputMode(app.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

DOF::DOF() {

    
    vector<string> skyboxFaces({
        "assets/SpaceboxCollection/Spacebox5/Sky2_right1.png",
        "assets/SpaceboxCollection/Spacebox5/Sky2_left2.png",
        "assets/SpaceboxCollection/Spacebox5/Sky2_top3.png",
        "assets/SpaceboxCollection/Spacebox5/Sky2_bottom4.png",
        "assets/SpaceboxCollection/Spacebox5/Sky2_front5.png",
        "assets/SpaceboxCollection/Spacebox5/Sky2_back6.png"
        });

    pSkybox = new Skybox(skyboxFaces);
    this->camera = new Camera;
    camera->setPosition(glm::vec3(-10, 0, 0));

    //initGUI();

    // 准备派蒙。
    Actor* paimon = new Actor;
    paimon->setScale(glm::vec3(0.2));
    paimon->setPosition(glm::vec3(3.0f, 0.0f, 0.0f));
    this->addActor(paimon);

    paimonModel = new Model("assets/genshin-impact/paimon/paimon.pmx");
    paimon->bindModel(paimonModel);

    if (characterShader.errcode != ShaderError::SHADER_OK) {
        cout << "paimon shader err: " << characterShader.errmsg << endl;
    }

    //准备草神
    Actor* nahida = new Actor;
    nahida->setScale(glm::vec3(0.2));
    //nahida->setRoll(2.0f);
    nahida->setPosition(glm::vec3(9.0f, 0.0f, 0.0f));
    this->addActor(nahida);

    nahidaModel = new Model("assets/genshin-impact/nahida/nahida.pmx");
    nahida->bindModel(nahidaModel);

    if (characterShader.errcode != ShaderError::SHADER_OK) {
        cout << "nahida shader err: " << characterShader.errmsg << endl;
    }

    if (screenShader.errcode != ShaderError::SHADER_OK) {
        cout << "screen shader err: " << screenShader.errmsg << endl;
    }

    // screen quad
    glGenVertexArrays(1, &scrQuadVao);
    glGenBuffers(1, &scrQuadVbo);
    glBindVertexArray(scrQuadVao);
    glBindBuffer(GL_ARRAY_BUFFER, scrQuadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SCR_QUAD_VERTICES), SCR_QUAD_VERTICES, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    //screenShader.setInt("screenTexture", 0);

    auto& app = AppRuntime::getInstance();

    vcgui = new GUI(this, app.getWindow());
    //enable mouse
    glfwSetInputMode(app.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    
    screenShader.use();
    screenShader.setVector2f("screenSize", app.getWindowWidth(), app.getWindowHeight());
    // framebuffer
    // todo: 窗口尺寸改变时，重绘。
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, app.getWindowWidth(), app.getWindowHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, app.getWindowWidth(), app.getWindowHeight()); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



}




