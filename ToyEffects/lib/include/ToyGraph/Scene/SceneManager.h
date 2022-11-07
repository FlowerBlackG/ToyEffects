/*

    ������������
    part of the ToyGraph project.

    ������2022.11.6

*/

#pragma once

#include <functional>
#include <vector>

/**
 * ������������
 * �Ḻ������������г������ڴ棬�����������ڽ���ʱ�ͷ����ǡ�
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
    
private: // ˽�г�Ա��

    /**
     * ����ջ��
     */
    std::vector<class Scene*> sceneStack;

private:
    SceneManager() {};
    SceneManager(const SceneManager&) {};

};
