/*
    Mesh.h
    创建于 2022年10月16日。
*/

#pragma once
#include "ToyGraphCommon/EngineCompileOptions.h"
#include <ToyGraph/Shader.h>

#include <ToyGraph/Engine.h>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 textureCoord;
};


/**
 * Mesh.
 * 
 * 绘制传入的 shader 应满足特定规则。详见 draw() 函数说明。
 */
class Mesh {
public:
    // mesh data
    
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<struct Texture> textures;
    int indexCount;

    Mesh(
        std::vector<Vertex>& vertices,
        std::vector<GLuint>& indices,
        std::vector<struct Texture>& textures
    );

    /**
     * 高级构造。
     * 构造一个空 Mesh 对象，以便直接向其中注入 vertices 等信息，提高性能。
     * 注入内容完毕，应立即调用 setupMesh()。
     */
    Mesh();

    void CreateMesh(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices);
    void RenderMesh();

    void setupMesh();

    /**
     * 绘制。
     * 
     * @param shader 传入的绘图 shader 文件。
     *           规定：shader 内 diffuse map 和 specular map 的名称以如下形式命名：
     *                uniform sampler2D textureDiffuseN
     *                uniform sampler2D textureSpecularN
     *                其中，N 表示第 n 个贴图。n 从 0 开始标号，不超过对应 limit。
     *                上述所有 texture 应该放置于 material 结构内。
     *                即：material.textureDiffuse0, ...
     */
    void draw(class Shader& shader);
    

protected:
    // 固定参数定义。
    static const int TEXTURE_DIFFUSE_LIMIT = 3;
    static const int TEXTURE_SPECULAR_LIMIT = 2;

protected:

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;

    bool initialized = false;


};

