#define GLFW_INCLUDE_NONE        // Hinder GLFW from including gl headers, since glad does that for us
#define STB_IMAGE_IMPLEMENTATION // stb_image.h one time initialization

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "lib/glad/include/glad/glad.h"
#include "lib/stb_image.h"

// imgui
#include "lib/imgui/imgui.h"
#include "lib/imgui/imgui_impl_glfw.h"
#include "lib/imgui/imgui_impl_opengl3.h"

#include "DataDirHelper.h"
#include "Camera.h"
#include "Shader.h"

// prototypes
void init();
void initGlfw();
void initGlad();
void initGl();
void initImgui();
void deInit();
GLuint createTexture(const std::string &path, GLenum glTextureIndex, GLint wrappingMode);
void moveCamera();
void drawImgui();
void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void mouseCallback(GLFWwindow *window, double xPos, double yPos);
void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);
void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void errorCallback(int error, const char *description);

// imgui state
struct
{
    bool showMainWindow;
    bool showDemoWindow;
} imguiState;

// settings
const GLuint DEFAULT_WIDTH = 1280;
const GLuint DEFAULT_HEIGHT = 720;

GLFWwindow *window;
GLuint curWidth = DEFAULT_WIDTH;
GLuint curHeight = DEFAULT_HEIGHT;

// time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// camera slightly off to the side and looking down from above
Camera camera(glm::vec3(1.0f, 1.0f, 6.0f), glm::vec3(0.0f, 1.0f, 0.0f), -10.0f, -100.0f);

// this map will store the state for each key being pressed on the keyboard
// TODO: Replace this with a more flexible keyboard/input handling class at some point
std::unordered_map<int, bool> keyStates;

int main()
{
    init();
    DataDirHelper &dataDirHelper = DataDirHelper::getInstance();

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
    GLuint diffuseMap = createTexture(dataDirHelper.locate("textures/container2.png"), GL_TEXTURE0, GL_REPEAT);
    GLuint specularMap = createTexture(dataDirHelper.locate("textures/container2_specular.png"), GL_TEXTURE1, GL_REPEAT);
    GLuint emissionMap = createTexture(dataDirHelper.locate("textures/matrix.jpg"), GL_TEXTURE2, GL_REPEAT);

    // create shader programs
    // note: path assumes that binary is in a subfolder of the project (bin/)
    Shader lightingShader(dataDirHelper.locate("shaders/06_normalTexCoord.vert"), dataDirHelper.locate("shaders/06_multipleLights.frag"));
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);
    lightingShader.setInt("material.emission", 2);
    lightingShader.setFloat("material.shininess", 32.0f);

    // directional light
    lightingShader.setFloat("directionalLight.direction", -0.2f, -1.0f, -0.3f);
    lightingShader.setFloat("directionalLight.ambient", 0.02f, 0.02f, 0.02f);
    lightingShader.setFloat("directionalLight.diffuse", 0.08f, 0.08f, 0.08f);
    lightingShader.setFloat("directionalLight.specular", 0.3f, 0.3f, 0.3f);

    // point lights
    for (int i = 0; i < sizeof(pointLightPositions) / sizeof(glm::vec3); i++)
    {
        std::ostringstream pointLightIdentifier;
        pointLightIdentifier << "pointLights[" << i << "]";
        std::string pointLight = pointLightIdentifier.str();
        lightingShader.setFloat(pointLight + ".ambient", 0.03f, 0.008f, 0.0f);
        lightingShader.setFloat(pointLight + ".diffuse", 0.2f, 0.05f, 0.0f);
        lightingShader.setFloat(pointLight + ".specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat(pointLight + ".constant", 1.0f);
        lightingShader.setFloat(pointLight + ".linear", 0.14f);
        lightingShader.setFloat(pointLight + ".quadratic", 0.07f);
    }

    // spotlight
    // we are simulating a flashlight that's shining from the player's viewpoint
    glm::vec3 spotLightViewPosition(0.0f, 0.0f, 0.0f);
    glm::vec3 spotLightViewDirection(0.0f, 0.0f, -1.0f);
    lightingShader.setFloat("spotLight.position", spotLightViewPosition);
    lightingShader.setFloat("spotLight.direction", spotLightViewDirection);
    lightingShader.setFloat("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    lightingShader.setFloat("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("spotLight.specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("spotLight.constant", 1.0f);
    lightingShader.setFloat("spotLight.linear", 0.09f);
    lightingShader.setFloat("spotLight.quadratic", 0.032f);
    lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f))); // going to be used for dot product, so use cos
    lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

    Shader lightSourceShader(dataDirHelper.locate("shaders/04_normalCorrected.vert"), dataDirHelper.locate("shaders/04_color.frag"));
    lightSourceShader.setFloat("iColor", 0.2f, 0.05f, 0.0f);

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
        glfwPollEvents();

        // keep record of time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        moveCamera();

        // render background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // calculate new view and projection
        view = camera.calculateView();
        projection = glm::perspective(glm::radians(camera.getFov()), (GLfloat)curWidth / (GLfloat)curHeight, 0.1f, 100.0f);

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

        // bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, emissionMap);

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

        drawImgui();

        // swap buffers
        glfwSwapBuffers(window);
    }

    return 0;
}

void init()
{
    initGlfw();
    initGlad();
    initGl();
    initImgui();
}

void initGlfw()
{
    glfwSetErrorCallback(errorCallback);
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(-1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "E", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // TODO
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyboardCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // enable vsync
    glfwSwapInterval(1);
}

void initGlad()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }
}

void initGl()
{
    glViewport(0, 0, curWidth, curHeight);

    // enable depth testing through z-buffer
    glEnable(GL_DEPTH_TEST);

    // set clear color (background color)
    // state setting, call once
    glClearColor(.01f, .01f, .01f, 1.0f);
}

void initImgui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void deInit()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

GLuint createTexture(const std::string &path, GLenum glTextureUnit, GLint wrappingMode)
{
    // make sure the image is loaded in a way that represents OpenGL texture coordinates
    stbi_set_flip_vertically_on_load(true);

    // read image into byte array
    int width, height, nrChannels;
    unsigned char *textureData = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

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

void moveCamera()
{
    if (keyStates[GLFW_KEY_W])
    {
        camera.move(CameraDirection::FORWARD, deltaTime);
    }

    if (keyStates[GLFW_KEY_S])
    {
        camera.move(CameraDirection::BACKWARD, deltaTime);
    }

    if (keyStates[GLFW_KEY_A])
    {
        camera.move(CameraDirection::LEFT, deltaTime);
    }

    if (keyStates[GLFW_KEY_D])
    {
        camera.move(CameraDirection::RIGHT, deltaTime);
    }
}

void drawImgui()
{
    // depending on if any window is visible we need to either show or hide the cursor
    int currentInputMode = glfwGetInputMode(window, GLFW_CURSOR);
    bool imguiVisible =
        imguiState.showMainWindow || imguiState.showDemoWindow;

    if (imguiVisible && currentInputMode != GLFW_CURSOR_NORMAL)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else if (!imguiVisible && currentInputMode != GLFW_CURSOR_DISABLED)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    // start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // show the main debug window
    if (imguiState.showMainWindow)
    {
        ImGui::Begin("Main", &imguiState.showMainWindow);

        if (ImGui::Button("Show demo window"))
        {
            imguiState.showDemoWindow = !imguiState.showDemoWindow;
        }

        if (ImGui::Button("Quit"))
        {
            glfwSetWindowShouldClose(window, true);
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // show the integrated demo window of imgui
    if (imguiState.showDemoWindow)
    {
        ImGui::ShowDemoWindow(&imguiState.showDemoWindow);
    }

    // draw imgui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    curWidth = width;
    curHeight = height;
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow *window, double xPos, double yPos)
{
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        camera.rotate(xPos, yPos);
    }
}

void scrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        camera.zoom(yOffset);
    }
}

void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        imguiState.showMainWindow = !imguiState.showMainWindow;
    }
    else
    {
        keyStates[key] = action != GLFW_RELEASE; // we want to treat GLFW_PRESS and GLFW_REPEAT as the same
    }
}

void errorCallback(int error, const char *description)
{
    std::cerr << "Glfw Error " << error << ": " << description << std::endl;
}