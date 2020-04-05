#include <iostream>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLuint createShaderProgram(const char *vertexShaderSource, const char *fragmentShaderSource);
GLuint createTriangleVao(float vertices[]);
void checkShaderCompileSuccess(GLuint shader);
void checkProgramCompileSuccess(GLuint program);
void framebufferSizeCallback(GLFWwindow *window, int widht, int height);
void processInput(GLFWwindow *window);

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 pos;\n"
                                 "\n"
                                 "void main()\n"
                                 "{\n"
                                 "    gl_Position = vec4(pos, 1.0);\n"
                                 "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
                                   "uniform vec4 uniformColor;\n"
                                   "\n"
                                   "out vec4 color;\n"
                                   "\n"
                                   "void main()\n"
                                   "{\n"
                                   "    color = uniformColor;\n"
                                   "}\0";

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "E", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // set clear color (background color)
    // state setting, call once
    glClearColor(.8f, .3f, .3f, 1.0f);

    // create two triangles with one vao for each
    GLfloat triangleOne[] = {
        0.25f, 0.5f, 0.0f,
        0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f};

    GLuint vaoOne = createTriangleVao(triangleOne);

    GLfloat triangleTwo[] = {
        0.0f, 0.0f, 0.0f,
        -0.25f, 0.5f, 0.0f,
        -0.5f, 0.0f, 0.0f};

    GLuint vaoTwo = createTriangleVao(triangleTwo);

    // create shader programs and link shaders
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // render background
        glClear(GL_COLOR_BUFFER_BIT); // state using, uses the clearColor set earlier

        // update uniform color in fragment shader
        double time = glfwGetTime();
        GLfloat greenValue = (sin(time) / 2.0f) + 0.5f;
        int uniformLocation = glGetUniformLocation(shaderProgram, "uniformColor");

        // use our shader program
        glUseProgram(shaderProgram);

        // update uniform value of shader program currently in use
        glUniform4f(uniformLocation, 0.0f, greenValue, 0.0f, 0.0f);

        // draw the first triangle
        glBindVertexArray(vaoOne);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        // draw the second triangle
        glBindVertexArray(vaoTwo);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        // poll events and swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

GLuint createShaderProgram(const char *vertexShaderSource, const char *fragmentShaderSource)
{
    // vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompileSuccess(vertexShader);

    // fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompileSuccess(fragmentShader);

    // create shader program and link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkProgramCompileSuccess(shaderProgram);

    // delete shaders that have been linked to a program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

/**
 * vertices is currently expected to have a length of 9!
 */
GLuint createTriangleVao(GLfloat vertices[])
{
    // vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // vertex buffer object
    GLuint vbo;
    glGenBuffers(1, &vbo);
    // note: glBindBuffer does not affect the vao when binding to GL_ARRAY_BUFFER!
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // copy vertex data into buffer
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    // tell OpenGL how to interpret vertex data and enable it as the first attribute
    // only after this call will the bound vao be modified!
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);

    // we can unbind the buffer, since we just registered it to the vao
    // note: this is only allowed for GL_ARRAY_BUFFER, otherwise this would affect the vao state!
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // and we can also unbind the array object, since we finished setting it up
    glBindVertexArray(0);

    return vao;
}

void checkProgramCompileSuccess(GLuint program)
{
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "Program compilation failed\n"
                  << success
                  << infoLog << std::endl;
    }
}

void checkShaderCompileSuccess(GLuint shader)
{
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "Shader compilation failed\n"
                  << infoLog << std::endl;
    }
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}