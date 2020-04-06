#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    // shader program ID
    GLuint id;

    // constructor reads and builds the shader program
    Shader(const char *vertexShaderPath, const char *fragmentShaderPath);

    // use/activate the shader
    void use();

    // uniform functions
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, GLint value) const;
    void setFloat(const std::string &name, GLfloat value) const;

private:
    void checkProgramLinkSuccess(GLuint program) const;
    void checkShaderCompileSuccess(GLuint shader) const;
};

#endif