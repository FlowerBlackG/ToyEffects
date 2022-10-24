/*
 * Shader �����װ�ࡣ
 * ������ OpenGL ����
 */

#pragma once
#include "ToyGraphCommon/EngineCompileOptions.h"
#include <ToyGraph/Engine.h>

enum class ShaderError {
    UNINITIALIZED,
    SHADER_OK,
    V_SHADER_OPEN_FAILED,
    F_SHADER_OPEN_FAILED,
    V_SHADER_COMPILE_FAILED,
    F_SHADER_COMPILE_FAILED,
    LINKING_FAILED,
};

/**
 * Shader �����װ�ࡣ 
 * ������ OpenGL ����
 */
class Shader {
public:

    /**
     * ��������ʱ�������׳��쳣�����ǻ����� errcode �� errmsg��
     * 
     * @param vertexShaderFilePath vertex shader �ļ�·����
     * @param fragmentShaderFilePath fragment shader �ļ�·����
     */
    Shader(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);

    
    Shader& resetErrCodeAndErrMsg();
    

    ~Shader() {};

    GLuint getId();
    GLuint setId(GLuint id);

    /**
     * ʹ����� shader ���� 
     */
    void use();

    /* ------------ ���� uniform ------------ */

    const Shader& setBool(const std::string& name, bool value) const;

    const Shader& setInt(const std::string& name, int value) const;

    const Shader& setFloat(const std::string& name, float value) const;

    const Shader& setMatrix4fv(
        const std::string& name, 
        const float* value, 
        GLsizei count = 1, 
        GLboolean transpose = GL_FALSE
    ) const;

    const Shader& setMatrix4fv(
        const std::string& name, 
        const glm::mat4& value, 
        GLsizei count = 1, 
        GLboolean transpose = GL_FALSE
    ) const;

    const Shader& setVector3f(
        const std::string& name, 
        const glm::vec3& vec,
        GLsizei count = 1
    ) const;

    const Shader& setVector3f(
        const std::string& name, 
        float x, float y, float z,
        GLsizei count = 1
    ) const;

public:

    /**
     * �����롣ִ��Σ�ղ������繹�죩��Ӧ����ֵ��
     */
    ShaderError errcode = ShaderError::UNINITIALIZED;

    /**
     * ������Ϣ��
     */
    std::string errmsg = "";

protected:

    // ��ֱֹ�ӹ���͸��ƹ��졣

    Shader() {}
    Shader(const Shader& shader) {}

    /**
     * ��ʼ����ʹ�ù��캯�����ô˺��������Ը����������;�˳���
     */
    void init(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);

protected:

    /**
     * shader ����id��
     */
    GLuint id = 0;
};
