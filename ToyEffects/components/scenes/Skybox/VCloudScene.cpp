#include <iostream>
#include <ToyEffects/scenes/Skybox/VCloudScene.h>
#include <ToyEffects/scenes/Skybox/shared.h>

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
    glDeleteBuffers(1, &fbo);
    glDeleteBuffers(1, &copyfbo);
    glDeleteBuffers(1, &subbuffer);
    glDeleteTextures(1, &fbotex);
    glDeleteTextures(1, &copyfbotex);
    glDeleteTextures(1, &subbuffertex);
    glDeleteTextures(1, &perlworltex);
    glDeleteTextures(1, &worltex);
    glDeleteTextures(1, &curltex);
    glDeleteTextures(1, &weathertex);

}

void VCloudScene::tick(float deltaT) {

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
    glBindFramebuffer(GL_FRAMEBUFFER, subbuffer);
    glViewport(0, 0, WIDTH / downscale, HEIGHT / downscale);

    skyShader.use();


    //glUniform1f(timeu, timePassed);
    GLfloat timePassed= glfwGetTime();
    skyShader.setFloat("time", timePassed);
    //glUniformMatrix4fv(uniformMatrix, 1, GL_FALSE, glm::value_ptr(MVPM));

    skyShader.setMatrix4fv("MVPM", projection * view);
    //glUniform1f(aspectUniform, ASPECT);
    GLfloat ASPECT = float(WIDTH) / float(HEIGHT);
    skyShader.setFloat("aspect", ASPECT);

    //glUniform1i(checku, (check)%(downscalesq));
    skyShader.setInt("check", (check) % (downscalesq));
    //glUniform2f(resolutionu, GLfloat(WIDTH), GLfloat(HEIGHT));
    skyShader.setVector2f("resolution", WIDTH, HEIGHT);
    //glUniform1f(downscaleu, GLfloat(downscale));
    skyShader.setFloat("downscale", downscale);

    //glUniform1i(perlworluniform, 0);
    skyShader.setInt("perlworl", 0);
    //glUniform1i(worluniform, 1);
    skyShader.setInt("worl", 1);
    //glUniform1i(curluniform, 2);
    skyShader.setInt("curl", 2);
    //glUniform1i(weatheruniform, 3);
    skyShader.setInt("weather", 3);

    //variables for preetham model
    const float PI = 3.141592653589793238462643383279502884197169;
    float theta = PI * (-0.23 + 0.25 * sin(timePassed * 0.1));
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


    //upscale the buffer into full size framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);


    glViewport(0, 0, WIDTH, HEIGHT);//???
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //upscaleShader.use();
    ////glUniformMatrix4fv(upLFuniformMatrix, 1, GL_FALSE, glm::value_ptr(LFMVPM));
    //upscaleShader.setMat4("LFMVPM", LFMVPM);
    ////glUniformMatrix4fv(upuniformMatrix, 1, GL_FALSE, glm::value_ptr(MVPM));
    //upscaleShader.setMat4("MVPM", MVPM);
    ///*glUniform1i(upcheck, (check)%downscalesq);
    //glUniform2f(upresolution, GLfloat(WIDTH), GLfloat(HEIGHT));
    //glUniform1f(updownscale, GLfloat(downscale));
    //glUniform1f(upaspect, ASPECT);*/

    //upscaleShader.setInt("check", (check) % (downscalesq));//这个是什么？
    //upscaleShader.setVec2("resolution", WIDTH, HEIGHT);
    //upscaleShader.setFloat("downscale", downscale);
    //upscaleShader.setFloat("aspect", ASPECT);

    ///*upscaleShader.setInt("perlworl", 0);
    //upscaleShader.setInt("worl", 1);
    //upscaleShader.setInt("curl", 2);
    //upscaleShader.setInt("weather", 3);*/

    ////glUniform1i(buffuniform, 0);
    ////upscaleShader.setFloat("buff",0);
    //upscaleShader.setInt("buff",0);
    ////glUniform1i(ponguniform, 1);
    ////upscaleShader.setFloat("pong", 0);
    //upscaleShader.setInt("pong", 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, subbuffertex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, copyfbotex);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);


    //copy the full size buffer so it can be read from next frame
    glBindFramebuffer(GL_FRAMEBUFFER, copyfbo);

    postShader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fbotex);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);


    //copy to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    postShader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fbotex);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    check++;


}


VCloudScene::VCloudScene() {
    //TODO:add skybox

    //init camera
    this->camera = new Camera;
    camera->setPosition(glm::vec3(0, 0, 3));
    camera->setYaw(-84.0f);
    camera->setPitch(23.8f);
    //get appruntime
    auto& app = AppRuntime::getInstance();

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

    //our main full size framebuffer
    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &fbotex);
    glBindTexture(GL_TEXTURE_2D, fbotex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, app.getWindowWidth(), app.getWindowHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbotex, 0);

    //our secondary full size framebuffer for copying and reading from the main framebuffer


    glGenFramebuffers(1, &copyfbo);
    glGenTextures(1, &copyfbotex);
    glBindTexture(GL_TEXTURE_2D, copyfbotex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, app.getWindowWidth(), app.getWindowHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, copyfbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, copyfbotex, 0);


    //our downscaled buffer that we actually render to

    glGenFramebuffers(1, &subbuffer);
    glGenTextures(1, &subbuffertex);
    glBindTexture(GL_TEXTURE_2D, subbuffertex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, app.getWindowWidth() / downscale, app.getWindowHeight() / downscale, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, subbuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, subbuffertex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    //setup noise textures

    int x, y, n;
    unsigned char* curlNoiseArray = stbi_load("assets/VolumeCloud/curlNoise_1.png", &x, &y, &n, 0);

    glGenTextures(1, &curltex);
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

    check = 0;
}




