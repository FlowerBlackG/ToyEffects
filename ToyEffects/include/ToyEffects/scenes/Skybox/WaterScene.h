
#pragma once

#include <ToyGraph/Scene/Scene.h>
#include <ToyGraph/Skybox.h>
#include <ToyGraph/Camera.h>
#include <ToyGraph/Actor.h>

class WaterScene : public Scene {
public:
	//变量
	int cnt;		//暂定
	//函数
    ~WaterScene();

    static WaterScene* constructor() {
        return new WaterScene;
    }

    virtual void render() override;
    virtual void tick(float deltaT) override;

    WaterScene();

    void cursorPosCallback(double xPos, double yPos) override;

    void activeKeyInputProcessor(GLFWwindow* window, float deltaT) override;

    bool InitScene();
	void UninitScene();
	//void GenerateLODLevels(OpenGLAttributeRange** subsettable, unsigned int* numsubsets, uint32_t* idata);
	unsigned int GenerateBoundaryMesh(int deg_left, int deg_top, int deg_right, int deg_bottom, int levelsize, uint32_t* idata);
    float Phillips(const glm::vec2& k, const glm::vec2& w, float V, float A);

    void RenderWater(); 
    void CreateStrip(int hVertices, int ​vVertices, float size);
    void calculateAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
        unsigned int vLength, unsigned int normalOffset, int hVertices);
    Skybox* pSkybox = nullptr;

    //画个cube
    Shader cube{
        "shaders/cube.vs",
        "shaders/cube.fs"
    };

    std::vector<Mesh*> meshList;
    std::vector<Shader> shaderList;

    glm::mat4 projection;
};

