#include <ToyEffects/scenes/Skybox/WaterCloudShadow.h>
#include <ToyGraph/Scene/SceneManager.h>
//#include <ToyGraph/Material.h>
//#include <ToyGraph/Light.h>
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


//改完fft再一并移入类——现在就移进去，不然全局变量复制场景麻烦啊
//WaterTexture waterTexture;
//WaterTexture waterTexture1;
//WaterTexture waterTexture2;
//WaterTexture waterTexture3;
//Material waterMaterial;
//Light mainLight;
//std::vector<WaterMesh*> meshList;
//std::vector<WaterShader> shaderList;
//
//GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformUvScroll = 0, uniformEyePosition = 0,
//uniformAmbientIntensity = 0, uniformAmbientColor = 0, uniformDirection = 0, uniformDiffuseIntensity = 0,
//uniformSpecularIntensity = 0, uniformShininess = 0;


//把wn标准化
void Vec2Normalize(glm::vec2& out, glm::vec2& v);
//求向量长度，可替换库
float Vec2Length(const glm::vec2& v);
//求log2x的整数
uint32_t Log2OfPow2(uint32_t x);

//GLenum glCheckError_(const char* file, int line)
//{
//	GLenum errorCode;
//	while ((errorCode = glGetError()) != GL_NO_ERROR)
//	{
//		std::string error;
//		switch (errorCode)
//		{
//		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
//		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
//		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
//		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
//		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
//		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
//		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
//		}
//		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
//	}
//	return errorCode;
//}
//#define glCheckError() glCheckError_(__FILE__, __LINE__) 


WaterCloudShadowScene::WaterCloudShadowScene()
{
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
	camera->setPosition(glm::vec3(3.3f, -7.27f, 16.83f));
	camera->setYaw(-84.0f);
	camera->setPitch(10.8f);
	//初始化云
	initGUI();
	bindCloudShader();
	initCloud();
	//初始化水
	initWater();
	//初始化阴影
	initDirLightShadow();
	//初始化导入的模型
	initModel();

	initPostProcessKits();
}

static const float SCR_QUAD_VERTICES[] = {
	-1, 1, 0, 1,
	-1, -1, 0, 0,
	1, -1, 1, 0,

	-1, 1, 0, 1,
	1, -1, 1, 0,
	1, 1, 1, 1
};

void WaterCloudShadowScene::initPostProcessKits() {
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

WaterCloudShadowScene::~WaterCloudShadowScene()
{
	if (this->pSkybox) {
		delete this->pSkybox;
		this->pSkybox = nullptr;
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteTextures(1, &(waterTexture.id));
	glDeleteTextures(1, &(waterTexture1.id));
	glDeleteTextures(1, &(waterTexture2.id));
	glDeleteTextures(1, &(waterTexture3.id));
	glDeleteTextures(1, &perlworltex);
	glDeleteTextures(1, &worltex);
	glDeleteTextures(1, &curltex);
	glDeleteTextures(1, &weathertex);
	if (vcgui)
		delete vcgui;
}

void WaterCloudShadowScene::render()
{
	

	getDirLightShadowMap();//第一遍得到阴影图

	// 启用后效帧缓冲。
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pSkybox->render();//【移出】这个需要深度测试，且需要在画云之前，否则会把阴影场景弄没
	renderCloud();   //画云
	
	renderSceneWithShadows();//第二遍画阴影（云是没有阴影的）


	renderWater();    //画水

	glm::vec3 p=camera->getPosition();
	cout << "相机" << p.x << ' ' << p.y << ' ' << p.z << endl;
	// 将后效帧缓冲绘制到屏幕缓冲。
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);

	screenShader.use();
	screenShader.setInt("effect", this->postProcessEffect);
	glBindVertexArray(scrQuadVao);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glEnable(GL_DEPTH_TEST);

	vcgui->render();//【移出】控制云的gui最后渲染，无需判断也不会被水和场景挡住

}

void WaterCloudShadowScene::tick(float deltaT)
{
	// printf("%5.2f fps\r", 1 / deltaT);//输出FPS
}

void WaterCloudShadowScene::cursorPosCallback(double xPos, double yPos)
{
	__nahidaPaimonSharedCursorPosCallback(xPos, yPos);
}

void WaterCloudShadowScene::activeKeyInputProcessor(GLFWwindow* window, float deltaTime)
{
	__nahidaPaimonSharedActiveKeyInputProcessor(window, deltaTime);

	auto& runtime = AppRuntime::getInstance();
	// 后效切换。
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

void WaterCloudShadowScene::drawCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			 // bottom face
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 // top face
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void WaterCloudShadowScene::renderObjects( Shader& shader, const glm::mat4 projection, const glm::mat4 view)
{
	
	
	// floor
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f,-9.7f,0.0f));
	model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));
	shader.setMatrix4fv("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// wall
	//glDisable(GL_CULL_FACE);
	//model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(5.0f, -10.0f, 5.0f));
	//model = glm::scale(model, glm::vec3(0.1f,5.0f,5.1f));
	//shader.setMatrix4fv("model", model);
	//drawCube();
	//glEnable(GL_CULL_FACE);
	// cubes
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -10.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMatrix4fv("model", model);
	drawCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -10.0f, 0.0f));
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMatrix4fv("model", model);
	drawCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -10.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMatrix4fv("model", model);
	drawCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -10.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 10.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMatrix4fv("model", model);
	drawCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -10.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-10.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMatrix4fv("model", model);
	drawCube();

}

void WaterCloudShadowScene::renderModels(Shader& shader, const glm::mat4 projection, const glm::mat4 view)
{
	//models
	shader.setMatrix4fv("projection", projection)
		.setMatrix4fv("view", view);
	for (auto it : this->actors) {
		glm::mat4 curModelMat = it.second->getModelMatrix();
		
		//curModelMat = glm::translate(curModelMat, glm::vec3(0, static_cast<float>(sin(glfwGetTime() * 0.5) * 3.0),0 ));
		curModelMat = glm::translate(curModelMat, glm::vec3(0, 0, 0));

		shader.setMatrix4fv("model", curModelMat);
		it.second->render(&shader);
	}
}

GLuint WaterCloudShadowScene::load2DTexture(char const* path)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void WaterCloudShadowScene::initWater()
{
	auto& app = AppRuntime::getInstance();
	//生成水纹
	int hVert = 128;
	int vVert = 90;
	createStrip(hVert, vVert, 0.5f);

	//纹理
	waterTexture.setfileLocation((char*)("textures/water.png"), PNG_RGBA);
	waterTexture1.setfileLocation((char*)("textures/water_tranverse1.png"), JPG_RGB);
	waterTexture2.setfileLocation((char*)("textures/water_tranverse2.png"), JPG_RGB);
	waterTexture3.setfileLocation((char*)("textures/water_tranverse3.png"), PNG_RGBA);
	waterTexture.LoadTexture();
	waterTexture1.LoadTexture();
	waterTexture2.LoadTexture();
	waterTexture3.LoadTexture();
	//材质
	waterMaterial = Material(1.0f, 64);
	//光照 color:白 漫反射参数0.7 光源方向0.5*3
	//mainLight = Light(1.0f, 1.0f, 1.0f, 0.7f, 0.5, 0.5f, 0.5f, 1.0f);
	mainLight = Light(water_color_type.x, water_color_type.y, water_color_type.z, 0.8f, -5.5f, -0.5f, -0.5f, 1.0f);

	water_projection = glm::perspective(
		glm::radians(camera->getFov()),
		1.0f * app.getWindowWidth() / app.getWindowHeight(),
		0.1f,
		100.0f
	);
	//shader
	NewWaterShader ocean;
	ocean.read(this ,"../shaders/ocean/ocean.vert", "../shaders/ocean/ocean.frag", "../shaders/ocean/ocean.geom");
	shaderList.push_back(ocean);
}
void WaterCloudShadowScene::drawWaterBlock(glm::vec3 position, glm::vec3 block_size, int offset, int big_scale)
{
	offset /= big_scale;
	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, block_size);
	//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));

	//位置，需要加载的矩阵数，列优先矩阵，指向数组的指针
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	waterTexture.UseTexture();
	waterMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();
	//wave.UseTexture();
	//waterMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	//meshList[0]->RenderMesh();

	glm::vec3 position2 = glm::vec3(position.x, position.y, position.z + offset);
	model = glm::mat4(1.0);
	model = glm::translate(model, position2);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, block_size);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	waterTexture1.UseTexture();
	waterMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();

	////第2部分
	//本应该-128
	position = glm::vec3(position.x - 126.5 / big_scale, position.y, position.z);
	model = glm::mat4(1.0);

	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, block_size);
	//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));

	//位置，需要加载的矩阵数，列优先矩阵，指向数组的指针
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	waterTexture2.UseTexture();
	waterMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();
	//wave.UseTexture();
	//waterMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	//meshList[0]->RenderMesh();

	position2 = glm::vec3(position.x, position.y, position.z + offset);
	model = glm::mat4(1.0);
	model = glm::translate(model, position2);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, z.0f));
	model = glm::scale(model, block_size);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	waterTexture3.UseTexture();
	waterMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();
	//wave.UseTexture();
	//waterMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	//meshList[0]->RenderMesh();

}


void WaterCloudShadowScene::renderWater()
{
	mainLight.setLightColor(water_color_type);
	shaderList[0].useWater();

	NewWaterShader first = shaderList[0];
	glUseProgram(first.getId());
	glUniform1f(uniformUvScroll, glfwGetTime() / 10);

	mainLight.UseLight(uniformAmbientIntensity, uniformAmbientColor, uniformDiffuseIntensity, uniformDirection);

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(water_projection));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera->getViewMatrix()));
	glUniform3f(uniformEyePosition, camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);

	glm::vec3 position1 = water_pos + glm::vec3(0.0f, -15.0f, 0.0f);
	glm::vec3 position2 = water_pos + glm::vec3(0.0f, -15.0f, -88.0f);
	glm::vec3 position3 = water_pos + glm::vec3(0.0f, -15.0f, 88.0f);
	glm::vec3 position4 = water_pos + glm::vec3(-126.0f, -15.0f, 0.0f);
	glm::vec3 position5 = water_pos + glm::vec3(126.0f, -15.0f, 0.0f);
	glm::vec3 position6 = water_pos + glm::vec3(126.0f, -15.0f, -88.0f);
	glm::vec3 position7 = water_pos + glm::vec3(126.0f, -15.0f, 88.0f);
	glm::vec3 position8 = water_pos + glm::vec3(-126.0f, -15.0f, 88.0f);
	glm::vec3 position9 = water_pos + glm::vec3(-126.0f, -15.0f, -88.0f);
	glm::vec3 position10 = water_pos + glm::vec3(0.0f, -15.0f, -176.0f);
	glm::vec3 position11 = water_pos + glm::vec3(126.0f, -15.0f, -176.0f);
	glm::vec3 position12 = water_pos + glm::vec3(-126.0f, -15.0f, -176.0f);

	glm::vec3 block_size = glm::vec3(1.0f, 1.0f, 2.5f);
	int offset = 88.5;
	drawWaterBlock(position1, block_size, offset, 2);
	drawWaterBlock(position2, block_size, offset, 2);
	drawWaterBlock(position3, block_size, offset, 2);
	drawWaterBlock(position4, block_size, offset, 2);
	drawWaterBlock(position5, block_size, offset, 2);
	drawWaterBlock(position6, block_size, offset, 2);
	drawWaterBlock(position7, block_size, offset, 2);
	drawWaterBlock(position8, block_size, offset, 2);
	drawWaterBlock(position9, block_size, offset, 2);
	//不画了，地平线交接的问题不能通过铺远解决，反而降低性能
	//drawWaterBlock(position10, block_size, offset, 2);
	//drawWaterBlock(position11, block_size, offset, 2);
	//drawWaterBlock(position12, block_size, offset, 2);
	glUseProgram(0);
}

void WaterCloudShadowScene::createStrip(int hVertices, int ​vVertices, float size)
{

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
	meshList.push_back(obj1);
}

void WaterCloudShadowScene::calculateAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* water_vertices, unsigned int verticeCount, unsigned int vLength, unsigned int normalOffset, int hVertices)
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

void WaterCloudShadowScene::bindCloudShader()
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

void WaterCloudShadowScene::initCloud()
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

void WaterCloudShadowScene::renderCloud()
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

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	
	

	//由于云的frag深度是1，天空盒深度也是1，这里不用深度测试，而是开启混合模式，先画天空盒再画云
	//https://learnopengl-cn.readthedocs.io/zh/latest/04%20Advanced%20OpenGL/03%20Blending/
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);     //开透明度混合模式
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//混合function
	

	
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

	//画模型
	/*DirLightShader.use();
	DirLightShader.setMatrix4fv("projection", projection)
		.setMatrix4fv("view", view);
	for (auto it : this->actors) {
		DirLightShader.setMatrix4fv("model", it.second->getModelMatrix());
		it.second->render(&DirLightShader);
	}*/
	/*characterShader.use();
	characterShader.setMatrix4fv("projection", projection)
		.setMatrix4fv("view", view);
	for (auto it : this->actors) {
		characterShader.setMatrix4fv("model", it.second->getModelMatrix());
		it.second->render(&characterShader);
	}*/

	//render gui【移到最后】
	//vcgui->render();
}

void WaterCloudShadowScene::initDirLightShadow()
{
	if (DirLightShader.errcode != ShaderError::SHADER_OK) {
		std::cout << "WaterCloudShadow:DirLightShader shader err: " << DirLightShader.errmsg << std::endl;
	}

	if (simpleDepthShader.errcode != ShaderError::SHADER_OK) {
		std::cout << "WaterCloudShadow:simpleDepthShader shader err: " << simpleDepthShader.errmsg << std::endl;
	}

	if (lightSourceShader.errcode != ShaderError::SHADER_OK) {
		std::cout << "WaterCloudShadow:lightSourceShader shader err: " << lightSourceShader.errmsg << std::endl;
	}

	//地平面数据
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	woodTex = load2DTexture("assets/Shadow/wood1.jpg");

	//阴影图数据
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	glGenTextures(1, &depthMapTex);
	glBindTexture(GL_TEXTURE_2D, depthMapTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/
	//调整在深度地图范围之外的坐标都有1.0的深度，这意味着这些坐标永远不会在阴影中（但是光锥体外的需要在shader中判断）
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//shader配置
	DirLightShader.use();
	DirLightShader.setInt("diffuseTexture", 0);
	DirLightShader.setInt("shadowMap", 1);

	/*debugDepthQuad.use();
	debugDepthQuad.setInt("depthMap", 0);*/



}

void WaterCloudShadowScene::initPointLightShadow()
{

}

void WaterCloudShadowScene::renderSceneWithShadows()
{
	auto& runtime = AppRuntime::getInstance();

	int WIDTH = runtime.getWindowWidth();
	int HEIGHT = runtime.getWindowHeight();

	auto cameraPos = camera->getPosition();
	auto view = camera->getViewMatrix();
	auto projection = glm::perspective(
		glm::radians(camera->getFov()),
		1.0f * WIDTH / HEIGHT,
		0.1f,
		100.0f
	);

	/*第二个pass*/
	// --------------------------------------------------------------
	DirLightShader.use();
	DirLightShader.setMatrix4fv("projection", projection);
	DirLightShader.setMatrix4fv("view", view);
	// set light uniforms
	DirLightShader.setVector3f("viewPos", cameraPos);
	DirLightShader.setVector3f("lightPos", dirLight1_pos);
	DirLightShader.setFloat("lightWidth", dirLight1_width);
	DirLightShader.setInt("shadowType", dirLight1_shadowType);
	DirLightShader.setMatrix4fv("lightSpaceMatrix", lightSpaceMatrix);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, woodTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMapTex);

	renderObjects(DirLightShader, projection, view);
	renderModels(DirLightShader, projection, view);
	/*for (auto it : this->actors) {
		it.second->render(&DirLightShader);
	}*/
	//可视化光源
	//lightSourceShader.use();
	//lightSourceShader.setMatrix4fv("projection", projection);
	//lightSourceShader.setMatrix4fv("view", view);
	//glm::mat4 model = glm::mat4(1.0f);
	//model = glm::translate(model, dirLight1_pos);
	//model = glm::rotate(model, (float)glm::radians(glfwGetTime() * 100), glm::vec3(0.f, 1.f, 0.f));
	//model = glm::scale(model, glm::vec3(0.5f)); // 
	//lightSourceShader.setMatrix4fv("model", model);
	//drawCube();

	/*渲染深度图以debug*/
	/*debugDepthQuad.use();
	debugDepthQuad.setFloat("near_plane", dirLight1_nPlane);
	debugDepthQuad.setFloat("far_plane", dirLight1_fPlane);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMapTex);*/
	//renderQuad();



}

void WaterCloudShadowScene::getDirLightShadowMap()
{
	auto& runtime = AppRuntime::getInstance();

	int WIDTH = runtime.getWindowWidth();
	int HEIGHT = runtime.getWindowHeight();

	auto cameraPos = camera->getPosition();
	auto view = camera->getViewMatrix();
	auto projection = glm::perspective(
		glm::radians(camera->getFov()),
		1.0f * WIDTH / HEIGHT,
		0.1f,
		100.0f
	);




	//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*第一个pass*/
	// --------------------------------------------------------------
	//世界空间向量-->光源视角空间

	glm::mat4 lightModel = glm::mat4(1.0f);
	if(dirLight1_isMoving)
		lightModel = glm::rotate(lightModel, (float)glm::radians(glfwGetTime() * 30), glm::vec3(0.f, 1.f, 0.f));
	//glm::mat4 lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, dirLight1_nPlane, dirLight1_fPlane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
	glm::mat4 lightProjection 
		= glm::ortho(dirLight1_lPlane, dirLight1_rPlane, dirLight1_bPlane, dirLight1_tPlane, dirLight1_nPlane, dirLight1_fPlane);
	glm::mat4 lightView = glm::lookAt(dirLight1_pos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView * lightModel;

	//从光源的角度获取深度图
	simpleDepthShader.use();
	simpleDepthShader.setMatrix4fv("lightSpaceMatrix", lightSpaceMatrix);

	glCullFace(GL_FRONT);//用背面判断，防止阴影“悬浮”
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);//注意需要调成深度图的分辨率！！！
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, woodTex);


	renderObjects(simpleDepthShader, projection,view);
	renderModels(simpleDepthShader,projection,view);


	


	/*for (auto it : this->actors) {
		it.second->render(&simpleDepthShader);
	}*/

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK);

	// 重设视口并清空缓存
	glViewport(0, 0, WIDTH, HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	
}

void WaterCloudShadowScene::initModel()
{
	// 准备派蒙。
	Actor* paimon = new Actor;
	paimon->setScale(glm::vec3(0.2));
	paimon->setYaw(45.0f);
	paimon->setRoll(10.0f);
	paimon->setPosition(glm::vec3(0.0f, -10.0f, 0.5f));
	this->addActor(paimon);
	paimonModel = new Model("assets/genshin-impact/paimon/paimon.pmx");
	paimon->bindModel(paimonModel);

	//准备草神
	Actor* nahida = new Actor;
	nahida->setScale(glm::vec3(0.2));
	nahida->setYaw(-5.0f);
	nahida->setRoll(2.0f);
	nahida->setPosition(glm::vec3(2.0f, -10.0f, 2.0f));
	this->addActor(nahida);
	nahidaModel = new Model("assets/genshin-impact/nahida/nahida.pmx");
	nahida->bindModel(nahidaModel);

	//木头
	Actor* wood1 = new Actor;
	wood1->setScale(glm::vec3(0.3));
	wood1->setYaw(-50.0f);
	wood1->setRoll(2.0f);
	wood1->setPosition(glm::vec3(-2.0f, -10.31f, 6.0f));
	this->addActor(wood1);
	wood1Model = new Model("assets/SceneModels/wood1/Wood.obj");
	wood1->bindModel(wood1Model);

	//树
	Actor* tree1 = new Actor;
	tree1->setScale(glm::vec3(0.5));
	tree1->setPosition(glm::vec3(0.0f, -10.0f, -12.0f));
	this->addActor(tree1);
	tree1Model = new Model("assets/SceneModels/tree1/trees9.obj");
	tree1->bindModel(tree1Model);

	Actor* tree2 = new Actor;
	tree2->setScale(glm::vec3(0.4));
	tree2->setYaw(135.0f);
	tree2->setPosition(glm::vec3(6.0f, -10.0f, -9.0f));
	this->addActor(tree2);
	tree2Model = new Model("assets/SceneModels/tree1/trees9.obj");
	tree2->bindModel(tree2Model);

	Actor* tree3 = new Actor;
	tree3->setScale(glm::vec3(0.4));
	tree3->setYaw(-165.0f);
	tree3->setPosition(glm::vec3(5.0f, -10.0f, 18.0f));
	this->addActor(tree3);
	tree3Model = new Model("assets/SceneModels/tree1/trees9.obj");
	tree3->bindModel(tree3Model);

	//鹿
	Actor* deer1 = new Actor;
	deer1->setScale(glm::vec3(0.1));
	deer1->setYaw(-15.0f);
	deer1->setRoll(-90.0f);
	deer1->setPosition(glm::vec3(6.0f, -10.0f, -8.5f));
	this->addActor(deer1);
	deer1Model = new Model("assets/SceneModels/deer1/12961_White-Tailed_Deer_v1_l2.obj");
	deer1->bindModel(deer1Model);

	Actor* deer2 = new Actor;
	deer2->setScale(glm::vec3(0.1));
	deer2->setYaw(-135.0f);
	deer2->setRoll(-90.0f);
	deer2->setPosition(glm::vec3(9.0f, -10.0f, 2.0f));
	this->addActor(deer2);
	deer2Model = new Model("assets/SceneModels/deer1/12961_White-Tailed_Deer_v1_l2.obj");
	deer2->bindModel(deer2Model);

	Actor* deer3 = new Actor;
	deer3->setScale(glm::vec3(0.08));
	deer3->setYaw(160.0f);
	deer3->setRoll(-90.0f);
	deer3->setPosition(glm::vec3(-4.0f, -10.0f, 8.2f));
	this->addActor(deer3);
	deer3Model = new Model("assets/SceneModels/deer1/12961_White-Tailed_Deer_v1_l2.obj");
	deer3->bindModel(deer3Model);

	//房子
	//Actor* house1 = new Actor;
	//house1->setScale(glm::vec3(0.9));
	//house1->setYaw(10.0f);
	//house1->setRoll(0.0f);
	//house1->setPosition(glm::vec3(-9.0f, -10.0f, -1.0f));
	//this->addActor(house1);
	//house1Model = new Model("assets/SceneModels/house1/House.obj");
	//house1->bindModel(house1Model);
}

void WaterCloudShadowScene::setGUI()
{
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Scene average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SliderFloat("cloud type", &cloud_density, 0.3f, 1.0f);
	ImGui::ColorEdit3("cloud color", (float*)&(color_style));
	ImGui::SliderFloat("timespeed", &timespeed, 20.0f, 100.0f);

	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Light Control");
	ImGui::Checkbox("dynamic light", &dirLight1_isMoving);

	ImGui::TextColored(ImVec4(1, 1, 0, 1), "water control");
	ImGui::ColorEdit3("water color", (float*)&(water_color_type));


	ImGui::TextColored(ImVec4(1, 1, 0, 1), "shadow Type"); 
	ImGui::RadioButton("pcf square sample ", &dirLight1_shadowType, PCF_SQUARESAMPLE); 
	ImGui::RadioButton("pcf Poisson sample", &dirLight1_shadowType, PCF_POISSONSAMPLE);
	ImGui::RadioButton("pcss Poisson sample", &dirLight1_shadowType, PCSS_POISSONSAMPLE);
	if(dirLight1_shadowType==PCSS_POISSONSAMPLE)
		ImGui::SliderFloat("pcss light width", &dirLight1_width, 0.01f, 10.0f);
}

void WaterCloudShadowScene::initGUI()
{
	auto& app = AppRuntime::getInstance();
	vcgui = new GUI(this, app.getWindow());
	//enable mouse
	glfwSetInputMode(app.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void ShadowShader::read(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, const std::string& geometryShaderFilePath)
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

	geoId = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(geoId, 1, &gShaderCode, nullptr);
	glCompileShader(geoId);
	glGetShaderiv(geoId, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(geoId, sizeof(infoLog), nullptr, infoLog);
		errcode = ShaderError::F_SHADER_COMPILE_FAILED;
		errmsg = "failed to compile geometry shader. ";
		errmsg += infoLog;
		return;
	}

	this->id = glCreateProgram();
	glAttachShader(id, vertexId);
	glAttachShader(id, fragmentId);
	glAttachShader(id, geoId);
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

	/*uniformModel = glGetUniformLocation(id, "model");
	uniformProjection = glGetUniformLocation(id, "projection");
	uniformView = glGetUniformLocation(id, "view");
	uniformAmbientColor = glGetUniformLocation(id, "directionalLight.colour");
	uniformAmbientIntensity = glGetUniformLocation(id, "directionalLight.ambientIntensity");
	uniformDirection = glGetUniformLocation(id, "directionalLight.direction");
	uniformDiffuseIntensity = glGetUniformLocation(id, "directionalLight.diffuseIntensity");
	uniformSpecularIntensity = glGetUniformLocation(id, "material.specularIntensity");
	uniformShininess = glGetUniformLocation(id, "material.shininess");
	uniformEyePosition = glGetUniformLocation(id, "eyePosition");

	uniformUvScroll = glGetUniformLocation(id, "uvScroll");*/



	glDeleteShader(vertexId);
	glDeleteShader(fragmentId);
	glDeleteShader(geoId);

	vShaderFile.close();
	fShaderFile.close();
	gShaderFile.close();

	this->errcode = ShaderError::SHADER_OK;
}

void ShadowShader::useShadow()
{
	glUseProgram(id);
}

GLuint ShadowShader::getId()
{
	return id;
}

void NewWaterShader::read(WaterCloudShadowScene* scene, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, const std::string& geometryShaderFilePath)
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

	/*geoId = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(geoId, 1, &gShaderCode, nullptr);
	glCompileShader(geoId);
	glGetShaderiv(geoId, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(geoId, sizeof(infoLog), nullptr, infoLog);
		errcode = ShaderError::F_SHADER_COMPILE_FAILED;
		errmsg = "failed to compile geometry shader. ";
		errmsg += infoLog;
		return;
	}*/

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

	scene->uniformModel = glGetUniformLocation(id, "model");
	scene->uniformProjection = glGetUniformLocation(id, "projection");
	scene->uniformView = glGetUniformLocation(id, "view");
	scene->uniformAmbientColor = glGetUniformLocation(id, "directionalLight.colour");
	scene->uniformAmbientIntensity = glGetUniformLocation(id, "directionalLight.ambientIntensity");
	scene->uniformDirection = glGetUniformLocation(id, "directionalLight.direction");
	scene->uniformDiffuseIntensity = glGetUniformLocation(id, "directionalLight.diffuseIntensity");
	scene->uniformSpecularIntensity = glGetUniformLocation(id, "material.specularIntensity");
	scene->uniformShininess = glGetUniformLocation(id, "material.shininess");
	scene->uniformEyePosition = glGetUniformLocation(id, "eyePosition");
	scene->uniformUvScroll = glGetUniformLocation(id, "uvScroll");



	glDeleteShader(vertexId);
	glDeleteShader(fragmentId);
	//glDeleteShader(geoId);

	vShaderFile.close();
	fShaderFile.close();
	gShaderFile.close();

	this->errcode = ShaderError::SHADER_OK;
}

void NewWaterShader::useWater()
{
	glUseProgram(id);
}
GLuint NewWaterShader::getId()
{
	return id;
}

