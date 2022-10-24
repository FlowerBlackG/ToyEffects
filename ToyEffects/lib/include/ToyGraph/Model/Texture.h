/*
    ToyGraph Texture.h
    created on 2022.10.10
    refactored on 2022.10.17
*/


#pragma once
#include "ToyGraphCommon/EngineCompileOptions.h"
#include <ToyGraph/Engine.h>

enum class TextureType {
    DIFFUSE,
    SPECULAR
};

struct Texture {
    GLuint id;

    /**
     * 纹理类型。如：specular, diffuse.
     */
    TextureType type;
};

class TextureUtils {
public:
    static GLuint loadTextureFromFile(const std::string& filepath);


private:
    TextureUtils() = default;
    TextureUtils(const TextureUtils&) = default;

};
