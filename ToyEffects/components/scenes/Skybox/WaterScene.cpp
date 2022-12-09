#include <ToyEffects/scenes/Skybox/WaterScene.h>
#include <ToyGraph/Scene/SceneManager.h>
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

void WaterMesh::CreateMesh(GLfloat* water_vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices)
{
	indexCount = numOfIndices;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * numOfIndices, indices, GL_STATIC_DRAW);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(indices[0]) * numOfVertices, water_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(water_vertices[0]) * 8, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(water_vertices[0]) * 8, (void*)(sizeof(water_vertices[0]) * 3));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(water_vertices[0]) * 8, (void*)(sizeof(water_vertices[0]) * 5));
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

void WaterScene::cursorPosCallback(double xPos, double yPos) {
	__nahidaPaimonSharedCursorPosCallback(xPos, yPos);
}

void WaterScene::activeKeyInputProcessor(GLFWwindow* window, float deltaTime) {
	__nahidaPaimonSharedActiveKeyInputProcessor(window, deltaTime);

}

void WaterScene::bindCloudShader()
{
	//init a square
	GLfloat vertices[] = {
			 -1.0f, -1.0f,
	   -1.0f,  3.0f,
		3.0f, -1.0f,
	};
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
}
void  WaterScene::initGUI()
{
	auto& app = AppRuntime::getInstance();
	vcgui = new GUI(this, app.getWindow());
	//enable mouse
	glfwSetInputMode(app.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
void WaterScene::initCloud()
{
	if (skyShader.errcode != ShaderError::SHADER_OK) {
		cout << "VCloud:sky shader err: " << skyShader.errmsg << endl;
	}

	if (postShader.errcode != ShaderError::SHADER_OK) {
		cout << "VCloud:post shader err: " << postShader.errmsg << endl;
	}

	int x, y, n;
	unsigned char* curlNoiseArray = stbi_load("assets/VolumeCloud/curlNoise_1.png", &x, &y, &n, 0);
	//curl噪声
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
	//  create GUI 

}

void WaterScene::initWater()
{
	auto& app = AppRuntime::getInstance();
	//生成水纹
	int hVert = 128;
	int vVert = 90;
	createStrip(hVert, vVert, 0.5f);

	//纹理
	WaterTexture* waterTexture0 = new WaterTexture;
	WaterTexture* waterTexture1 = new WaterTexture;
	WaterTexture* waterTexture2 = new WaterTexture;
	WaterTexture* waterTexture3 = new WaterTexture;
	waterTexture0->setfileLocation((char*)("textures/water.png"), PNG_RGBA);
	waterTexture1->setfileLocation((char*)("textures/water_tranverse1.png"), JPG_RGB);
	waterTexture2->setfileLocation((char*)("textures/water_tranverse2.png"), JPG_RGB);
	waterTexture3->setfileLocation((char*)("textures/water_tranverse3.png"), PNG_RGBA);
	waterTexture0->LoadTexture();
	waterTexture1->LoadTexture();
	waterTexture2->LoadTexture();
	waterTexture3->LoadTexture();
	waterTexture.emplace_back(waterTexture0);
	waterTexture.emplace_back(waterTexture1);
	waterTexture.emplace_back(waterTexture2);
	waterTexture.emplace_back(waterTexture3);
	//材质
	//waterMaterial = Material(1.0f, 64);
	//光照 color:白 漫反射参数0.7 光源方向0.5*3
	//mainLight = Light(1.0f, 1.0f, 1.0f, 0.7f, 0.5, 0.5f, 0.5f, 1.0f);
	//mainLight = Light(1.0f, 1.0f, 1.0f, 0.7f, -5.5f, -0.5f, -0.5f, 1.0f);

	projection = glm::perspective(
		glm::radians(camera->getFov()),
		1.0f * app.getWindowWidth() / app.getWindowHeight(),
		0.1f,
		100.0f
	);
	//WaterShader ocean;
	//ocean.read("../shaders/ocean/ocean.vs", "../shaders/ocean/ocean.fs", "../shaders/ocean/ocean.geom");
	//shaderList.push_back(ocean);

}

void WaterScene::initModel()
{
	// 准备派蒙。
	Actor* paimon = new Actor;
	paimon->setScale(glm::vec3(0.2));
	paimon->setPosition(glm::vec3(0.0f,2.0f,0.0f));
	this->addActor(paimon);

	paimonModel = new Model("assets/genshin-impact/paimon/paimon.pmx");
	paimon->bindModel(paimonModel);

	if (paimonShader.errcode != ShaderError::SHADER_OK) {
		cout << "paimon shader err: " << paimonShader.errmsg << endl;
	}
}

WaterScene::WaterScene() {
	auto& app = AppRuntime::getInstance();
	//天空盒
	vector<string> skyboxFaces({
		"assets/SkyBox/right3.jpg",
		"assets/SkyBox/left3.jpg",
		"assets/SkyBox/top3.jpg",
		"assets/SkyBox/bottom3.jpg",
		"assets/SkyBox/front3.jpg",
		"assets/SkyBox/back3.jpg"
		});
	pSkybox = new Skybox(skyboxFaces);
	//摄像机
	camera = SceneManager::getInstance().currentScene()->camera;
	camera->setPosition(glm::vec3(0, 3, 3));
	camera->setYaw(-84.0f);
	camera->setPitch(23.8f);
	//初始化云
	initGUI();
	bindCloudShader();
	initCloud();
	//初始化水
	initWater();
	initModel();

}

WaterScene::~WaterScene() {
	if (this->pSkybox) {
		delete this->pSkybox;
		this->pSkybox = nullptr;
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteTextures(1, &(waterTexture[0]->id));
	glDeleteTextures(1, &(waterTexture[1]->id));
	glDeleteTextures(1, &(waterTexture[2]->id));
	glDeleteTextures(1, &(waterTexture[3]->id));
	glDeleteTextures(1, &perlworltex);
	glDeleteTextures(1, &worltex);
	glDeleteTextures(1, &curltex);
	glDeleteTextures(1, &weathertex);
	if (vcgui)
		delete vcgui;

}

void WaterScene::calculateAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* water_vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset, int hVertices)
{
	int counter = 0;
	for (size_t i = 0; i < indiceCount; i++)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(water_vertices[in1] - water_vertices[in0], water_vertices[in1 + 1] - water_vertices[in0 + 1], water_vertices[in1 + 2] - water_vertices[in0 + 2]);
		glm::vec3 v2(water_vertices[in2] - water_vertices[in0], water_vertices[in2 + 1] - water_vertices[in0 + 1], water_vertices[in2 + 2] - water_vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		water_vertices[in0] += normal.x; water_vertices[in0 + 1] += normal.y; water_vertices[in0 + 2] += normal.z;
		water_vertices[in1] += normal.x; water_vertices[in1 + 1] += normal.y; water_vertices[in1 + 2] += normal.z;
		water_vertices[in2] += normal.x; water_vertices[in2 + 1] += normal.y; water_vertices[in2 + 2] += normal.z;

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
		glm::vec3 vec(water_vertices[nOffset], water_vertices[nOffset + 1], water_vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		water_vertices[nOffset] = vec.x; water_vertices[nOffset + 1] = vec.y; water_vertices[nOffset + 2] = vec.z;
	}
}

void WaterScene::createStrip(int hVertices, int ​vVertices, float size)
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

	//GLfloat water_vertices[] = {
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

	GLfloat* water_vertices = new GLfloat[hVertices * ​vVertices * 8];
	int xMultiplier = 0;
	int yMultiplier = 0;
	float uIncrement = 0.0f;
	float vIncrement = 1.0f;
	for (int i = 0; i < hVertices * ​vVertices * 8; i += 8)
	{
		water_vertices[i] = xMultiplier * size;				//x
		water_vertices[i + 1] = yMultiplier * (-size);		//y
		water_vertices[i + 2] = 0;							//z
		water_vertices[i + 3] = uIncrement;					//u
		water_vertices[i + 4] = vIncrement;					//v
		water_vertices[i + 5] = 0.0f;							//normalX
		water_vertices[i + 6] = 0.0f;							//normalY
		water_vertices[i + 7] = 0.0f;							//normalZ

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

	calculateAverageNormals(indices, 2 * hVertices * (​vVertices - 1) + 2 * (​vVertices - 2), water_vertices, hVertices * ​vVertices * 8, 8, 5, hVertices);

	WaterMesh* obj1 = new WaterMesh;// (GL_TRIANGLE_STRIP);
	obj1->CreateMesh(water_vertices, indices, hVertices * ​vVertices * 8, 2 * hVertices * (​vVertices - 1) + 2 * (​vVertices - 2));
	waterMesh = obj1;
	//meshList.push_back(obj1);
}

//时刻改变位置，这里不需要
void WaterScene::tick(float deltaT) {
	printf("%5.2f fps\r", 1 / deltaT);
}
void WaterScene::render() {
	
	renderCloud();

	renderWater();

}
void WaterScene::setGUI()
{
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Scene average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SliderFloat("cloud type", &cloud_density, 0.3f, 1.0f);
	ImGui::ColorEdit3("cloud color", (float*)&(color_style));
	ImGui::SliderFloat("cloud type", &timespeed, 20.0f, 100.0f);
};
void WaterScene::renderCloud()
{
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


	//由于云的frag深度是1，天空盒深度也是1，这里不用深度测试，而是开启混合模式，先画天空盒再画云
	//https://learnopengl-cn.readthedocs.io/zh/latest/04%20Advanced%20OpenGL/03%20Blending/
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);     //开透明度混合模式
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//混合function

	pSkybox->render();
	skyShader.use();

	GLfloat timePassed = glfwGetTime();
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
	
	glDepthFunc(GL_LEQUAL);

	//draw screen
	paimonShader.use();
	paimonShader.setMatrix4fv("projection", projection)
		.setMatrix4fv("view", view);
	for (auto it : this->actors) {

		paimonShader.setMatrix4fv("model", it.second->getModelMatrix());
		it.second->render(&paimonShader);
	}

	//render gui
	vcgui->render();
}


void WaterScene::renderWater()
{

	watershader.use();
	watershader.setFloat("uvScroll", glfwGetTime() / 5);
	watershader.setVector3f("directionalLight.colour", 1.0f, 1.0f, 1.0f);
	watershader.setFloat("directionalLight.ambientIntensity", 0.7f);
	watershader.setVector3f("directionalLight.direction", -5.5f, -0.5f, -0.5f);
	watershader.setFloat("directionalLight.diffuseIntensity", 1.0f);
	watershader.setFloat("material.specularIntensity", 1.0f);
	watershader.setFloat("material.shininess", 64);
	watershader.setMatrix4fv("projection", projection);
	watershader.setMatrix4fv("view", camera->getViewMatrix());
	watershader.setVector3f("eyePosition", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
	glm::vec3 position = water_pos+glm::vec3(10.0f, -15.0f, 7.0f);
	glm::mat4 model = glm::mat4(1.0);

	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 5.0f));
	//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
	watershader.setMatrix4fv("model", model);
	waterTexture[0]->UseTexture();
	waterMesh->RenderMesh();

	const int offset = 88.5;
	glm::vec3 position2 = glm::vec3(position.x, position.y, position.z + offset);
	model = glm::mat4(1.0);
	model = glm::translate(model, position2);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, z.0f));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 5.0f));
	watershader.setMatrix4fv("model", model);
	waterTexture[1]->UseTexture();
	waterMesh->RenderMesh();

	//第2部分
	position = water_pos + glm::vec3(10 - 126.5, -15.0f, 7.0f);
	model = glm::mat4(1.0);
	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 5.0f));
	//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));

	watershader.setMatrix4fv("model", model);
	waterTexture[2]->UseTexture();
	waterMesh->RenderMesh();

	position2 = glm::vec3(position.x, position.y, position.z + offset);
	model = glm::mat4(1.0);
	model = glm::translate(model, position2);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, z.0f));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 5.0f));
	watershader.setMatrix4fv("model", model);
	waterTexture[3]->UseTexture();
	waterMesh->RenderMesh();

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
