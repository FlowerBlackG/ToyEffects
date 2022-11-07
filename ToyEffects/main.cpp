/*
    ToyEffects �������㡣
    ͬ�ü����ϵ�������ͼ��ѧ���γ̴���ҵ��Ŀ��

    ������2022��10��24�ա�

    GTY, GJT, SYB, TYN, AJQ, ZYF, GQW
*/

#include <ToyGraph/Engine.h>
#include <ToyGraph/Scene/SceneManager.h>

#include <ToyEffects/scenes/MainScene.h>

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
 * �ο��� ToyCompile ��Ŀ��
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

    // ������ʼ������
    auto& sceneMgr = SceneManager::getInstance();
    sceneMgr.navigateTo(MainScene::constructor);

    // ��ʼ���С�
    appRuntime.run();

    return 0;
}
