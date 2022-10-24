/*
    ToyEffects �������㡣
    ͬ�ü����ϵ�������ͼ��ѧ���γ̴���ҵ��Ŀ��

    ������2022��10��24�ա�

    GTY, GJT, SYB, TYN, AJQ, ZYF, GQW
*/

#include <ToyGraph/Engine.h>

#include <memory>
#include <set>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

const int SCR_WIDTH = 1200;
const int SCR_HEIGHT = 800;

/**
 * ���������в�����
 *
 * @param argc main �����յ��� argc��
 * @param argv main �����յ��� argv��
 * @param paramMap �洢 params ��ֵ��ӳ��� map��
 * @param paramSet �洢 params ���صļ��ϡ�
 * @param subProgramName �洢�ӳ������ֵ�ֵ��
 * @param additionalValues �洢�����������б�
 *
 * @return �Ƿ����������������Ϊ false����ʾ�����н��������������⡣
 */
bool parseParams(
    int argc, const char** argv,
    map<string, string>& paramMap,
    set<string>& paramSet,
    vector<string>& additionalValues
) {

    for (int idx = 0; idx < argc; idx++) {
        const char*& argCStr = argv[idx];

        if (argCStr[0] != '-') {
            additionalValues.emplace_back(argCStr);
            continue;
        }

        string arg = argCStr;

        auto colonPos = arg.find(':');

        if (colonPos == string::npos) {
            paramSet.insert(arg.substr(1));
            continue;
        }

        string paramKey = arg.substr(1, colonPos - 1);
        string paramVal = arg.substr(colonPos + 1);

        if (paramMap.count(paramKey)) {
            cout << "[Warning] main: redefine param key: " << paramKey << endl;
        }

        paramMap[paramKey] = paramVal;

    }

    return true;
}

Camera camera;

float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;
bool firstMouse = true;
void mouseCallback(double xPos, double yPos) {
    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    float xOff = xPos - lastX;
    float yOff = lastY - yPos; // reversed: y ranges bottom to top.
    lastX = xPos;
    lastY = yPos;

    const float sensitivity = 0.1f;
    xOff *= sensitivity;
    yOff *= sensitivity;

    float pitch = camera.getPitch();
    float yaw = camera.getYaw();

    pitch += yOff;
    yaw += xOff;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    } else if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    camera.setPitch(pitch);
    camera.setYaw(yaw);
}

void processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.move(cameraSpeed, camera.getDirectionVectorFront());
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {

        camera.move(-cameraSpeed, camera.getDirectionVectorFront());
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.move(-cameraSpeed, camera.getDirectionVectorRight());
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.move(cameraSpeed, camera.getDirectionVectorRight());
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.move(cameraSpeed, camera.getDirectionVectorUp());
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.move(-cameraSpeed, camera.getDirectionVectorUp());
    }
}

int main(const int argc, const char* argv[]) {
    // ���������в�����
    map<string, string> paramMap;
    set<string> paramSet;
    string subProgramName;
    vector<string> additionalValues;

    if (!parseParams(argc, argv, paramMap, paramSet, additionalValues)) {
        cout << "[Error] main: failed to parse commandline arguments." << endl;
        return -1;
    }

    // �������л�����
    auto& appRuntime = AppRuntime::getInstance("Toy Effects", SCR_WIDTH, SCR_HEIGHT);

    // �������λ�á�
    camera.setPosition(glm::vec3(-4, 2, 0));

    // �󶨻ص�������
    appRuntime.frameBufferSizeCallback = [&] (int w, int h) {
        appRuntime.setWindowSize(w, h);
    };

    appRuntime.cursorPosCallback = mouseCallback;
    appRuntime.activeKeyInputProcessor = processInput;

    // ���ز���ģ�͡�
    Model model("assets/nahida/nahida.pmx");

    if (model.errcode != ModelError::MODEL_OK) {
        cout << model.errmsg << endl;
        return -1;
    }

    model.setScale(glm::vec3(0.2)) // ��С��
        .setYaw(-90);

    // ���� shader��
    Shader shader("shaders/shader.vert", "shaders/shader.frag");
    if (shader.errcode != ShaderError::SHADER_OK) {
        cout << shader.errmsg << endl;
        return -2;
    }

    // ÿ֡���á�
    appRuntime.tick = [&](float deltaT) {

        shader.use();

        // transform
        auto projection = glm::perspective(glm::radians(camera.getFov()), 1.0f * SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        auto view = camera.getViewMatrix();
        shader.setMatrix4fv("projection", projection)
            .setMatrix4fv("view", view)
            .setMatrix4fv("model", model.getModelMatrix());

        model.draw(shader);
    };

    // ��ʼ���С�
    appRuntime.run();

    return 0;
}
