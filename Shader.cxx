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

void Shader::use() const
{
    glUseProgram(id);
}

// OpenGL 4.1 added glProgramUniform, which doesn't require you to use the program before setting a uniform
// This might perform better than manually calling glUseProgram before setting uniforms, as below
// Can be implemented when switching to OpenGL >= 4.1

void Shader::setBool1(const std::string &name, bool v1) const
{
    use();
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)v1);
}

void Shader::setInt1(const std::string &name, GLint v1) const
{
    use();
    glUniform1i(glGetUniformLocation(id, name.c_str()), v1);
}

void Shader::setFloat1(const std::string &name, GLfloat v1) const
{
    use();
    glUniform1f(glGetUniformLocation(id, name.c_str()), v1);
}

void Shader::setBool2(const std::string &name, bool v1, bool v2) const
{
    use();
    glUniform2i(glGetUniformLocation(id, name.c_str()), (int)v1, (int)v2);
}

void Shader::setInt2(const std::string &name, GLint v1, GLint v2) const
{
    use();
    glUniform2i(glGetUniformLocation(id, name.c_str()), v1, v2);
}

void Shader::setFloat2(const std::string &name, GLfloat v1, GLfloat v2) const
{
    use();
    glUniform2f(glGetUniformLocation(id, name.c_str()), v1, v2);
}

void Shader::setBool3(const std::string &name, bool v1, bool v2, bool v3) const
{
    use();
    glUniform3i(glGetUniformLocation(id, name.c_str()), (int)v1, (int)v2, (int)v3);
}

void Shader::setInt3(const std::string &name, GLint v1, GLint v2, GLint v3) const
{
    use();
    glUniform3i(glGetUniformLocation(id, name.c_str()), v1, v2, v3);
}

void Shader::setFloat3(const std::string &name, GLfloat v1, GLfloat v2, GLfloat v3) const
{
    use();
    glUniform3f(glGetUniformLocation(id, name.c_str()), v1, v2, v3);
}

void Shader::setBool4(const std::string &name, bool v1, bool v2, bool v3, bool v4) const
{
    use();
    glUniform4i(glGetUniformLocation(id, name.c_str()), (int)v1, (int)v2, (int)v3, (int)v4);
}

void Shader::setInt4(const std::string &name, GLint v1, GLint v2, GLint v3, GLint v4) const
{
    use();
    glUniform4i(glGetUniformLocation(id, name.c_str()), v1, v2, v3, v4);
}

void Shader::setFloat4(const std::string &name, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) const
{
    use();
    glUniform4f(glGetUniformLocation(id, name.c_str()), v1, v2, v3, v4);
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