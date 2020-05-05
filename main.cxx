#define GLFW_INCLUDE_NONE        // Hinder GLFW from including gl headers, since glad does that for us
#define STB_IMAGE_IMPLEMENTATION // stb_image.h one time initialization

#include <iostream>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "stb_image.h"

#include "Camera.h"
#include "Shader.h"

// prototypes
GLuint createTexture(const char *path, GLenum glTextureIndex, GLenum format, GLint wrappingMode);
void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void mouseCallback(GLFWwindow *window, double xPos, double yPos);
void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);
void processInput(GLFWwindow *window);

// settings
const GLuint DEFAULT_WIDTH = 800;
const GLuint DEFAULT_HEIGHT = 600;

// time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

GLuint curWidth = DEFAULT_WIDTH;
GLuint curHeight = DEFAULT_HEIGHT;

// camera slightly off to the side and looking down from above
Camera camera(glm::vec3(1.0f, 1.0f, 6.0f), glm::vec3(0.0f, 1.0f, 0.0f), -10.0f, -100.0f);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "E", NULL, NULL);
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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // enable depth testing through z-buffer
    glEnable(GL_DEPTH_TEST);

    // set clear color (background color)
    // state setting, call once
    glClearColor(.8f, .3f, .3f, 1.0f);

    // create two triangles with one vao for each
    // clang-format off
    GLfloat vertices[] = {
        // position           normal
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
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

    // normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // we can unbind the buffer, since we just registered it to the vao
    // note: this is only allowed for GL_ARRAY_BUFFER, otherwise this would affect the vao state!
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // and we can also unbind the array object, since we finished setting it up
    glBindVertexArray(0);

    // create reusable identity transformation matrix
    glm::mat4 identityMatrix(1.0f);

    // variables for transformation matrices to convert between the different coordinate spaces
    glm::mat4 model;      // from local to world space
    glm::mat4 view;       // from world to view space
    glm::mat4 projection; // from view to clip space

    glm::vec3 lightPosition(1.2f, 1.0f, 2.0f);

    // create shader program
    // note: path assumes that binary is in a subfolder of the project (bin/)
    Shader lightingShader("../shaders/05_gouraudShading.vert", "../shaders/01_vertColor.frag");
    lightingShader.setFloat("objectColor", 1.0f, 0.5f, 0.31f);
    lightingShader.setFloat("lightColor", 1.0f, 1.0f, 1.0f);

    // set up light VAO
    GLuint lightVao;
    glGenVertexArrays(1, &lightVao);
    glBindVertexArray(lightVao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Shader lightSourceShader("../shaders/04_normalCorrected.vert", "../shaders/04_white.frag");

    while (!glfwWindowShouldClose(window))
    {
        // keep record of time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // render background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // calculate new view and projection
        view = camera.calculateView();
        projection = glm::perspective(glm::radians(camera.getFov()), (GLfloat)curWidth / (GLfloat)curHeight, 0.1f, 100.0f);

        // move light dynamically
        lightPosition = glm::vec3(sin(currentFrame) * 3.0f, cos(currentFrame) * 3.0f, sin(currentFrame) * 3.0f + 2.0f);

        // calculate light position in view space, for shader
        glm::vec3 lightViewPosition = glm::vec3(view * glm::vec4(lightPosition, 1.0));

        // update object shader
        lightingShader.use();
        lightingShader.setFloat("view", view);
        lightingShader.setFloat("projection", projection);
        lightingShader.setFloat("lightViewPosition", lightViewPosition);

        // draw lit object in center
        glBindVertexArray(vao);
        model = identityMatrix;
        lightingShader.setFloat("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // update light shader
        lightSourceShader.use();
        lightSourceShader.setFloat("view", view);
        lightSourceShader.setFloat("projection", projection);

        // draw light source
        glBindVertexArray(lightVao);
        model = glm::translate(identityMatrix, lightPosition);
        model = glm::scale(model, glm::vec3(0.2f));
        lightSourceShader.setFloat("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
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
    curWidth = width;
    curHeight = height;
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow *window, double xPos, double yPos)
{
    camera.rotate(xPos, yPos);
}

void scrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
    camera.zoom(yOffset);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // Movements will be constantly interrupted, because key state changes from GLFW_PRESS to GLFW_REPEAT
    // TODO: Store button states, for when they're held down or switch to event based input handling
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.move(FORWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.move(BACKWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.move(LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.move(RIGHT, deltaTime);
    }
}