
#pragma once

#include <ToyGraph/Scene/Scene.h>
#include <ToyGraph/Skybox.h>
#include <ToyGraph/Camera.h>
#include <ToyGraph/Actor.h>
#include <stb_image.h>

class WaterScene : public Scene {
public:

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


	void RenderWater();
	void CreateStrip(int hVertices, int ​vVertices, float size);
	void calculateAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
		unsigned int vLength, unsigned int normalOffset, int hVertices);
	void draw_half_water(glm::vec3 position);
	Skybox* pSkybox = nullptr;


	glm::mat4 projection;
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