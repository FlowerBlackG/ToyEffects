/*
    ToyGraph Runtime Header
    created on 2022.10.17
*/

#pragma once
#include "ToyGraphCommon/EngineCompileOptions.h"
#include <ToyGraph/Engine.h>
#include <array>

enum class AppRuntimeError {
    RUNTIME_OK,
    FAILED_LOAD_GLAD,
    FAILED_CREATE_WINDOW
};

/**
 * ToyGraph ����ʱ��������
 * ����ʽ����ģʽ��
 */
class AppRuntime {
public:

    /**
     * ��ȡʵ������ʵ��δ�������򴴽��� 
     */
    static AppRuntime& getInstance();

    /**
     * ��ȡʵ������ʵ��δ����������ָ���������������򿪴��ڡ�
     * ��ʵ���Ѵ��ڣ��򴫲λᱻ���ԡ� 
     */
    static AppRuntime& getInstance(
        const std::string& title,
        int width, 
        int height
    );

    /**
     * ����һ�����ڣ�ͬʱ׼�� glad��
     */
    void createWindow(
        const std::string& title = "",
        int width = 1800, 
        int height = 1200
    );

    

public: // setters & getters.

    AppRuntime& setWindowWidth(int width);
    AppRuntime& setWindowHeight(int height);
    AppRuntime& setWindowSize(int width, int height);
    int getWindowHeight() { return windowHeight; }
    int getWindowWidth() { return windowWidth; }

    AppRuntime& setTargetFrameRate(int target);
    AppRuntime& setTargetFrameTimeMs(int target);

    GLFWwindow* getWindow() { return this->window; };

public: // tick control.
    void run(); 

public:

    std::function<void (int width, int height)> frameBufferSizeCallback = [&] (int w, int h) {
        this->setWindowSize(w, h);
    };

    AppRuntimeError errcode = AppRuntimeError::RUNTIME_OK;
    std::string errmsg;

    std::array<int, GLFW_KEY_LAST + 1> lastFrameKeyStatus;


protected: // ���ԡ�
    GLFWwindow* window = nullptr;
    int windowWidth;
    int windowHeight;
    int targetFrameRate = 50;
    int targetFrameTimeMs = 1000 / this->targetFrameRate;
    float lastFrameTime = 0.0f;

protected: // event callback bridges.
    static void frameBufferSizeCallbackBridge(GLFWwindow* window, int width, int height);
    static void cursorPosCallbackBridge(GLFWwindow* window, double xPos, double yPos);

protected:
    /** ʵ������ָ�롣 */
    static AppRuntime* pInstance;

    /**
     * ʵ�������ͷ������Զ��ͷ�ʵ����
     */
    class SingletonInstanceDeletor {
        ~SingletonInstanceDeletor() {
            if (AppRuntime::pInstance) {
                delete AppRuntime::pInstance;
            }
        }
    };

    /** ʵ�������ͷ����� */
    static SingletonInstanceDeletor singletonInstanceDeletor;


private:
    /*
        ����ģʽ�£���ֹ�ⲿ���ù���Ⱥ�����
    */

    AppRuntime(const AppRuntime&) = default;

    AppRuntime();
    AppRuntime(
        const std::string& windowTitle,
        int windowWidth,
        int windowHeight
    );

    ~AppRuntime();

protected:

    /**
     * ��ʼ�� glfw��
     */
    void prepareGlfw();

};
