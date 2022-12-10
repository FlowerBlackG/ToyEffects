#include <iostream>
#include <ToyEffects/scenes/Skybox/VCloudScene.h>
#include <ToyEffects/scenes/Skybox/shared.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include <stb_image.h>

using namespace std;



void VCloudScene::cursorPosCallback(double xPos, double yPos) {
    __nahidaPaimonSharedCursorPosCallback(xPos, yPos);
}

void VCloudScene::activeKeyInputProcessor(GLFWwindow* window, float deltaTime) {
    __nahidaPaimonSharedActiveKeyInputProcessor(window, deltaTime);

}


VCloudScene::~VCloudScene() {
    if (this->pSkybox) {
        delete this->pSkybox;
        this->pSkybox = nullptr;
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glDeleteTextures(1, &perlworltex);
    glDeleteTextures(1, &worltex);
    glDeleteTextures(1, &curltex);
    glDeleteTextures(1, &weathertex);

    if (vcgui)
        delete vcgui;
    
}

void VCloudScene::tick(float deltaT) {
    //cout <<setw(5)<< 1/deltaT<<" fps\r";
}


void VCloudScene::render() {
    auto& runtime = AppRuntime::getInstance();

    int WIDTH = runtime.getWindowWidth();
    int HEIGHT = runtime.getWindowHeight();


    auto view = camera->getViewMatrix();
    auto projection = glm::perspective(
        glm::radians(camera->getFov()),
        1.0f * runtime.getWindowWidth() / runtime.getWindowHeight(),
        0.1f,
        100.0f
    );

   //云层厚度减到50%，光线步进的步长增大两倍，采样深度除以2——平视时fps大概在20左右，仰视100+
   //删去3了个不必要的framebuffer，速度提升了4-5倍，fps在100-800不等
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
   
    //draw screen
    paimonShader.use();
    paimonShader.setMatrix4fv("projection", projection)
        .setMatrix4fv("view", view);
    for (auto it : this->actors) { 

        paimonShader.setMatrix4fv("model", it.second->getModelMatrix());
        it.second->render(&paimonShader);
    }

    //由于云的frag深度是1，天空盒深度也是1，这里不用深度测试，而是开启混合模式，先画天空盒再画云
    //https://learnopengl-cn.readthedocs.io/zh/latest/04%20Advanced%20OpenGL/03%20Blending/
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);     //开透明度混合模式
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//混合function

    pSkybox->render();
    skyShader.use();

    GLfloat timePassed= glfwGetTime();
    skyShader.setFloat("time", timePassed);

    skyShader.setMatrix4fv("MVPM", projection * view);
    GLfloat ASPECT = float(WIDTH) / float(HEIGHT);
    skyShader.setFloat("aspect", ASPECT);
    skyShader.setVector3f("cameraPos", camera->getPosition());
    skyShader.setInt("check", 0);//已废除
    skyShader.setVector2f("resolution", WIDTH, HEIGHT);
    skyShader.setFloat("downscale", downscale);
    skyShader.setFloat("cloud_density", cloud_density);
    skyShader.setInt("perlworl", 0);
    skyShader.setInt("worl", 1);
    skyShader.setInt("curl", 2);
    skyShader.setInt("weather", 3);
    skyShader.setFloat("speed", timespeed);
    skyShader.setVector3f("color_style", color_style);

    //variables for preetham model
    const float PI = 3.141;
    float time_fixed = 1;
    float theta = PI * (-0.23 + 0.25 * sin(time_fixed * 0.1));
    float phi = 2 * PI * (-0.25);
    float sunposx = cos(phi);
    float sunposy = sin(phi) * sin(theta);
    float sunposz = sin(phi) * cos(theta);

    //glUniform3f(psunPosition, GLfloat(sunposx), GLfloat(sunposy), GLfloat(sunposz));
    skyShader.setVector3f("sunPosition", sunposx, sunposy, sunposz);
    

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, perlworltex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, worltex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, curltex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, weathertex);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    //开启深度测试、关闭混合模式
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    //render gui
    vcgui->render();
}


VCloudScene::VCloudScene() {

    //init camera
    this->camera = new Camera;
    camera->setPosition(glm::vec3(0, 0, 3));
    camera->setYaw(-84.0f);
    camera->setPitch(23.8f);
    //get appruntime
    auto& app = AppRuntime::getInstance();
    vector<string> skyboxFaces({
        "assets/SpaceboxCollection/Spacebox1/_left.png",
        "assets/SpaceboxCollection/Spacebox1/_right.png",
        "assets/SpaceboxCollection/Spacebox1/_top.png",
        "assets/SpaceboxCollection/Spacebox1/_bottom.png",
        "assets/SpaceboxCollection/Spacebox1/_front.png",
        "assets/SpaceboxCollection/Spacebox1/_back.png"
        });

    pSkybox = new Skybox(skyboxFaces);

    if (skyShader.errcode != ShaderError::SHADER_OK) {
        cout << "VCloud:sky shader err: " << skyShader.errmsg << endl;
    }

    if (postShader.errcode != ShaderError::SHADER_OK) {
        cout << "VCloud:post shader err: " << postShader.errmsg << endl;
    }
    
    //init a square
    GLfloat vertices[] = {
             -1.0f, -1.0f,
       -1.0f,  3.0f,
        3.0f, -1.0f,
    };

    glGenVertexArrays(1,&VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);



    //setup noise textures

    int x, y, n;
    unsigned char* curlNoiseArray = stbi_load("assets/VolumeCloud/curlNoise_1.png", &x, &y, &n, 0);
    //curl噪声
    glGenTextures(1, &curltex);
    glBindTexture(GL_TEXTURE_2D, curltex);
    glBindTexture(GL_TEXTURE_2D, curltex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, curlNoiseArray);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(curlNoiseArray);

    unsigned char* weatherNoiseArray = stbi_load("assets/VolumeCloud/weather.bmp", &x, &y, &n, 0);
    
    glGenTextures(1, &weathertex);
    glBindTexture(GL_TEXTURE_2D, weathertex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, weatherNoiseArray);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(weatherNoiseArray);
 
    unsigned char* worlNoiseArray = stbi_load("assets/VolumeCloud/worlnoise.bmp", &x, &y, &n, 0);
    glGenTextures(1, &worltex);
    glBindTexture(GL_TEXTURE_3D, worltex);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, 32, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, worlNoiseArray);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, 0);
    stbi_image_free(worlNoiseArray);

    unsigned char* perlWorlNoiseArray = stbi_load("assets/VolumeCloud/perlworlnoise.tga", &x, &y, &n, 4);
 
    glGenTextures(1, &perlworltex);
    glBindTexture(GL_TEXTURE_3D, perlworltex);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 128, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, perlWorlNoiseArray);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, 0);
    stbi_image_free(perlWorlNoiseArray);


     // 准备派蒙。
    Actor* paimon = new Actor;
    paimon->setScale(glm::vec3(0.2));
    this->addActor(paimon);

    paimonModel = new Model("assets/genshin-impact/paimon/paimon.pmx");
    paimon->bindModel(paimonModel);

    if (paimonShader.errcode != ShaderError::SHADER_OK) {
        cout << "paimon shader err: " << paimonShader.errmsg << endl;
    }

    //  create GUI 
    vcgui = new GUI(this,app.getWindow());
    //enable mouse
    glfwSetInputMode(app.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
void VCloudScene::setGUI() 
{
    ImGui::TextColored(ImVec4(1, 1, 0, 1),"Scene average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::SliderFloat("cloud type", &cloud_density, 0.3f, 1.0f);
    ImGui::ColorEdit3("cloud color", (float*)&(color_style));
    ImGui::SliderFloat("cloud type", &timespeed, 20.0f, 100.0f);
};