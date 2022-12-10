/*
    Mesh.h
    ������ 2022��10��16�ա�
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
 * ���ƴ���� shader Ӧ�����ض�������� draw() ����˵����
 */
class Mesh {
public:
    // mesh data
    
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<struct Texture> textures;


    Mesh(
        std::vector<Vertex>& vertices,
        std::vector<GLuint>& indices,
        std::vector<struct Texture>& textures
    );

    /**
     * �߼����졣
     * ����һ���� Mesh �����Ա�ֱ��������ע�� vertices ����Ϣ��������ܡ�
     * ע��������ϣ�Ӧ�������� setupMesh()��
     */
    Mesh();
    
    void setupMesh();

    /**
     * ���ơ�
     * 
     * @param shader ����Ļ�ͼ shader �ļ���
     *           �涨��shader �� diffuse map �� specular map ��������������ʽ������
     *                uniform sampler2D textureDiffuseN
     *                uniform sampler2D textureSpecularN
     *                ���У�N ��ʾ�� n ����ͼ��n �� 0 ��ʼ��ţ���������Ӧ limit��
     *                �������� texture Ӧ�÷����� material �ṹ�ڡ�
     *                ����material.textureDiffuse0, ...
     */
    void draw(class Shader& shader);
    

protected:
    // �̶��������塣
    static const int TEXTURE_DIFFUSE_LIMIT = 3;
    static const int TEXTURE_SPECULAR_LIMIT = 2;

protected:

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;

    bool initialized = false;


};

