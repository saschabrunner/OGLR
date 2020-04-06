#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"

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

    // set clear color (background color)
    // state setting, call once
    glClearColor(.8f, .3f, .3f, 1.0f);

    // create two triangles with one vao for each
    // clang-format off
    GLfloat vertices[] = {
        // positions            colors
         0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,   // bottom left
         0.0f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f    // top
    };
    // clang-format on

    // vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // vertex buffer object
    GLuint vbo;
    glGenBuffers(1, &vbo);
    // note: glBindBuffer does not affect the vao when binding to GL_ARRAY_BUFFER! glVertexAttribPointer will!
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // copy vertex data into buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);

    // color attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // we can unbind the buffer, since we just registered it to the vao
    // note: this is only allowed for GL_ARRAY_BUFFER, otherwise this would affect the vao state!
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // and we can also unbind the array object, since we finished setting it up
    glBindVertexArray(0);

    // create shader program
    // note: path assumes that binary is in a subfolder of the project (bin/)
    Shader shaderProgram("../shaders/offset.vert", "../shaders/vertColor.frag");
    shaderProgram.setFloat3("offset", .25f, .55f, 0.0f);

    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // render background
        glClear(GL_COLOR_BUFFER_BIT); // state using, uses the clearColor set earlier

        // use our shader program
        shaderProgram.use();

        // draw the triangle
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        // poll events and swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
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