#pragma once

#include <ToyGraphCommon/EngineCompileOptions.h>
#include <ToyGraph/Actor.h>
#include <ToyGraph/Shader.h>

#include <string>
#include <vector>


class Mirror : public Actor {
public:
    Mirror(const std::vector<std::string>& faces);

    virtual void render(class Shader* pShader = nullptr) override;

protected:

    Shader mirrorShader{
        "../shaders/mirror/mirror.vert",
        "../shaders/mirror/mirror.frag"
    };

    GLuint cubemapTexture;

    GLuint mirrorVao;
    GLuint mirrorVbo;

};
