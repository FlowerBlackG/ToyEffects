#include <ToyEffects/scenes/Skybox/WaterScene.h>
#include <ToyGraph/Scene/SceneManager.h>
#include <ToyEffects/scenes/Skybox/shared.h>
#include <iostream>
#include <random>
#include <complex>

using namespace std;



//�궨��
#define DISP_MAP_SIZE		512					// 1024 ������ɵ�����
#define PATCH_SIZE			20.0f				// m
#define WIND_DIRECTION		{ -0.4f, -0.9f }	//����������
#define WIND_SPEED			6.5f				// m/s  ����
#define AMPLITUDE_CONSTANT	(0.45f * 1e-3f)		// Phillips ���׵�A
#define GRAV_ACCELERATION	9.81f				// m/s^2 ���ٶ�
#define MESH_SIZE			64					// [64, 256]
#define FURTHEST_COVER		8					// ����������= PATCH_SIZE * (1 << FURTHEST_COVER)
#define MAX_COVERAGE		64.0f				// pixel limit for a distant patch to be rendered
//Ϊopengl��չapi��glet.h�е����ݣ�����ɾ
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84ff
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE

static const int IndexCounts[] = {
	0,
	0,
	0,
	0,
	0,
	0,
	961920,		// 64x64
	3705084,	// 128x128
	14500728	// 256x256
};
float square_vertices[] = {
	// positions          // normals           // texture coords
	// ��һ��������
	0.5f, 0.5f, 0.0f,   // ���Ͻ�
	0.5f, -0.5f, 0.0f,  // ���½�
	-0.5f, 0.5f, 0.0f,  // ���Ͻ�
	// �ڶ���������
	0.5f, -0.5f, 0.0f,  // ���½�
	-0.5f, -0.5f, 0.0f, // ���½�
	-0.5f, 0.5f, 0.0f   // ���Ͻ�

};
unsigned int VBO, VAO;


//��ѧ����
static const float PI = 3.141592f;
static const float ONE_OVER_SQRT_2 = 0.7071067f;	//����2��֮1
static const float ONE_OVER_SQRT_TWO_PI = 0.39894228f;
//��д��ȫ�ֱ���������water�����ٷ�װ
unsigned int				initial = 0;			// ��ʼ����,h0(����)
unsigned int				frequencies = 0;		// Ƶ�� w_i ÿ��������
unsigned int				updated[2] = { 0 };		// updated spectra h~(k,t)��D~(k,t)
unsigned int				tempdata = 0;			// �м���� FT
unsigned int				displacement = 0;		// λ��ͼ
unsigned int				gradients = 0;			// �����۵���ͼ
uint32_t					numlods = 0;
unsigned int				perlintex = 0;		// Perlin ���� to remove tiling artifacts
unsigned int				environment = 0;
unsigned int				debugvao = 0;
unsigned int				helptext = 0;


//��wn��׼��
void Vec2Normalize(glm::vec2& out, glm::vec2& v);
//���������ȣ����滻��
float Vec2Length(const glm::vec2& v);
//��log2x������
uint32_t Log2OfPow2(uint32_t x);


//��������д����ʵӦ�õ���
void WaterScene::cursorPosCallback(double xPos, double yPos) {
    __nahidaPaimonSharedCursorPosCallback(xPos, yPos);
}

void WaterScene::activeKeyInputProcessor(GLFWwindow* window, float deltaTime) {
    __nahidaPaimonSharedActiveKeyInputProcessor(window, deltaTime);

}


WaterScene::~WaterScene() {
    if (this->pSkybox) {
        delete this->pSkybox;
    }

}
//ʱ�̸ı�λ�ã����ﲻ��Ҫ
void WaterScene::tick(float deltaT) {
    auto cube1 = this->actors[0];
    //cube1->setYaw(cube1->getYaw() + deltaT * 20);
}


void WaterScene::render() {
    auto& runtime = AppRuntime::getInstance();

    pSkybox->render();

    cube.use();
    auto view = camera->getViewMatrix();
    auto projection = glm::perspective(
        glm::radians(camera->getFov()),
        1.0f * runtime.getWindowWidth() / runtime.getWindowHeight(),
        0.1f,
        100.0f
    );
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(1.0f, 0.3f, 0.5f));
    model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube

    cube.setMatrix4fv("projection", projection)
        .setMatrix4fv("view", view)
        .setMatrix4fv("model", model);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    for (auto it : this->actors) {
        it.second->render(&cube);
    }

	
}

WaterScene::WaterScene() {

    vector<string> skyboxFaces({
        "assets/SpaceboxCollection/Spacebox3/LightGreen_right1.png",
        "assets/SpaceboxCollection/Spacebox3/LightGreen_left2.png",
        "assets/SpaceboxCollection/Spacebox3/LightGreen_top3.png",
        "assets/SpaceboxCollection/Spacebox3/LightGreen_bottom4.png",
        "assets/SpaceboxCollection/Spacebox3/LightGreen_front5.png",
        "assets/SpaceboxCollection/Spacebox3/LightGreen_back6.png"
        });

    pSkybox = new Skybox(skyboxFaces);

	InitScene();

    Actor* cube1 = new Actor;
    cube1->setScale(glm::vec3(1.0));
    this->addActor(cube1);

    // ��Ⱦһ�������ʵ�������,����һЩ����

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    //���ɲ���VAO��VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // ���������ݰ�����ǰĬ�ϵĻ�����
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices), square_vertices, GL_STATIC_DRAW);
    // ���ö�������ָ��
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    //glEnableVertexAttribArray(2);

    this->camera = new Camera;
    //camera->setPosition(glm::vec3(-10, 0, 0));

    if (cube.errcode != ShaderError::SHADER_OK) {
        cout << "cube shader err: " << cube.errmsg << endl;
    }
	cnt = 0;
}


bool WaterScene::InitScene()
{
	auto& runtime = AppRuntime::getInstance();

	std::mt19937 gen;//�����
	std::normal_distribution<> gaussian(0.0, 1.0);//��̬�ֲ�������0������1
	GLint maxanisotropy = 1;		//�������ԣ�ʲô����..

	uint32_t screenwidth = runtime.getWindowWidth();
	uint32_t screenheight = runtime.getWindowHeight();


	// setup OpenGL
	glClearColor(0.0f, 0.125f, 0.3f, 1.0f);
	glClearDepth(1.0);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//ͼԪ����(Primitive restart) �����û����Ʋ������ġ���ɢ��ͼ��
	//�������ֵ��ʱ��OpenGL�������ͼԪ�����ǽ�����һ�λ��ƣ�Ȼ�����������µĻ���
	glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);

	glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxanisotropy);
	maxanisotropy = max(maxanisotropy, 2);

	// ���ɳ�ʼƵ�׺�Ƶ��
	glm::vec2 k;
	float L = PATCH_SIZE;

	glGenTextures(1, &initial);
	glGenTextures(1, &frequencies);

	glBindTexture(GL_TEXTURE_2D, initial);
	//���������ͼ��
	//������ƻὫָ�� ����ͼ�� ��һ����ӳ�䵽���������ÿ��ͼ�λ�Ԫ
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG32F, DISP_MAP_SIZE + 1, DISP_MAP_SIZE + 1);

	glBindTexture(GL_TEXTURE_2D, frequencies);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, DISP_MAP_SIZE + 1, DISP_MAP_SIZE + 1);

	// n��[-N / 2, N / 2]��
	int start = DISP_MAP_SIZE / 2;

	// Ϊ�˶Գ�,  (N + 1) x (N + 1) 
	//����һ����������,�洢����̬�ֲ�������������h0(k)������
	complex<float>* h0data = new complex<float>[(DISP_MAP_SIZE + 1) * (DISP_MAP_SIZE + 1)];

	float* wdata = new float[(DISP_MAP_SIZE + 1) * (DISP_MAP_SIZE + 1)];

	glm::vec2 w = WIND_DIRECTION;		//������
	glm::vec2 wn;
	float V = WIND_SPEED;				//����
	float A = AMPLITUDE_CONSTANT;		//PHilipƵ�׵�A

	cout << "����߶ȳ�����ӡǰ50��" << endl;
	//��w��׼����wn,w�Ȳ������������ﲻ�ÿ⺯��
	Vec2Normalize(wn, w);
	//����m,n���������е�w��h0ֵ���̶�ֵ
	for (int m = 0; m <= DISP_MAP_SIZE; ++m)
	{
		k.y = ((2 * PI) * (start - m)) / L;

		for (int n = 0; n <= DISP_MAP_SIZE; ++n) {
			k.x = ((2 * PI) * (start - n)) / L;

			int index = m * (DISP_MAP_SIZE + 1) + n;
			float sqrt_P_h = 0;

			if (k.x != 0.0f || k.y != 0.0f)
				sqrt_P_h = sqrtf(Phillips(k, wn, V, A));

			h0data[index].real((float)(sqrt_P_h * gaussian(gen) * ONE_OVER_SQRT_2));
			h0data[index].imag((float)(sqrt_P_h * gaussian(gen) * ONE_OVER_SQRT_2));
			// ��������� w^2(k) = gk
			wdata[index] = sqrtf(GRAV_ACCELERATION * Vec2Length(k));
			//ֻ��ӡһ��
			if (cnt <= 50) {
				cout << "m:" << m << "    " << "n:" << n << "    ";
				cout <<"h~0(m,n):"<< h0data[index].real() << "+" << h0data[index].imag() << "i" << endl;
				cout << "index: " << index << "    ������w2(k):" << wdata[index] << endl << endl;;
				cnt ++;
			}


		}
	}


	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, DISP_MAP_SIZE + 1, DISP_MAP_SIZE + 1, GL_RED, GL_FLOAT, wdata);

	glBindTexture(GL_TEXTURE_2D, initial);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, DISP_MAP_SIZE + 1, DISP_MAP_SIZE + 1, GL_RG, GL_FLOAT, h0data);

	delete[] wdata;
	delete[] h0data;

	// ��������Ƶ����ͼ
	glGenTextures(2, updated);
	glBindTexture(GL_TEXTURE_2D, updated[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG32F, DISP_MAP_SIZE, DISP_MAP_SIZE);

	glBindTexture(GL_TEXTURE_2D, updated[1]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG32F, DISP_MAP_SIZE, DISP_MAP_SIZE);

	glGenTextures(1, &tempdata);
	glBindTexture(GL_TEXTURE_2D, tempdata);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG32F, DISP_MAP_SIZE, DISP_MAP_SIZE);

	// λ��ͼ��ʲô��˼����
	glGenTextures(1, &displacement);
	glBindTexture(GL_TEXTURE_2D, displacement);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, DISP_MAP_SIZE, DISP_MAP_SIZE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// create gradient & folding map
	glGenTextures(1, &gradients);
	glBindTexture(GL_TEXTURE_2D, gradients);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, DISP_MAP_SIZE, DISP_MAP_SIZE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxanisotropy / 2);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Shader LOD��ʵ���Ǹ����豸���ܵĲ�ͬ���벻ͬ�汾��Shader
	// ���������LOD����(������δ��ʹ��tess shader),,������
	//OpenGLVertexElement decl[] = {
	//	{ 0, 0, GLDECLTYPE_FLOAT3, GLDECLUSAGE_POSITION, 0 },
	//	{ 0xff, 0, 0, 0, 0 }
	//};

	//numlods = Log2OfPow2(MESH_SIZE);

	//if (!GLCreateMesh((MESH_SIZE + 1) * (MESH_SIZE + 1), IndexCounts[numlods], GLMESH_32BIT, decl, &oceanmesh))
	//	return false;

	//OpenGLAttributeRange* subsettable = nullptr;
	//glm::vec3* vdata = nullptr;
	//uint32_t* idata = nullptr;
	//unsigned int numsubsets = 0;

	//oceanmesh->LockVertexBuffer(0, 0, GLLOCK_DISCARD, (void**)&vdata);
	//oceanmesh->LockIndexBuffer(0, 0, GLLOCK_DISCARD, (void**)&idata);
	//{
	//	float tilesize = PATCH_SIZE / MESH_SIZE;

	//	// vertex data
	//	for (int z = 0; z <= MESH_SIZE; ++z) {
	//		for (int x = 0; x <= MESH_SIZE; ++x) {
	//			int index = z * (MESH_SIZE + 1) + x;

	//			vdata[index].x = (float)x;
	//			vdata[index].y = (float)z;
	//			vdata[index].z = 0.0f;
	//		}
	//	}

	//	// index data
	//	GenerateLODLevels(&subsettable, &numsubsets, idata);
	//}
	//oceanmesh->UnlockIndexBuffer();
	//oceanmesh->UnlockVertexBuffer();

	//oceanmesh->SetAttributeTable(subsettable, numsubsets);
	//delete[] subsettable;

	//// load shaders
	//char defines[128];

	//sprintf_s(defines, "#define DISP_MAP_SIZE	%d\n#define LOG2_DISP_MAP_SIZE	%d\n#define TILE_SIZE_X2	%.4f\n#define INV_TILE_SIZE	%.4f\n",
	//	DISP_MAP_SIZE,
	//	Log2OfPow2(DISP_MAP_SIZE),
	//	PATCH_SIZE * 2.0f / DISP_MAP_SIZE,
	//	DISP_MAP_SIZE / PATCH_SIZE);

	//if (!GLCreateEffectFromFile("shaders/ocean/screenquad.vert", 0, 0, 0, "shaders/ocean/debugspectra.frag", &debugeffect, defines))
	//	return false;

	//if (!GLCreateComputeProgramFromFile("shaders/ocean/updatespectrum.comp", &updatespectrum, defines))
	//	return false;

	//if (!GLCreateComputeProgramFromFile("shaders/ocean/fourier_dft.comp", &fourier_dft, defines))
	//	return false;

	//if (!GLCreateComputeProgramFromFile("shaders/ocean/fourier_fft.comp", &fourier_fft, defines))
	//	return false;

	//if (!GLCreateComputeProgramFromFile("shaders/ocean/createdisplacement.comp", &createdisp, defines))
	//	return false;

	//if (!GLCreateComputeProgramFromFile("shaders/ocean/creategradients.comp", &creategrad, defines))
	//	return false;

	//if (!GLCreateEffectFromFile("shaders/ocean/ocean.vert", 0, 0, 0, "shaders/ocean/ocean.frag", &oceaneffect, defines))
	//	return false;

	//if (!GLCreateEffectFromFile("shaders/ocean/ocean.vert", 0, 0, 0, "shaders/ocean/simplecolor.frag", &wireeffect, defines))
	//	return false;


	//screenquad = new OpenGLScreenQuad();

	//// NOTE: can't query image bindings
	//updatespectrum->SetInt("tilde_h0", 0);
	//updatespectrum->SetInt("frequencies", 1);
	//updatespectrum->SetInt("tilde_h", 2);
	//updatespectrum->SetInt("tilde_D", 3);

	//fourier_dft->SetInt("readbuff", 0);
	//fourier_dft->SetInt("writebuff", 1);

	//fourier_fft->SetInt("readbuff", 0);
	//fourier_fft->SetInt("writebuff", 1);

	//createdisp->SetInt("heightmap", 0);
	//createdisp->SetInt("choppyfield", 1);
	//createdisp->SetInt("displacement", 2);

	//creategrad->SetInt("displacement", 0);
	//creategrad->SetInt("gradients", 1);

	//oceaneffect->SetInt("displacement", 0);
	//oceaneffect->SetInt("perlin", 1);
	//oceaneffect->SetInt("envmap", 2);
	//oceaneffect->SetInt("gradients", 3);

	//float white[] = { 1, 1, 1, 1 };

	//wireeffect->SetInt("displacement", 0);
	//wireeffect->SetInt("perlin", 1);
	//wireeffect->SetVector("matColor", white);

	//skyeffect->SetInt("sampler0", 0);
	//debugeffect->SetInt("sampler0", 0);

	//// other
	//if (!GLCreateTextureFromFile("../../Media/Textures/perlin_noise.dds", false, &perlintex))
	//	return false;

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxanisotropy / 2);


	//if (!GLCreateCubeTextureFromDDS("../../Media/Textures/ocean_env.dds", true, &environment))
	//	return false;

	//glGenVertexArrays(1, &debugvao);
	//glBindVertexArray(debugvao);
	//{
	//	// empty
	//}
	//glBindVertexArray(0);

	//// init quadtree
	//float ocean_extent = PATCH_SIZE * (1 << FURTHEST_COVER);
	//float ocean_start[2] = { -0.5f * ocean_extent, -0.5f * ocean_extent };

	//tree.Initialize(ocean_start, ocean_extent, (int)numlods, MESH_SIZE, PATCH_SIZE, MAX_COVERAGE, (float)(screenwidth * screenheight));

	// render text
	//GLCreateTexture(512, 512, 1, GLFMT_A8B8G8R8, &helptext);

	//GLRenderText(
	//	"Use WASD and mouse to move around\n1-8: Change ocean color\n\nT - Toggle FFT/DFT\nR - Toggle debug camera\nH - Toggle help text",
	//	helptext, 512, 512);

	return true;
}

void WaterScene::UninitScene()
{
	//delete oceanmesh;
	//delete skyeffect;
	//delete oceaneffect;
	//delete wireeffect;
	//delete debugeffect;
	//delete updatespectrum;
	//delete fourier_dft;
	//delete fourier_fft;
	//delete createdisp;
	//delete creategrad;
	//delete screenquad;

	glDeleteVertexArrays(1, &debugvao);

	glDeleteTextures(1, &displacement);
	glDeleteTextures(1, &gradients);
	glDeleteTextures(1, &initial);
	glDeleteTextures(1, &frequencies);
	glDeleteTextures(2, updated);
	glDeleteTextures(1, &tempdata);
	glDeleteTextures(1, &environment);
	glDeleteTextures(1, &perlintex);
	glDeleteTextures(1, &helptext);

	//OpenGLContentManager().Release();
}

// Phillips����
 float WaterScene::Phillips(const glm::vec2& k, const glm::vec2& w, float V, float A)
{
	float L = (V * V) / GRAV_ACCELERATION;	// largest possible wave for wind speed V
	float l = L / 1000.0f;					// supress waves smaller than this

	float kdotw =glm::dot(k, w);
	float k2 = glm::dot(k, k);			// squared length of wave vector k

	// k^6 because k must be normalized
	float P_h = A * (expf(-1.0f / (k2 * L * L))) / (k2 * k2 * k2) * (kdotw * kdotw);

	if (kdotw < 0.0f) {
		// wave is moving against wind direction w
		P_h *= 0.07f;
	}

	return P_h * expf(-k2 * l * l);
}

 unsigned int WaterScene::GenerateBoundaryMesh(int deg_left, int deg_top, int deg_right, int deg_bottom, int levelsize, uint32_t* idata)
 {
#define CALC_BOUNDARY_INDEX(x, z) \
	((z) * (MESH_SIZE + 1) + (x))
	 // END

	 unsigned int numwritten = 0;

	 // top edge
	 if (deg_top < levelsize) {
		 int t_step = levelsize / deg_top;

		 for (int i = 0; i < levelsize; i += t_step) {
			 idata[numwritten++] = CALC_BOUNDARY_INDEX(i, 0);
			 idata[numwritten++] = CALC_BOUNDARY_INDEX(i + t_step / 2, 1);
			 idata[numwritten++] = CALC_BOUNDARY_INDEX(i + t_step, 0);

			 for (int j = 0; j < t_step / 2; ++j) {
				 if (i == 0 && j == 0 && deg_left < levelsize)
					 continue;

				 idata[numwritten++] = CALC_BOUNDARY_INDEX(i, 0);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(i + j, 1);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(i + j + 1, 1);
			 }

			 for (int j = t_step / 2; j < t_step; ++j) {
				 if (i == levelsize - t_step && j == t_step - 1 && deg_right < levelsize)
					 continue;

				 idata[numwritten++] = CALC_BOUNDARY_INDEX(i + t_step, 0);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(i + j, 1);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(i + j + 1, 1);
			 }
		 }
	 }

	 // left edge
	 if (deg_left < levelsize) {
		 int l_step = levelsize / deg_left;

		 for (int i = 0; i < levelsize; i += l_step) {
			 idata[numwritten++] = CALC_BOUNDARY_INDEX(0, i);
			 idata[numwritten++] = CALC_BOUNDARY_INDEX(0, i + l_step);
			 idata[numwritten++] = CALC_BOUNDARY_INDEX(1, i + l_step / 2);

			 for (int j = 0; j < l_step / 2; ++j) {
				 if (i == 0 && j == 0 && deg_top < levelsize)
					 continue;

				 idata[numwritten++] = CALC_BOUNDARY_INDEX(0, i);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(1, i + j + 1);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(1, i + j);
			 }

			 for (int j = l_step / 2; j < l_step; ++j) {
				 if (i == levelsize - l_step && j == l_step - 1 && deg_bottom < levelsize)
					 continue;

				 idata[numwritten++] = CALC_BOUNDARY_INDEX(0, i + l_step);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(1, i + j + 1);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(1, i + j);
			 }
		 }
	 }

	 // right edge
	 if (deg_right < levelsize) {
		 int r_step = levelsize / deg_right;

		 for (int i = 0; i < levelsize; i += r_step) {
			 idata[numwritten++] = CALC_BOUNDARY_INDEX(levelsize, i);
			 idata[numwritten++] = CALC_BOUNDARY_INDEX(levelsize - 1, i + r_step / 2);
			 idata[numwritten++] = CALC_BOUNDARY_INDEX(levelsize, i + r_step);

			 for (int j = 0; j < r_step / 2; ++j) {
				 if (i == 0 && j == 0 && deg_top < levelsize)
					 continue;

				 idata[numwritten++] = CALC_BOUNDARY_INDEX(levelsize, i);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(levelsize - 1, i + j);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(levelsize - 1, i + j + 1);
			 }

			 for (int j = r_step / 2; j < r_step; ++j) {
				 if (i == levelsize - r_step && j == r_step - 1 && deg_bottom < levelsize)
					 continue;

				 idata[numwritten++] = CALC_BOUNDARY_INDEX(levelsize, i + r_step);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(levelsize - 1, i + j);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(levelsize - 1, i + j + 1);
			 }
		 }
	 }

	 // bottom edge
	 if (deg_bottom < levelsize) {
		 int b_step = levelsize / deg_bottom;

		 for (int i = 0; i < levelsize; i += b_step) {
			 idata[numwritten++] = CALC_BOUNDARY_INDEX(i, levelsize);
			 idata[numwritten++] = CALC_BOUNDARY_INDEX(i + b_step, levelsize);
			 idata[numwritten++] = CALC_BOUNDARY_INDEX(i + b_step / 2, levelsize - 1);

			 for (int j = 0; j < b_step / 2; ++j) {
				 if (i == 0 && j == 0 && deg_left < levelsize)
					 continue;

				 idata[numwritten++] = CALC_BOUNDARY_INDEX(i, levelsize);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(i + j + 1, levelsize - 1);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(i + j, levelsize - 1);
			 }

			 for (int j = b_step / 2; j < b_step; ++j) {
				 if (i == levelsize - b_step && j == b_step - 1 && deg_right < levelsize)
					 continue;

				 idata[numwritten++] = CALC_BOUNDARY_INDEX(i + b_step, levelsize);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(i + j + 1, levelsize - 1);
				 idata[numwritten++] = CALC_BOUNDARY_INDEX(i + j, levelsize - 1);
			 }
		 }
	 }

	 return numwritten;
 }

// void WaterScene::GenerateLODLevels(OpenGLAttributeRange** subsettable, unsigned int* numsubsets, uint32_t* idata)
// {
//#define CALC_INNER_INDEX(x, z) \
//	((top + (z)) * (MESH_SIZE + 1) + left + (x))
//	 // END
//
//	 assert(subsettable);
//	 assert(numsubsets);
//
//	 *numsubsets = (numlods - 2) * 3 * 3 * 3 * 3 * 2;
//	 *subsettable = new OpenGLAttributeRange[*numsubsets];
//
//	 int currsubset = 0;
//	 unsigned int indexoffset = 0;
//	 unsigned int numwritten = 0;
//	 OpenGLAttributeRange* subset = 0;
//
//	 //int numrestarts = 0;
//
//	 for (uint32_t level = 0; level < numlods - 2; ++level) {
//		 int levelsize = MESH_SIZE >> level;
//		 int mindegree = levelsize >> 3;
//
//		 for (int left_degree = levelsize; left_degree > mindegree; left_degree >>= 1) {
//			 for (int right_degree = levelsize; right_degree > mindegree; right_degree >>= 1) {
//				 for (int bottom_degree = levelsize; bottom_degree > mindegree; bottom_degree >>= 1) {
//					 for (int top_degree = levelsize; top_degree > mindegree; top_degree >>= 1) {
//						 int right = ((right_degree == levelsize) ? levelsize : levelsize - 1);
//						 int left = ((left_degree == levelsize) ? 0 : 1);
//						 int bottom = ((bottom_degree == levelsize) ? levelsize : levelsize - 1);
//						 int top = ((top_degree == levelsize) ? 0 : 1);
//
//						 // generate inner mesh (triangle strip)
//						 int width = right - left;
//						 int height = bottom - top;
//
//						 numwritten = 0;
//
//						 for (int z = 0; z < height; ++z) {
//							 if ((z & 1) == 1) {
//								 idata[numwritten++] = CALC_INNER_INDEX(0, z);
//								 idata[numwritten++] = CALC_INNER_INDEX(0, z + 1);
//
//								 for (int x = 0; x < width; ++x) {
//									 idata[numwritten++] = CALC_INNER_INDEX(x + 1, z);
//									 idata[numwritten++] = CALC_INNER_INDEX(x + 1, z + 1);
//								 }
//
//								 idata[numwritten++] = UINT32_MAX;
//								 //++numrestarts;
//							 }
//							 else {
//								 idata[numwritten++] = CALC_INNER_INDEX(width, z + 1);
//								 idata[numwritten++] = CALC_INNER_INDEX(width, z);
//
//								 for (int x = width - 1; x >= 0; --x) {
//									 idata[numwritten++] = CALC_INNER_INDEX(x, z + 1);
//									 idata[numwritten++] = CALC_INNER_INDEX(x, z);
//								 }
//
//								 idata[numwritten++] = UINT32_MAX;
//								 //++numrestarts;
//							 }
//						 }
//
//						 // add inner subset
//						 subset = ((*subsettable) + currsubset);
//
//						 subset->AttribId = currsubset;
//						 subset->Enabled = (numwritten > 0);
//						 subset->IndexCount = numwritten;
//						 subset->IndexStart = indexoffset;
//						 subset->PrimitiveType = GL_TRIANGLE_STRIP;
//						 subset->VertexCount = 0;
//						 subset->VertexStart = 0;
//
//						 indexoffset += numwritten;
//						 idata += numwritten;
//
//						 ++currsubset;
//
//						 // generate boundary mesh (triangle list)
//						 numwritten = GenerateBoundaryMesh(left_degree, top_degree, right_degree, bottom_degree, levelsize, idata);
//
//						 // add boundary subset
//						 subset = ((*subsettable) + currsubset);
//
//						 subset->AttribId = currsubset;
//						 subset->Enabled = (numwritten > 0);
//						 subset->IndexCount = numwritten;
//						 subset->IndexStart = indexoffset;
//						 subset->PrimitiveType = GL_TRIANGLES;
//						 subset->VertexCount = 0;
//						 subset->VertexStart = 0;
//
//						 indexoffset += numwritten;
//						 idata += numwritten;
//
//						 ++currsubset;
//					 }
//				 }
//			 }
//		 }
//	 }
//
//	 //OpenGLAttributeRange& lastsubset = (*subsettable)[currsubset - 1];
//	 //printf("Total indices: %lu (%lu restarts)\n", lastsubset.IndexStart + lastsubset.IndexCount, numrestarts);
// }

void Vec2Normalize(glm::vec2& out, glm::vec2& v)
{
	float il = 1.0f / sqrtf(v.x * v.x + v.y * v.y);

	out[0] = v[0] * il;
	out[1] = v[1] * il;
}

//���������ȣ����滻��
float Vec2Length(const glm::vec2& v)
{
	return sqrtf(v.x * v.x + v.y * v.y);
}
//��log2x������
uint32_t Log2OfPow2(uint32_t x)
{
	uint32_t ret = 0;

	while (x >>= 1)
		++ret;

	return ret;
}
