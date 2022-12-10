/*

    场景。
    part of the ToyGraph project

    创建：2022.11.6

*/

#pragma once

#include <ToyGraphCommon/EngineCompileOptions.h>
#include <ToyGraph/Engine.h>

#include <unordered_map>

class Scene {

public:
    Scene() {};
    virtual ~Scene();

public:

    virtual void onReload() {};
    virtual void onPause() {};


public:
    virtual void tick(float deltaSeconds);

    virtual void render();
    
public:
    void addActor(class Actor* actor, int id = -1);
    void removeActor(int id);

public:
    virtual void cursorPosCallback(double xPos, double yPos) {};

    /**
     * 主动按键输入处理回调。
     * 调用时，由函数判断是否真的有按键按下。
     */
    virtual void activeKeyInputProcessor(GLFWwindow* window, float deltaT) {};

    /*
    * tyn 12/7:为每个场景添加更新GUI的函数
    */
    virtual void setGUI() {};
public:
    class Camera* camera = nullptr;

protected:
    /**
     * 
     * id -> pActor
     */
    std::unordered_map<int, class Actor*> actors;

private:
    Scene(const Scene&) {};
    const Scene& operator = (const Scene&) { return *this; }

};
