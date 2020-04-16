#define GLFW_INCLUDE_NONE        // Hinder GLFW from including gl headers, since glad does that for us
#define STB_IMAGE_IMPLEMENTATION // stb_image.h one time initialization

#include <iostream>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "stb_image.h"

#include "Shader.h"

GLuint createTexture(const char *path, GLenum glTextureIndex, GLenum format, GLint wrappingMode);
void framebufferSizeCallback(GLFWwindow *window, int widht, int height);
void processInput(GLFWwindow *window, Shader &shader);

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
        // positions            colors              texture coords
         0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // top left
         0.5f,  0.5f, 0.0f,     1.0f, 1.0f, 0.0f,   1.0f, 1.0f  // top right
    };

    GLuint indices[] = {
        0, 1, 2,    // first triangle
        0, 2, 3     // second triangle
    };
    // clang-format on

    // create textures
    GLuint texture1 = createTexture("../textures/container.jpg", GL_TEXTURE0, GL_RGB, GL_REPEAT);
    GLuint texture2 = createTexture("../textures/awesomeface.png", GL_TEXTURE1, GL_RGBA, GL_REPEAT);

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);

    // color attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // texture coordinate attribute (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // we can unbind the buffer, since we just registered it to the vao
    // note: this is only allowed for GL_ARRAY_BUFFER, otherwise this would affect the vao state!
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // and we can also unbind the array object, since we finished setting it up
    glBindVertexArray(0);

    // create shader program
    // note: path assumes that binary is in a subfolder of the project (bin/)
    Shader shaderProgram("../shaders/transform.vert", "../shaders/mixTexturesConfigurable.frag");
    shaderProgram.setInt("texture1", 0);
    shaderProgram.setInt("texture2", 1);
    shaderProgram.setFloat("texture2Opacity", 0.2);

    // create reusable identity transformation matrix
    glm::mat4 identityMatrix(1.0f);

    while (!glfwWindowShouldClose(window))
    {
        // use our shader program
        shaderProgram.use();

        // input
        processInput(window, shaderProgram);

        // render background
        glClear(GL_COLOR_BUFFER_BIT); // state using, uses the clearColor set earlier

        // activate and bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // calculate transformation
        glm::mat4 trans = glm::translate(identityMatrix, glm::vec3(0.5f, -0.5f, 0.0f));
        trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
        shaderProgram.setFloat("transform", trans);

        // draw
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // poll events and swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

GLuint createTexture(const char *path, GLenum glTextureIndex, GLenum format, GLint wrappingMode)
{
    // make sure the image is loaded in a way that represents OpenGL texture coordinates
    stbi_set_flip_vertically_on_load(true);

    // read image into byte array
    int width, height, nrChannels;
    unsigned char *textureData = stbi_load(path, &width, &height, &nrChannels, 0);

    if (!textureData)
    {
        std::cerr << "Could not read texture from '" << path << "'" << std::endl;
        return -1;
    }

    // create texture
    GLuint texture;
    glGenTextures(1, &texture);

    // activate first texture unit and bind texture to it
    glActiveTexture(glTextureIndex);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set texture attributes (repeat and use linear filtering)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // copy texture data
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, textureData);

    // let OpenGL generate mipmaps for us
    glGenerateMipmap(GL_TEXTURE_2D);

    // free the texture data again
    stbi_image_free(textureData);

    return texture;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, Shader &shader)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        GLfloat opacity;
        shader.getFloat("texture2Opacity", &opacity);
        if (opacity < 1.0f)
        {
            opacity += 0.01f;
        }
        shader.setFloat("texture2Opacity", opacity);
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        GLfloat opacity;
        shader.getFloat("texture2Opacity", &opacity);
        if (opacity > 0.0f)
        {
            opacity -= 0.01f;
        }
        shader.setFloat("texture2Opacity", opacity);
    }
}