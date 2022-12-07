﻿#include <ToyEffects/scenes/Skybox/WaterScene.h>
#include <ToyGraph/Scene/SceneManager.h>
#include <ToyGraph/Material.h>
#include <ToyGraph/Light.h>
//#include <ToyGraph/shader.h>
//#include <ToyGraph/model/Mesh.h>
//#include <ToyGraph/model/Texture.h>
#include <ToyEffects/scenes/Skybox/shared.h>
#include <iostream>
#include <random>
#include <complex>

//暂时
#include <fstream>
#include <sstream>
using namespace std;

#define PNG_RGBA 0
#define JPG_RGB 1


////fft数学常量
//static const float PI = 3.141592f;
//static const float ONE_OVER_SQRT_2 = 0.7071067f;	//根号2分之1
//static const float ONE_OVER_SQRT_TWO_PI = 0.39894228f;
////先写在全局变量，创建water类了再封装
//unsigned int				initial = 0;			// 初始光谱,h0(共轭)
//unsigned int				frequencies = 0;		// 频率 w_i 每个波向量
//unsigned int				updated[2] = { 0 };		// updated spectra h~(k,t)和D~(k,t)
//unsigned int				tempdata = 0;			// 中间变量 FT
//unsigned int				displacement = 0;		// 位移图
//unsigned int				gradients = 0;			// 法线折叠贴图
//uint32_t					numlods = 0;
//unsigned int				perlintex = 0;		// Perlin 噪声 to remove tiling artifacts
//unsigned int				environment = 0;
//unsigned int				debugvao = 0;
//unsigned int				helptext = 0;


//改完fft再一并移入类
WaterTexture waterTexture;
WaterTexture waterTexture1;
WaterTexture waterTexture2;
WaterTexture waterTexture3;
Material waterMaterial;
Light mainLight;
std::vector<WaterMesh*> meshList;
std::vector<WaterShader> shaderList;

GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformUvScroll = 0, uniformEyePosition = 0,
uniformAmbientIntensity = 0, uniformAmbientColor = 0, uniformDirection = 0, uniformDiffuseIntensity = 0,
uniformSpecularIntensity = 0, uniformShininess = 0;


//把wn标准化
void Vec2Normalize(glm::vec2& out, glm::vec2& v);
//求向量长度，可替换库
float Vec2Length(const glm::vec2& v);
//求log2x的整数
uint32_t Log2OfPow2(uint32_t x);

GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

WaterMesh::WaterMesh()
{
	indexCount = 0;
}

void WaterMesh::CreateMesh(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices)
{
	indexCount = numOfIndices;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * numOfIndices, indices, GL_STATIC_DRAW);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(indices[0]) * numOfVertices, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, (void*)(sizeof(vertices[0]) * 3));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, (void*)(sizeof(vertices[0]) * 5));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void WaterMesh::RenderMesh()
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

WaterTexture::WaterTexture()
{
	id = 0;
	RGB_type = PNG_RGBA;
	type = TextureType::SPECULAR;
	width = height = bitDepth = 0;
	fileLocation = nullptr;
}
void WaterTexture::setfileLocation(char* s, int RGBtype)
{
	fileLocation = s;
	RGB_type = RGBtype;
}

void WaterTexture::LoadTexture()
{
	unsigned char* texData = stbi_load(fileLocation, &width, &height, &bitDepth, 0);
	if (!texData)
	{
		printf("Could not find: %s\n", fileLocation);
		return;
	}

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//使用png贴图
	if (RGB_type == PNG_RGBA)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
	else if (RGB_type == JPG_RGB)
		//使用jpg贴图
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);

	glCheckError();
	glGenerateMipmap(GL_TEXTURE_2D);


	//std::cout << glGetError() << std::endl; // 返回 0 (无错误)

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(texData);
}

void WaterTexture::UseTexture()
{
	//加
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);
}

//实际没有用几何
void WaterShader::read(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, const std::string& geometryShaderFilePath)
{
	//cout << vertexShaderFilePath;
	ifstream vShaderFile(vertexShaderFilePath, ios::binary);
	ifstream fShaderFile(fragmentShaderFilePath, ios::binary);
	ifstream gShaderFile(geometryShaderFilePath, ios::binary);

	if (!vShaderFile.is_open()) {
		errmsg = "failed to open vertex shader file.";
		errcode = ShaderError::V_SHADER_OPEN_FAILED;
		return;
	}

	if (!fShaderFile.is_open()) {
		errmsg = "failed to open fragment shader file.";
		errcode = ShaderError::F_SHADER_OPEN_FAILED;
		return;
	}
	if (!gShaderFile.is_open()) {
		errmsg = "failed to open geometry shader file.";
		errcode = ShaderError::F_SHADER_OPEN_FAILED;
		return;
	}
	stringstream vShaderStream;
	stringstream fShaderStream;
	stringstream gShaderStream;
	vShaderStream << vShaderFile.rdbuf();
	fShaderStream << fShaderFile.rdbuf();
	gShaderStream << gShaderFile.rdbuf();

	string stdVShaderCode = vShaderStream.str();
	string stdFShaderCode = fShaderStream.str();
	string stdGShaderCode = gShaderStream.str();
	const char* vShaderCode = stdVShaderCode.c_str();
	const char* fShaderCode = stdFShaderCode.c_str();
	const char* gShaderCode = stdGShaderCode.c_str();

	GLuint vertexId;
	GLuint fragmentId;
	GLuint geoId;
	int success;
	char infoLog[512];

	vertexId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexId, 1, &vShaderCode, nullptr);
	glCompileShader(vertexId);
	glGetShaderiv(vertexId, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexId, sizeof(infoLog) / sizeof(char), nullptr, infoLog);
		errcode = ShaderError::V_SHADER_COMPILE_FAILED;
		errmsg = "failed to compile vertex shader. ";
		errmsg += infoLog;
		return;
	}

	fragmentId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentId, 1, &fShaderCode, nullptr);
	glCompileShader(fragmentId);
	glGetShaderiv(fragmentId, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentId, sizeof(infoLog), nullptr, infoLog);
		errcode = ShaderError::F_SHADER_COMPILE_FAILED;
		errmsg = "failed to compile fragment shader. ";
		errmsg += infoLog;
		return;
	}

	//geoId = glCreateShader(GL_GEOMETRY_SHADER);
	//glShaderSource(geoId, 1, &gShaderCode, nullptr);
	//glCompileShader(geoId);
	//glGetShaderiv(geoId, GL_COMPILE_STATUS, &success);
	//if (!success) {
	//	glGetShaderInfoLog(geoId, sizeof(infoLog), nullptr, infoLog);
	//	errcode = ShaderError::F_SHADER_COMPILE_FAILED;
	//	errmsg = "failed to compile geometry shader. ";
	//	errmsg += infoLog;
	//	return;
	//}

	this->id = glCreateProgram();
	glAttachShader(id, vertexId);
	glAttachShader(id, fragmentId);
	//	glAttachShader(id, geoId);
	glLinkProgram(id);
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(id, sizeof(infoLog), nullptr, infoLog);
		errcode = ShaderError::LINKING_FAILED;
		errmsg = "failed to link program. ";
		errmsg += infoLog;
		return;
	}

	glValidateProgram(id);
	glGetProgramiv(id, GL_VALIDATE_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(id, sizeof(infoLog), nullptr, infoLog);
		printf("Program Validation FAILED: '%s'\n", infoLog);
		return;
	}

	uniformModel = glGetUniformLocation(id, "model");
	uniformProjection = glGetUniformLocation(id, "projection");
	uniformView = glGetUniformLocation(id, "view");
	uniformAmbientColor = glGetUniformLocation(id, "directionalLight.colour");
	uniformAmbientIntensity = glGetUniformLocation(id, "directionalLight.ambientIntensity");
	uniformDirection = glGetUniformLocation(id, "directionalLight.direction");
	uniformDiffuseIntensity = glGetUniformLocation(id, "directionalLight.diffuseIntensity");
	uniformSpecularIntensity = glGetUniformLocation(id, "material.specularIntensity");
	uniformShininess = glGetUniformLocation(id, "material.shininess");
	uniformEyePosition = glGetUniformLocation(id, "eyePosition");

	uniformUvScroll = glGetUniformLocation(id, "uvScroll");



	//glDeleteShader(vertexId);
	//glDeleteShader(fragmentId);
	//	glDeleteShader(geoId);

	vShaderFile.close();
	fShaderFile.close();
	gShaderFile.close();

	this->errcode = ShaderError::SHADER_OK;

}

void WaterShader::useWater()
{
	glUseProgram(id);
}
GLuint WaterShader::getId()
{
	return id;
}
void WaterScene::cursorPosCallback(double xPos, double yPos) {
	__nahidaPaimonSharedCursorPosCallback(xPos, yPos);
}

void WaterScene::activeKeyInputProcessor(GLFWwindow* window, float deltaTime) {
	__nahidaPaimonSharedActiveKeyInputProcessor(window, deltaTime);

}

WaterScene::~WaterScene() {
	if (this->pSkybox) {
		delete this->pSkybox;
		this->pSkybox = nullptr;
	}
	glDeleteTextures(1, &(waterTexture.id));
	glDeleteTextures(1, &(waterTexture1.id));
	glDeleteTextures(1, &(waterTexture2.id));
	glDeleteTextures(1, &(waterTexture3.id));

}
void WaterScene::calculateAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset, int hVertices)
{
	int counter = 0;
	for (size_t i = 0; i < indiceCount; i++)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;

		if (counter == 2 * hVertices - 3)
		{
			counter = 0;
			i += 4;
			continue;
		}

		counter++;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}

void WaterScene::CreateStrip(int hVertices, int ​vVertices, float size)
{
	/*GLuint indices[] = {0, 4, 1, 5, 2, 6, 3, 7,
						7, 4,
						4, 8, 5, 9, 6, 10, 7, 11,
						11, 8,
						8, 12, 9, 13, 10, 14, 11, 15};*/

	GLuint* indices = new GLuint[2 * hVertices * (​vVertices - 1) + 2 * (​vVertices - 2)];

	//Setting the odd numbered indices
	int number = 0;
	int hIndex = 0;
	for (int i = 0; i < 2 * hVertices * (​vVertices - 1) + 2 * (​vVertices - 2); i += 2)
	{
		indices[i] = number;
		number++;

		if (hIndex == 2 * hVertices - 2)
		{
			i += 2;
			hIndex = 0;
			continue;
		}

		hIndex += 2;
	}

	//Setting the even numbered indices
	number = hVertices;
	hIndex = 1;
	for (int i = 1; i < 2 * hVertices * (​vVertices - 1) + 2 * (​vVertices - 2); i += 2)
	{
		indices[i] = number;
		number++;

		if (hIndex == 2 * hVertices - 1)
		{
			i += 2;
			hIndex = 1;
			continue;
		}

		hIndex += 2;
	}

	//Setting the connecting indices
	number = 0;
	for (int i = hVertices * 2; i < 2 * hVertices * (​vVertices - 1) + 2 * (​vVertices - 2); i += hVertices * 2 + 2)
	{
		indices[i] = hVertices * 2 - 1 + (number * hVertices);
		indices[i + 1] = hVertices + (number * hVertices);
		number++;
	}

	//GLfloat vertices[] = {
	//	//x			y			z			u		v			normalX		normalY		normalZ
	//	0.0f,		0.0f,		0.0f,		0.0f,	1.0f,		0.0f,		0.0f,		0.0f,
	//	size,		0.0f,		0.0f,		0.33f,	1.0f,		0.0f,		0.0f,		0.0f,
	//	2 * size,	0.0f,		0.0f,		0.66f,	1.0f,		0.0f,		0.0f,		0.0f,
	//	3 * size,	0.0f,		0.0f,		1.0f,	1.0f,		0.0f,		0.0f,		0.0f,

	//	0.0f,		-size,		0.0f,		0.0f,	0.66f,		0.0f,		0.0f,		0.0f,
	//	size,		-size,		0.0f,		0.33f,	0.66f,		0.0f,		0.0f,		0.0f,
	//	2 * size,	-size,		0.0f,		0.66f,	0.66f,		0.0f,		0.0f,		0.0f,
	//	3 * size,	-size,		0.0f,		1.0f,	0.66f,		0.0f,		0.0f,		0.0f,

	//	0.0f,		-2 * size,	0.0f,		0.0f,	0.33f,		0.0f,		0.0f,		0.0f,
	//	size,		-2 * size,	0.0f,		0.33f,	0.33f,		0.0f,		0.0f,		0.0f,
	//	2 * size,	-2 * size,	0.0f,		0.66f,	0.33f,		0.0f,		0.0f,		0.0f,
	//	3 * size,	-2 * size,	0.0f,		1.0f,	0.33f,		0.0f,		0.0f,		0.0f,

	//	0.0f,		-3 * size,	0.0f,		0.0f,	0.0f,		0.0f,		0.0f,		0.0f,
	//	size,		-3 * size,	0.0f,		0.33f,	0.0f,		0.0f,		0.0f,		0.0f,
	//	2 * size,	-3 * size,	0.0f,		0.66f,	0.0f,		0.0f,		0.0f,		0.0f,
	//	3 * size,	-3 * size,	0.0f,		1.0f,	0.0f,		0.0f,		0.0f,		0.0f
	//};

	GLfloat* vertices = new GLfloat[hVertices * ​vVertices * 8];
	int xMultiplier = 0;
	int yMultiplier = 0;
	float uIncrement = 0.0f;
	float vIncrement = 1.0f;
	for (int i = 0; i < hVertices * ​vVertices * 8; i += 8)
	{
		vertices[i] = xMultiplier * size;				//x
		vertices[i + 1] = yMultiplier * (-size);		//y
		vertices[i + 2] = 0;							//z
		vertices[i + 3] = uIncrement;					//u
		vertices[i + 4] = vIncrement;					//v
		vertices[i + 5] = 0.0f;							//normalX
		vertices[i + 6] = 0.0f;							//normalY
		vertices[i + 7] = 0.0f;							//normalZ

		xMultiplier++;
		uIncrement += (1.0f / (hVertices - 1));

		if (xMultiplier == hVertices)
		{
			xMultiplier = 0;
			yMultiplier++;
			uIncrement = 0.0f;
			vIncrement -= (1.0f / (​vVertices - 1));
		}
	}

	calculateAverageNormals(indices, 2 * hVertices * (​vVertices - 1) + 2 * (​vVertices - 2), vertices, hVertices * ​vVertices * 8, 8, 5, hVertices);

	WaterMesh* obj1 = new WaterMesh;// (GL_TRIANGLE_STRIP);
	obj1->CreateMesh(vertices, indices, hVertices * ​vVertices * 8, 2 * hVertices * (​vVertices - 1) + 2 * (​vVertices - 2));
	meshList.push_back(obj1);
}

//时刻改变位置，这里不需要
void WaterScene::tick(float deltaT) {

	//auto cube1 = this->actors[0];
	//cube1->setYaw(cube1->getYaw() + deltaT * 20);
}


void WaterScene::render() {
	auto& runtime = AppRuntime::getInstance();

	pSkybox->render();
	RenderWater();

	//for (auto it : this->actors) {
	//    it.second->render(&shaderList[0]);
	//}


}

WaterScene::WaterScene() {
	auto& runtime = AppRuntime::getInstance();

	projection = glm::mat4(1.0f);
	vector<string> skyboxFaces({
		"assets/SpaceboxCollection/Spacebox3/LightGreen_right1.png",
		"assets/SpaceboxCollection/Spacebox3/LightGreen_left2.png",
		"assets/SpaceboxCollection/Spacebox3/LightGreen_top3.png",
		"assets/SpaceboxCollection/Spacebox3/LightGreen_bottom4.png",
		"assets/SpaceboxCollection/Spacebox3/LightGreen_front5.png",
		"assets/SpaceboxCollection/Spacebox3/LightGreen_back6.png"
		});

	pSkybox = new Skybox(skyboxFaces);

	camera = SceneManager::getInstance().currentScene()->camera;

	camera->setPosition(glm::vec3(0, 3, 3));
	camera->setYaw(-84.0f);
	camera->setPitch(23.8f);

	//camera->setPosition(glm::vec3(0.0f, 3.0f, 0.0f));


	int hVert = 128;
	int vVert = 90;

	CreateStrip(hVert, vVert, 0.5f);

	WaterShader ocean;
	ocean.read("../shaders/ocean/ocean.vs", "../shaders/ocean/ocean.fs", "../shaders/ocean/ocean.geom");

	shaderList.push_back(ocean);

	waterTexture.setfileLocation((char*)("textures/water.png"), PNG_RGBA);
	waterTexture1.setfileLocation((char*)("textures/water_tranverse1.png"), JPG_RGB);
	waterTexture2.setfileLocation((char*)("textures/water_tranverse2.png"), JPG_RGB);
	waterTexture3.setfileLocation((char*)("textures/water_tranverse3.png"), PNG_RGBA);
	waterTexture.LoadTexture();
	waterTexture1.LoadTexture();
	waterTexture2.LoadTexture();
	waterTexture3.LoadTexture();

	waterMaterial = Material(1.0f, 64);
	//color:白
	//漫反射参数0.7
	//光源方向0.5*3
	//mainLight = Light(1.0f, 1.0f, 1.0f, 0.7f, 0.5, 0.5f, 0.5f, 1.0f);
	mainLight = Light(1.0f, 1.0f, 1.0f, 0.7f, -5.5f, -0.5f, -0.5f, 1.0f);

	projection = glm::perspective(
		glm::radians(camera->getFov()),
		1.0f * runtime.getWindowWidth() / runtime.getWindowHeight(),
		0.1f,
		100.0f
	);

}

void WaterScene::draw_half_water(glm::vec3 position)
{
	;
}
void WaterScene::RenderWater()
{

	shaderList[0].useWater();

	WaterShader first = shaderList[0];
	glUseProgram(first.getId());
	glUniform1f(uniformUvScroll, glfwGetTime() / 5);

	mainLight.UseLight(uniformAmbientIntensity, uniformAmbientColor, uniformDiffuseIntensity, uniformDirection);

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera->getViewMatrix()));
	glUniform3f(uniformEyePosition, camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);

	//cout << "camera:" << camera->getPosition().x << ' ' << camera->getPosition().y << ' ' << camera->getPosition().z << endl;
	draw_half_water(glm::vec3(10.0f, -25.0f, 7.0f));
	draw_half_water(glm::vec3(10 - 126.5, -25.0f, 7.0f));

	glm::vec3 position = glm::vec3(10.0f, -25.0f, 7.0f);
	glm::mat4 model = glm::mat4(1.0);

	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 5.0f));
	//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));

	//位置，需要加载的矩阵数，列优先矩阵，指向数组的指针
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	waterTexture.UseTexture();
	waterMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();

	const int offset = 88.5;
	glm::vec3 position2 = glm::vec3(position.x, position.y, position.z + offset);
	model = glm::mat4(1.0);
	model = glm::translate(model, position2);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, z.0f));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 5.0f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	waterTexture1.UseTexture();
	waterMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();

	//第2部分
	position = glm::vec3(10 - 126.5, -25.0f, 7.0f);
	model = glm::mat4(1.0);

	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 5.0f));
	//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));

	//位置，需要加载的矩阵数，列优先矩阵，指向数组的指针
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	waterTexture2.UseTexture();
	waterMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();

	position2 = glm::vec3(position.x, position.y, position.z + offset);
	model = glm::mat4(1.0);
	model = glm::translate(model, position2);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, z.0f));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 5.0f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	waterTexture3.UseTexture();
	waterMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();

	glUseProgram(0);
}

//等用于fft
void Vec2Normalize(glm::vec2& out, glm::vec2& v)
{
	float il = 1.0f / sqrtf(v.x * v.x + v.y * v.y);

	out[0] = v[0] * il;
	out[1] = v[1] * il;
}

//求向量长度，可替换库
float Vec2Length(const glm::vec2& v)
{
	return sqrtf(v.x * v.x + v.y * v.y);
}
//求log2x的整数
uint32_t Log2OfPow2(uint32_t x)
{
	uint32_t ret = 0;

	while (x >>= 1)
		++ret;

	return ret;
}
