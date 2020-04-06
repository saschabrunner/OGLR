#include "Shader.h"

Shader::Shader(const char *vertexShaderPath, const char *fragmentShaderPath)
{
    // retrieve shader source from file system
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    // ifstream requires you to define what should throw an exception
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        vShaderFile.open(vertexShaderPath);
        fShaderFile.open(fragmentShaderPath);

        // read files
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        // close files
        vShaderFile.close();
        fShaderFile.close();

        // convert streams into C strings
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (const std::ifstream::failure &e)
    {
        std::cerr << e.what() << '\n';
    }

    const char *vertexCodeC = vertexCode.c_str();
    const char *fragmentCodeC = fragmentCode.c_str();

    // vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexCodeC, NULL);
    glCompileShader(vertexShader);
    checkShaderCompileSuccess(vertexShader);

    // fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentCodeC, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompileSuccess(fragmentShader);

    // linking
    id = glCreateProgram();
    glAttachShader(id, vertexShader);
    glAttachShader(id, fragmentShader);
    glLinkProgram(id);
    checkProgramLinkSuccess(id);

    // delete linked shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::use()
{
    glUseProgram(id);
}

void Shader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, GLint value) const
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, GLfloat value) const
{
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::checkProgramLinkSuccess(GLuint program) const
{
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Program linking failed\n"
                  << success
                  << infoLog << std::endl;
    }
}

void Shader::checkShaderCompileSuccess(GLuint shader) const
{
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed\n"
                  << infoLog << std::endl;
    }
}