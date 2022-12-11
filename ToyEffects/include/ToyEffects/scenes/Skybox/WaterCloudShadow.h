
#pragma once

#include <ToyGraph/Scene/Scene.h>
#include <ToyEffects/scenes/Skybox/WaterScene.h>//这里面有几个水场景相关的类和一个全局函数glCheckError_()和宏定义，复制场景暂时就引它的。。。
#include <ToyGraph/Skybox.h>
#include <ToyGraph/Camera.h>
#include <ToyGraph/Actor.h>
#include <ToyGraph/GUI.h>
#include <ToyGraph/Material.h>
#include <ToyGraph/Light.h>
#include <stb_image.h>


#define PCF_SQUARESAMPLE 1
#define PCF_POISSONSAMPLE 2
#define PCSS_POISSONSAMPLE 3

class WaterCloudShadowScene;
//同WaterShader一样，但确实需要使用几何着色器，所以继承了Shader【这两个Shader中需要访问场景类的变量，因为不太会所以暂时用此下策（全局变量就算了）】
class ShadowShader :Shader
{
public:
	void read(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, const std::string& geometryShaderFilePath);
	void useShadow();
	GLuint getId();
};

class NewWaterShader :Shader //跟原来WaterShader没啥区别，改掉了之前用全局变量来进行类通信，也不一定用得上几何着色器
{
public:
	void read(WaterCloudShadowScene* scene, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, const std::string& geometryShaderFilePath);
	void useWater();
	GLuint getId();
};



class WaterCloudShadowScene : public Scene {

	//演示的地平面
	float planeVertices[48] = {
		// positions            // normals         // texcoords
		 50.0f, -0.5f,  50.0f,  0.0f, 1.0f, 0.0f,  50.0f,  0.0f,
		-50.0f, -0.5f,  50.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-50.0f, -0.5f, -50.0f,  0.0f, 1.0f, 0.0f,   0.0f, 50.0f,

		 50.0f, -0.5f,  50.0f,  0.0f, 1.0f, 0.0f,  50.0f,  0.0f,
		-50.0f, -0.5f, -50.0f,  0.0f, 1.0f, 0.0f,   0.0f, 50.0f,
		 50.0f, -0.5f, -50.0f,  0.0f, 1.0f, 0.0f,  50.0f, 50.0f
	};
public:
	//初始化和删除
	WaterCloudShadowScene();
	~WaterCloudShadowScene();
	static WaterCloudShadowScene* constructor() {
		return new WaterCloudShadowScene;
	}
	virtual void render() override;
	virtual void tick(float deltaT) override;
	void cursorPosCallback(double xPos, double yPos) override;
	void activeKeyInputProcessor(GLFWwindow* window, float deltaT) override;
	//可通用的函数
	void drawCube(); //绘制方块
	void renderObjects(Shader& shader, const glm::mat4 projection, const glm::mat4 view);//自己的绘制物体
	void renderModels(Shader& shader, const glm::mat4 projection, const glm::mat4 view);//使用对应shader绘制导入的模型
	GLuint load2DTexture(char const* path);//【加载纹理暂时随便一些】
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
	void initPostProcessKits();
	//阴影函数	
	void initDirLightShadow();//平行光阴影
	void initPointLightShadow();//点光阴影
	void renderSceneWithShadows();
	void getDirLightShadowMap();//阴影生成
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
	Shader DirLightShader{ //方向光的pcf
		"shaders/ShadowTest/shadowMapping.vs", 
		"shaders/ShadowTest/shadowMapping.fs" 
	};
	Shader simpleDepthShader{ //方向光的阴影图
		"shaders/ShadowTest/shadowDepthMap.vs", 
		"shaders/ShadowTest/shadowDepthMap.fs" 
	};
	Shader characterShader{  
		"shaders/shader.vert",
		"shaders/shader.frag"
	};
	Shader lightSourceShader{
		"shaders/lightSrcVisualization.vs",
		"shaders/lightSrcVisualization.fs"
	};
	//Shader pcssPointShader{ //点光源的pcss
	//	"shaders/lightSrcVisualization.vs",
	//	"shaders/lightSrcVisualization.fs"
	//};
	//Shader cubeDepthShader{ //点光源的立方阴影图
	//	"shaders/lightSrcVisualization.vs",
	//	"shaders/lightSrcVisualization.fs"
	//};

	//模型
	Model* paimonModel = nullptr;
	Model* nahidaModel = nullptr;
	Model* wood1Model = nullptr;
	Model* deer1Model = nullptr,* deer2Model = nullptr;
	Model* tree1Model = nullptr;
	Model* house1Model = nullptr;
	Model* building1Model = nullptr;

	//水变量
	glm::mat4 water_projection;
	glm::vec3 water_pos = glm::vec3(0.0f, 3.0f, 0.0f);

	//改完fft再一并移入类——移进去先，不然全局变量复制场景麻烦啊
	WaterTexture waterTexture;
	WaterTexture waterTexture1;
	WaterTexture waterTexture2;
	WaterTexture waterTexture3;
	Material waterMaterial;
	Light mainLight;
	std::vector<WaterMesh*> meshList;
	std::vector<NewWaterShader> shaderList;

	//着色器变量序号，在read中获取
	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformUvScroll = 0, uniformEyePosition = 0,
		uniformAmbientIntensity = 0, uniformAmbientColor = 0, uniformDirection = 0, uniformDiffuseIntensity = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;


	//体积云变量
	GLuint VBO, VAO;
	//setup noise textures
	GLuint curltex, worltex, perlworltex, weathertex;
	const GLuint downscale = 1; //4 is best//any more and the gains dont make up for the lag
	GLuint downscalesq = downscale * downscale;
	int check;
	GLfloat cloud_density = 0.5;

	glm::vec3 color_style = glm::vec3(0.8, 0.5, 0.8);
	float timespeed = 60.0f;

	//阴影与光源相关
	/*阴影贴图分辨率，随着灯光正交投影矩阵变大
	（也就是要阴影的范围变大，为保证质量，这个值）
	需要更大，但是会卡*/
	const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

	glm::mat4 lightSpaceMatrix;//平行光源视角空间相关

	glm::vec3 dirLight1_pos = glm::vec3(-3.0f, 5.0f, -3.0f);//【正交投影这个足够了】
	float dirLight1_width = 1.0f;//按理平行光就没有宽度，也没有半影，但是为了用上PCSS。。。
	int dirLight1_shadowType = PCSS_POISSONSAMPLE;
	bool dirLight1_isMoving = true;
	//glm::vec3 dirLight1_pos = glm::vec3(-2.0f, 10.0f, -1.0f); //透视
	float dirLight1_lPlane = -75.0f, dirLight1_rPlane = 75.0f;//平行光源正交投影的左右平面
	float dirLight1_bPlane = -30.0f, dirLight1_tPlane = 30.0f;//平行光源正交投影的底顶平面
	float dirLight1_nPlane = 0.0f, dirLight1_fPlane = 35.0f;//平行光源的正交投影远近平面

	GLuint planeVBO, planeVAO; //自绘制的模型
	GLuint cubeVBO, cubeVAO; //测试用自绘制物体
	GLuint depthMapFBO, depthMapTex;//帧缓冲与深度图	
	GLuint woodTex;//纹理
	
	// 后效。
	int postProcessEffect = 0;
	const int N_EFFECTS = 6;
	Shader screenShader{
		"shaders/postprocess-screen-shader.vert",
		"shaders/postprocess-screen-shader.frag"
	};

	GLuint fbo = 0;
	GLuint scrQuadVao;
	GLuint scrQuadVbo;
	GLuint textureColorbuffer;
	
	//GUI相关
	GUI* vcgui = nullptr;
	virtual void setGUI()override;
	void initGUI();
};






/*下面就是WaterScene里的三个类，定义和实现在WaterScene中【WaterShader合并到上面，其他两个直接通过头文件引】*/
//class WaterMesh :Mesh {
//protected:
//	unsigned int indexCount;
//
//
//public:
//	WaterMesh();
//	void CreateMesh(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices);
//	void RenderMesh();
//
//
//};
//
//////class WaterShader :Shader
//////{
//////public:
//////	void read(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, const std::string& geometryShaderFilePath);
//////	void useWater();
//////	GLuint getId();
//////};
//
//class WaterTexture {
//
//	int width, height, bitDepth;
//	char* fileLocation;
//	/**
//	 * 纹理类型。如：specular, diffuse.
//	 */
//	TextureType type;
//	int RGB_type;
//
//public:
//	GLuint id;
//	void setfileLocation(char* s, int RGBtype);
//	void LoadTexture();
//	void UseTexture();
//	WaterTexture();
//};
