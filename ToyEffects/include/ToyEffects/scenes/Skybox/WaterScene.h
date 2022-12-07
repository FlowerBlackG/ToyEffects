
#pragma once

#include <ToyGraph/Scene/Scene.h>
#include <ToyGraph/Skybox.h>
#include <ToyGraph/Camera.h>
#include <ToyGraph/Actor.h>
#include <stb_image.h>

class WaterScene : public Scene {
public:
	//初始化和删除
	WaterScene();
	~WaterScene();
	static WaterScene* constructor() {
		return new WaterScene;
	}
	virtual void render() override;
	virtual void tick(float deltaT) override;
	void cursorPosCallback(double xPos, double yPos) override;
	void activeKeyInputProcessor(GLFWwindow* window, float deltaT) override;

	//水函数
	void initWater();
	void renderWater();
	void createStrip(int hVertices, int ​vVertices, float size);
	void calculateAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
		unsigned int vLength, unsigned int normalOffset, int hVertices);
	//云函数
	void bindCloudShader();
	void initCloud();
	void renderCloud();
	//模型
	void initModel();
	//天空盒
	Skybox* pSkybox = nullptr;
	//shader
	Shader skyShader{
	"shaders/VolumeCloud/sky.vert",
	"shaders/VolumeCloud/sky.frag"
	};
	Shader postShader{
		"shaders/VolumeCloud/tex.vert",
		"shaders/VolumeCloud/tex.frag"
	};
	Shader paimonShader{
	"shaders/shader.vert",
	"shaders/shader.frag"
	};
	//模型
	Model* paimonModel = nullptr;

	//水变量
	glm::mat4 projection;

	//体积云变量
	GLuint VBO, VAO;
	//setup noise textures
	GLuint curltex, worltex, perlworltex, weathertex;
	const GLuint downscale = 1; //4 is best//any more and the gains dont make up for the lag
	GLuint downscalesq = downscale * downscale;
	int check;
	GLfloat cloud_density = 0.5;

};

class WaterMesh :Mesh {
protected:
	unsigned int indexCount;


public:
	WaterMesh();
	void CreateMesh(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices);
	void RenderMesh();


};

class WaterShader :Shader
{
public:
	void read(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, const std::string& geometryShaderFilePath);
	void useWater();
	GLuint getId();
};

class WaterTexture {

	int width, height, bitDepth;
	char* fileLocation;
	/**
	 * 纹理类型。如：specular, diffuse.
	 */
	TextureType type;
	int RGB_type;

public:
	GLuint id;
	void setfileLocation(char* s, int RGBtype);
	void LoadTexture();
	void UseTexture();
	WaterTexture();
};