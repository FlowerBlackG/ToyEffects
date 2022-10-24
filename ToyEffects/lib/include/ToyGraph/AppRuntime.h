/*
    ToyGraph Runtime Header
    created on 2022.10.17
*/

#pragma once
#include "ToyGraphCommon/EngineCompileOptions.h"
#include <ToyGraph/Engine.h>

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
    AppRuntimeError errcode = AppRuntimeError::RUNTIME_OK;
    std::string errmsg;


public: // callables.
    std::function<void (
        int width, int height
    )> frameBufferSizeCallback = [] (...) {};

    std::function<void (
        double xPos, double yPos
    )> cursorPosCallback = [] (...) {};

    /**
     * 主动按键输入处理回调。
     * 调用时，由函数判断是否真的有按键按下。
     */
    std::function<void (
        GLFWwindow* window, float deltaT
    )> activeKeyInputProcessor = [] (...) {};

    std::function<void (
        float deltaT
    )> tick = [] (...) {}; 

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
