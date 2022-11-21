
#include <ToyGraph/Engine.h>
#include <ToyGraph/Scene/SceneManager.h>
#include <ToyEffects/scenes/Skybox/PaimonScene.h>
#include <ToyEffects/scenes/Skybox/NahidaScene.h>
#include <ToyEffects/scenes/Skybox/WaterScene.h>
#include <ToyEffects/scenes/Skybox/shared.h>

static float lastX = 0;
static float lastY = 0;
static bool firstMouse = true;


void __nahidaPaimonSharedCursorPosCallback(double xPos, double yPos) {
    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    auto pCamera = SceneManager::getInstance().currentScene()->camera;
    auto& camera = *pCamera;
    

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

void __nahidaPaimonSharedActiveKeyInputProcessor(GLFWwindow* window, float deltaTime) {

    auto pCamera = SceneManager::getInstance().currentScene()->camera;
    auto& camera = *pCamera;
    auto& runtime = AppRuntime::getInstance();

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

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS 
        && runtime.lastFrameKeyStatus[GLFW_KEY_P] == GLFW_RELEASE
    ) {
        SceneManager::getInstance().navigateTo(PaimonScene::constructor);
    }

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS
        && runtime.lastFrameKeyStatus[GLFW_KEY_P] == GLFW_RELEASE
        ) {
        SceneManager::getInstance().navigateTo(WaterScene::constructor);
    }

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS 
        && runtime.lastFrameKeyStatus[GLFW_KEY_N] == GLFW_RELEASE
    ) {
        SceneManager::getInstance().navigateTo(NahidaScene::constructor);
    }

    
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS 
        && runtime.lastFrameKeyStatus[GLFW_KEY_B] == GLFW_RELEASE
    ) {
        SceneManager::getInstance().navigateBack(); 
    }
}
