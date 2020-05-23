#define GLFW_INCLUDE_NONE        // Hinder GLFW from including gl headers, since glad does that for us
#define STB_IMAGE_IMPLEMENTATION // stb_image.h one time initialization

#include <iostream>
#include <sstream>
#include <string>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "stb_image.h"

#include "Camera.h"
#include "Shader.h"

// prototypes
GLuint createTexture(const char *path, GLenum glTextureIndex, GLint wrappingMode);
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
    glClearColor(.8f, .8f, .8f, 1.0f);

    // create two triangles with one vao for each
    // clang-format off
    GLfloat vertices[] = {
        // position           normal               texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    glm::vec3 pointLightPositions[] = {
        glm::vec3( 0.7f,  0.2f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f)
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);

    // normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // texture coordinates (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

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

    // Load diffuse map texture
    GLuint diffuseMap = createTexture("../textures/container2.png", GL_TEXTURE0, GL_REPEAT);
    GLuint specularMap = createTexture("../textures/container2_specular.png", GL_TEXTURE1, GL_REPEAT);
    GLuint emissionMap = createTexture("../textures/matrix.jpg", GL_TEXTURE2, GL_REPEAT);

    // create shader programs
    // note: path assumes that binary is in a subfolder of the project (bin/)
    Shader lightingShader("../shaders/06_normalTexCoord.vert", "../shaders/06_multipleLights.frag");
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);
    lightingShader.setInt("material.emission", 2);
    lightingShader.setFloat("material.shininess", 32.0f);

    // directional light
    lightingShader.setFloat("directionalLight.direction", -0.2f, -1.0f, -0.3f);
    lightingShader.setFloat("directionalLight.ambient", 0.2f, 0.2f, 0.2f);
    lightingShader.setFloat("directionalLight.diffuse", 0.5f, 0.5f, 0.5f);
    lightingShader.setFloat("directionalLight.specular", 1.0f, 1.0f, 1.0f);

    // point lights
    for (int i = 0; i < sizeof(pointLightPositions) / sizeof(glm::vec3); i++)
    {
        std::ostringstream pointLightIdentifier;
        pointLightIdentifier << "pointLights[" << i << "]";
        std::string pointLight = pointLightIdentifier.str();
        lightingShader.setFloat(pointLight + ".ambient", 0.06f, 0.2f, 0.0f);
        lightingShader.setFloat(pointLight + ".diffuse", 0.12f, 0.4f, 0.0f);
        lightingShader.setFloat(pointLight + ".specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat(pointLight + ".constant", 1.0f);
        lightingShader.setFloat(pointLight + ".linear", 0.09f);
        lightingShader.setFloat(pointLight + ".quadratic", 0.032f);
    }

    // spotlight
    // we are simulating a flashlight that's shining from the player's viewpoint
    glm::vec3 spotLightViewPosition(0.0f, 0.0f, 0.0f);
    glm::vec3 spotLightViewDirection(0.0f, 0.0f, -1.0f);
    lightingShader.setFloat("spotLight.position", spotLightViewPosition);
    lightingShader.setFloat("spotLight.direction", spotLightViewDirection);
    lightingShader.setFloat("spotLight.ambient", 0.2f, 0.2f, 0.2f);
    lightingShader.setFloat("spotLight.diffuse", 0.5f, 0.5f, 0.5f);
    lightingShader.setFloat("spotLight.specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("spotLight.constant", 1.0f);
    lightingShader.setFloat("spotLight.linear", 0.09f);
    lightingShader.setFloat("spotLight.quadratic", 0.032f);
    lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f))); // going to be used for dot product, so use cos
    lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

    Shader lightSourceShader("../shaders/04_normalCorrected.vert", "../shaders/04_color.frag");
    lightSourceShader.setFloat("iColor", 0.239f, 0.8f, 0.0f);

    // set up light VAO
    GLuint lightVao;
    glGenVertexArrays(1, &lightVao);
    glBindVertexArray(lightVao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window))
    {
        // keep record of time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        std::cout << "frame start " << currentFrame << std::endl;

        // input
        processInput(window);

        std::cout << "processed input" << std::endl;

        // render background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // calculate new view and projection
        view = camera.calculateView();
        projection = glm::perspective(glm::radians(camera.getFov()), (GLfloat)curWidth / (GLfloat)curHeight, 0.1f, 100.0f);
        std::cout << "transformations done" << std::endl;

        // update object shader
        lightingShader.use();
        lightingShader.setFloat("view", view);
        lightingShader.setFloat("projection", projection);
        lightingShader.setFloat("material.emissionVerticalOffset", -glfwGetTime() / 5.0);

        for (int i = 0; i < sizeof(pointLightPositions) / sizeof(glm::vec3); i++)
        {
            std::ostringstream pointLightIdentifier;
            pointLightIdentifier << "pointLights[" << i << "].position";
            glm::vec3 viewPosition = glm::vec3(view * glm::vec4(pointLightPositions[i], 1.0));
            lightingShader.setFloat(pointLightIdentifier.str(), viewPosition);
        }

        std::cout << "object shader uniforms set" << std::endl;

        // bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, emissionMap);

        std::cout << "bound texture" << std::endl;

        // draw cubes
        glBindVertexArray(vao);
        for (int i = 0; i < sizeof(cubePositions) / sizeof(glm::vec3); i++)
        {
            model = glm::translate(identityMatrix, cubePositions[i]);
            model = glm::rotate(model, (float)glfwGetTime() * glm::radians(20.0f * i), glm::vec3(1.0f, 0.3f, 0.5f));
            lightingShader.setFloat("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        std::cout << "object(s) drawn" << std::endl;

        // update light shader
        lightSourceShader.use();
        lightSourceShader.setFloat("view", view);
        lightSourceShader.setFloat("projection", projection);

        // draw light sources
        glBindVertexArray(lightVao);
        for (int i = 0; i < sizeof(pointLightPositions) / sizeof(glm::vec3); i++)
        {
            model = glm::translate(identityMatrix, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            lightSourceShader.setFloat("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        // poll events and swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
        std::cout << "frame end" << std::endl;
    }

    glfwTerminate();
    return 0;
}

GLuint createTexture(const char *path, GLenum glTextureUnit, GLint wrappingMode)
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

    GLenum format;

    if (nrChannels == 1)
    {
        format = GL_RED;
    }
    else if (nrChannels == 3)
    {
        format = GL_RGB;
    }
    else if (nrChannels == 4)
    {
        format = GL_RGBA;
    }
    else
    {
        std::cerr << "Unexpected number of channels: " << nrChannels << std::endl;
        return -1;
    }

    // create texture
    GLuint texture;
    glGenTextures(1, &texture);

    // activate first texture unit and bind texture to it
    glActiveTexture(glTextureUnit);
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