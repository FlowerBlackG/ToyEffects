#include <ToyEffects/scenes/Skybox/NahidaScene.h>
#include <ToyEffects/scenes/Skybox/shared.h>
#include <iostream>
using namespace std;

static const float SCR_QUAD_VERTICES[] = {
    -1, 1, 0, 1,
    -1, -1, 0, 0,
    1, -1, 1, 0,

    -1, 1, 0, 1,
    1, -1, 1, 0,
    1, 1, 1, 1
};

void NahidaScene::cursorPosCallback(double xPos, double yPos) {
    __nahidaPaimonSharedCursorPosCallback(xPos, yPos);
}

void NahidaScene::activeKeyInputProcessor(GLFWwindow* window, float deltaTime) {
    __nahidaPaimonSharedActiveKeyInputProcessor(window, deltaTime);

    auto& runtime = AppRuntime::getInstance();

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS 
        && runtime.lastFrameKeyStatus[GLFW_KEY_LEFT] == GLFW_RELEASE
    ) {
        this->postProcessEffect --;
        if (this->postProcessEffect < 0) {
            this->postProcessEffect = this->N_EFFECTS - 1;
        }

        
        cout << "effect: " << this->postProcessEffect << endl;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS 
        && runtime.lastFrameKeyStatus[GLFW_KEY_RIGHT] == GLFW_RELEASE
    ) {
        this->postProcessEffect ++;
        if (this->postProcessEffect >= this->N_EFFECTS) {
            this->postProcessEffect = 0;
        }

        
        cout << "effect: " << this->postProcessEffect << endl;
    }

    
}


NahidaScene::~NahidaScene() {
    if (this->pSkybox) {
        delete this->pSkybox;
        this->pSkybox = nullptr;
    }

    if (this->nahidaModel) {
        delete this->nahidaModel;
        this->nahidaModel = nullptr;
    }

    glDeleteFramebuffers(1, &fbo);
}

void NahidaScene::tick(float deltaT) {
    auto nahida = this->actors[0];
    nahida->setYaw(nahida->getYaw() + deltaT * 20);
}


void NahidaScene::render() {
    auto& runtime = AppRuntime::getInstance();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // todo: ToyGraph 封装应该调整以适配后处理模式。

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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);

    screenShader.use();
    screenShader.setInt("effect", this->postProcessEffect);
    glBindVertexArray(scrQuadVao);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_DEPTH_TEST);
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
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (2 * sizeof(float)));

    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    auto& app = AppRuntime::getInstance();

    // framebuffer
    // todo: 窗口尺寸改变时，重绘。
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, app.getWindowWidth(), app.getWindowHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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




