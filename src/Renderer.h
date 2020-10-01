#ifndef RENDERER_H
#define RENDERER_H

#define GLFW_INCLUDE_NONE        // Hinder GLFW from including gl headers, since glad does that for us
#define STB_IMAGE_IMPLEMENTATION // stb_image.h one time initialization

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "lib/glad/include/glad/glad.h"

#include "Camera.h"
#include "Shader.h"

class Renderer
{
private:
    void initGlfw();
    void initGlad();
    void initGl();
    void initImgui();
    void initScene();

    GLuint createTexture(const std::string &path, GLenum glTextureIndex, GLint wrappingMode);

    void moveCamera();
    void drawScene();
    void drawImgui();

    void framebufferSizeCallback(GLFWwindow *window, int width, int height);
    void mouseCallback(GLFWwindow *window, double xPos, double yPos);
    void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);
    void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void errorCallback(int error, const char *description);

    // settings
    static const GLuint DEFAULT_WIDTH = 1280;
    static const GLuint DEFAULT_HEIGHT = 720;

    // reusable identity transformation matrix
    static const glm::mat4 identityMatrix;

    // imgui state
    struct
    {
        bool showMainWindow = false;
        bool showDemoWindow = false;
    } imguiState;

    GLFWwindow *window;
    GLuint curWidth;
    GLuint curHeight;

    // time
    float deltaTime{0.0f};
    float lastFrame{0.0f};

    Camera camera;

    // this map will store the state for each key being pressed on the keyboard
    // TODO: Replace this with a more flexible keyboard/input handling class at some point
    std::unordered_map<int, bool> keyStates;

    // variables for transformation matrices to convert between the different coordinate spaces
    glm::mat4 model;      // from local to world space
    glm::mat4 view;       // from world to view space
    glm::mat4 projection; // from view to clip space

    std::unique_ptr<Shader> lightingShader;
    std::unique_ptr<Shader> lightSourceShader;

    std::vector<glm::vec3> cubePositions;
    std::vector<glm::vec3> pointLightPositions;

    GLuint vao;
    GLuint lightVao;

    // textures
    GLuint diffuseMap;
    GLuint specularMap;
    GLuint emissionMap;

public:
    Renderer();
    virtual ~Renderer();

    bool isRunning() const;
    void renderFrame();
};

#endif