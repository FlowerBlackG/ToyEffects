/*
 * Shader 程序封装类。
 * 适用于 OpenGL 程序。
 */

#pragma once
#include "ToyGraphCommon/EngineCompileOptions.h"

 // glad & glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// stl
#include <string>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
 * Shader 程序封装类。 
 * 适用于 OpenGL 程序。
 */
class Shader {
public:

    Shader() {};

    /**
     * 遇到错误时，不会抛出异常，但是会设置 errcode 和 errmsg。
     * 
     * @param vertexShaderFilePath vertex shader 文件路径。
     * @param fragmentShaderFilePath fragment shader 文件路径。
     */
    Shader(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);

    
    Shader& resetErrCodeAndErrMsg();
    

    ~Shader() {};

    GLuint getId();
    GLuint setId(GLuint id);

    /**
     * 使用这个 shader 程序。 
     */
    void use();

    /* ------------ 设置 uniform ------------ */

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
    //乐 直接加在.h里了
    const Shader& setVector2f(
        const std::string& name, 
        float x, float y,
        GLsizei count = 1) const
    {
        glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
        return *this;
    }
public:

    /**
     * 错误码。执行危险操作（如构造）后应检查此值。
     */
    ShaderError errcode = ShaderError::UNINITIALIZED;

    /**
     * 错误信息。
     */
    std::string errmsg = "";

protected:

    /**
     * 初始化。使用构造函数调用此函数，可以更方便地在中途退出。
     */
    void init(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);

protected:

    /**
     * shader 程序id。
     */
    GLuint id = 0;
};
