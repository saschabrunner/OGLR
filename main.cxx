#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void checkShaderCompileSuccess(GLuint shader);
void framebufferSizeCallback(GLFWwindow *window, int widht, int height);
void processInput(GLFWwindow *window);

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

    // state setting, call once
    glClearColor(.8f, .3f, .3f, 1.0f);

    // vertices for a triangle
    GLfloat vertices[] = {
        -0.5f,
        -0.5f,
        0.0f,
        0.5f,
        -0.5f,
        0.0f,
        0.0f,
        0.5f,
        0.0f};

    // vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // vertex buffer object (will be added to bound vao)
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // copy vertex data into buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // tell OpenGL how to interpret vertex data and enable it as the first attribute (will also be stored in bound vao)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);

    // vertex shader
    const char *vertexShaderSource = "#version 330 core\n"
                                     "layout (location = 0) in vec3 pos;\n"
                                     "\n"
                                     "void main()\n"
                                     "{\n"
                                     "    gl_Position = vec4(pos, 1.0);\n"
                                     "}\0";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompileSuccess(vertexShader);

    // fragment shader
    const char *fragmentShaderSource = "#version 330 core\n"
                                       "out vec4 color;\n"
                                       "\n"
                                       "void main()\n"
                                       "{\n"
                                       "    color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                                       "}\0";

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompileSuccess(fragmentShader);

    // create shader program and link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // delete shaders that have been linked to a program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // rendering
        // state using, uses the clearColor set earlier
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        // TODO: draw triangle

        // poll events and swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void checkProgramCompileSuccess(GLuint program)
{
    GLint success;
    glGetProgramiv(program, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "Shader compilation failed\n"
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