/*

    Ìì¿ÕºÐ

    ´´½¨£º2022.11.7

*/

#pragma once

#include <ToyGraphCommon/EngineCompileOptions.h>

#include <ToyGraph/Actor.h>
#include <ToyGraph/Shader.h>

#include <string>
#include <vector>

class Skybox : public Actor {
public:
    Skybox(const std::vector<std::string>& faces);

    virtual void render(class Shader* pShader = nullptr) override;

protected:
    Shader skyboxShader{
        "../shaders/skybox/skybox.vert",
        "../shaders/skybox/skybox.frag"
    };

    GLuint cubemapTexture;

    GLuint skyboxVao;
    GLuint skyboxVbo;

};
