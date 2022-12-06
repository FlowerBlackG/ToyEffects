#pragma once
#include <ToyGraph/Scene/Scene.h>
#include <ToyGraph/Skybox.h>
#include <ToyGraph/Camera.h>
#include <ToyGraph/Actor.h>

class VCloudScene : public Scene {
public:
    GLuint VBO, VAO;
    //our main full size framebuffer

    //setup noise textures
    GLuint curltex, worltex, perlworltex, weathertex;


    ~VCloudScene();

    static VCloudScene* constructor() {
        return new VCloudScene;
    }

    virtual void render() override;
    virtual void tick(float deltaT) override;

    VCloudScene();

    void cursorPosCallback(double xPos, double yPos) override;

    void activeKeyInputProcessor(GLFWwindow* window, float deltaT) override;

    Skybox* pSkybox = nullptr;

    Shader skyShader{
        "shaders/VolumeCloud/sky.vert",
        "shaders/VolumeCloud/sky.frag"
    };

    Shader postShader{
        "shaders/VolumeCloud/tex.vert",
        "shaders/VolumeCloud/tex.frag"
    };

    const GLuint downscale = 1; //4 is best//any more and the gains dont make up for the lag
    GLuint downscalesq = downscale * downscale;
   // glm::mat4 MVPM;
  // glm::mat4 LFMVPM;
    int check;


    Model* paimonModel = nullptr;
    Shader paimonShader{
        "shaders/shader.vert",
        "shaders/shader.frag"
    };
    GLfloat cloud_density=0.5;
};


