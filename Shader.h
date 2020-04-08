#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h"

#include <array>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    // constructor reads and builds the shader program
    Shader(const char *vertexShaderPath, const char *fragmentShaderPath);

    // use/activate the shader
    void use() const;

    // uniform functions
    void setBool(const std::string &name, bool v1) const;
    void setInt(const std::string &name, GLint v1) const;
    void setFloat(const std::string &name, GLfloat v1) const;
    void setBool(const std::string &name, bool v1, bool v2) const;
    void setInt(const std::string &name, GLint v1, GLint v2) const;
    void setFloat(const std::string &name, GLfloat v1, GLfloat v2) const;
    void setBool(const std::string &name, bool v1, bool v2, bool v3) const;
    void setInt(const std::string &name, GLint v1, GLint v2, GLint v3) const;
    void setFloat(const std::string &name, GLfloat v1, GLfloat v2, GLfloat v3) const;
    void setBool(const std::string &name, bool v1, bool v2, bool v3, bool v4) const;
    void setInt(const std::string &name, GLint v1, GLint v2, GLint v3, GLint v4) const;
    void setFloat(const std::string &name, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) const;
    void getBool(const std::string &name, bool *result) const;
    void getInt(const std::string &name, GLint *result) const;
    void getFloat(const std::string &name, GLfloat *result) const;

private:
    // shader program ID
    GLuint id;

    void checkProgramLinkSuccess(GLuint program) const;
    void checkShaderCompileSuccess(GLuint shader) const;
};

#endif