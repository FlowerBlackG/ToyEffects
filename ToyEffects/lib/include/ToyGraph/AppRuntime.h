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
 * ToyGraph 运行时控制器。
 * 懒汉式单例模式。
 */
class AppRuntime {
public:

    /**
     * 获取实例。若实例未创建，则创建。 
     */
    static AppRuntime& getInstance();

    /**
     * 获取实例。若实例未创建，则以指定参数创建，并打开窗口。
     * 若实例已存在，则传参会被忽略。 
     */
    static AppRuntime& getInstance(
        const std::string& title,
        int width, 
        int height
    );

    /**
     * 创建一个窗口，同时准备 glad。
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


protected: // 属性。
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
    /** 实例对象指针。 */
    static AppRuntime* pInstance;

    /**
     * 实例对象释放器。自动释放实例。
     */
    class SingletonInstanceDeletor {
        ~SingletonInstanceDeletor() {
            if (AppRuntime::pInstance) {
                delete AppRuntime::pInstance;
            }
        }
    };

    /** 实例对象释放器。 */
    static SingletonInstanceDeletor singletonInstanceDeletor;


private:
    /*
        单例模式下，禁止外部调用构造等函数。
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
     * 初始化 glfw。
     */
    void prepareGlfw();

};
