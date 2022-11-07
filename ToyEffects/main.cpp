/*
    ToyEffects 程序进入点。
    同济计算机系《计算机图形学》课程大作业项目。

    创建于2022年10月24日。

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
 * 解析命令行参数。
 * 参考自 ToyCompile 项目。
 *
 * @param argc main 函数收到的 argc。
 * @param argv main 函数收到的 argv。
 * @param paramMap 存储 params 键值对映射的 map。
 * @param paramSet 存储 params 开关的集合。
 * @param subProgramName 存储子程序名字的值。
 * @param additionalValues 存储独立参数的列表。
 *
 * @return 是否遇到错误。如果返回为 false，表示命令行解析遇到致命问题。
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
    // 解析命令行参数。
    map<string, string> paramMap;
    set<string> paramSet;
    string subProgramName;
    vector<string> additionalValues;

    if (!parseParams(argc, argv, paramMap, paramSet, additionalValues)) {
        cout << "[Error] main: failed to parse commandline arguments." << endl;
        return -1;
    }

    // 创造运行环境。
    auto& appRuntime = AppRuntime::getInstance("Toy Effects", SCR_WIDTH, SCR_HEIGHT);

    // 启动初始场景。
    auto& sceneMgr = SceneManager::getInstance();
    sceneMgr.navigateTo(MainScene::constructor);

    // 开始运行。
    appRuntime.run();

    return 0;
}
