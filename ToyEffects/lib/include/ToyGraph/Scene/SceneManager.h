/*

    场景管理器。
    part of the ToyGraph project.

    创建：2022.11.6

*/

#pragma once

#include <functional>
#include <vector>

/**
 * 场景管理器。
 * 会负责管理其中所有场景的内存，即在生命周期结束时释放它们。
 */
class SceneManager final {
public:
    static SceneManager& getInstance();
    ~SceneManager();

public:
    class Scene* navigateTo(std::function<class Scene* ()> constructor);
    class Scene* redirectTo(std::function<class Scene* ()> constructor);
    void navigateBack(int delta = 1);

public:
    class Scene* currentScene();
    
private: // 私有成员。

    /**
     * 场景栈。
     */
    std::vector<class Scene*> sceneStack;

private:
    SceneManager();
    SceneManager(const SceneManager&) {};

};
