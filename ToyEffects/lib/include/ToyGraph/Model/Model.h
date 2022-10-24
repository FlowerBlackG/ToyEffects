/*
    模型。
    创建于 2022年10月16日。
*/


#pragma once

#include "ToyGraphCommon/EngineCompileOptions.h"

// assimp library.
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <ToyGraph/Engine.h>

#include <ToyGraphCommon/CodePage.h>

enum class ModelError {
    MODEL_OK,
    FAILED_TO_LOAD_MODEL,
    FAILED_TO_LOAD_TEXTURE
};


class Model : public Actor {

public:
    Model() = default;
    Model(
        const std::string& filepath, 
        bool flipUVs = true, 
        CodePage binaryFileCp = CodePage::UTF8
    );

    void draw(class Shader& shader);

    void loadModel(
        const std::string& filepath, 
        bool flipUVs = true, 
        CodePage binaryFileCp = CodePage::UTF8
    );

public:
    ModelError errcode = ModelError::MODEL_OK;
    std::string errmsg;

protected:

    std::vector<class Mesh> meshes;
    std::string directory;

    

    void processNode(aiNode* node, const aiScene* scene, CodePage binaryFileCp);
    
    void processAndAppendMesh(aiMesh* mesh, const aiScene* scene, CodePage binaryFileCp);
    
    void loadMaterialTextures(
        aiMaterial *material, 
        aiTextureType aitype, 
        enum class TextureType type, 
        std::vector<Texture>& container,
        CodePage binaryFileCp
    );

};
