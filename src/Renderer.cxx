#include "Renderer.h"

#define GLFW_INCLUDE_NONE        // Hinder GLFW from including gl headers, since glad does that for us
#define STB_IMAGE_IMPLEMENTATION // stb_image.h one time initialization

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#include "lib/glad/include/glad/glad.h"

#include "lib/stb_image.h"

// imgui
#include "lib/imgui/imgui.h"
#include "lib/imgui/imgui_impl_glfw.h"
#include "lib/imgui/imgui_impl_opengl3.h"

#include "Camera.h"
#include "DirectoryHelper.h"
#include "Shader.h"

namespace
{
    // settings
    const GLuint DEFAULT_WIDTH{1280};
    const GLuint DEFAULT_HEIGHT{720};

    // reusable identity transformation matrix
    const glm::mat4 identityMatrix(1.0);

    glm::vec4 clearColor{0.3f, 0.1f, 0.0f, 1.0f};

    // imgui state
    struct
    {
        bool showMainWindow{false};
        bool showDemoWindow{false};
    } imguiState;

    // shader uniforms state
    struct
    {
        float shininess{32.0f};
    } material;

    struct
    {
        glm::vec3 direction;                           // direction of light in view space (calculated every frame)
        glm::vec3 worldDirection{-0.2f, -1.0f, -0.3f}; // direction of light in world space
        glm::vec3 ambient{0.02f, 0.02f, 0.02f};
        glm::vec3 diffuse{0.08f, 0.08f, 0.08f};
        glm::vec3 specular{0.3f, 0.3f, 0.3f};
    } directionalLight;

    struct
    {
        glm::vec3 cubeColor{0.2f, 0.05f, 0.0f}; // color it is represented with in the scene
        glm::vec3 ambient{0.03f, 0.008f, 0.0f};
        glm::vec3 diffuse{0.2f, 0.05f, 0.0f};
        glm::vec3 specular{1.0f, 1.0f, 1.0f};
        float constant{1.0f};
        float linear{0.14f};
        float quadratic{0.07f};
    } pointLight;

    struct
    {
        glm::vec3 position{0.0f, 0.0f, 0.0f};
        glm::vec3 direction{0.0f, 0.0f, -1.0f}; // direction of spotlight in view space
        glm::vec3 ambient{0.0f, 0.0f, 0.0f};
        glm::vec3 diffuse{1.0f, 1.0f, 1.0f};
        glm::vec3 specular{1.0f, 1.0f, 1.0f};
        float constant{1.0f};
        float linear{0.09f};
        float quadratic{0.032f};
        float cutOff{glm::cos(glm::radians(12.5f))}; // used for dot product, so use cos
        float outerCutOff{glm::cos(glm::radians(15.0f))};
    } spotLight;

    GLFWwindow *window;
    GLuint curWidth{DEFAULT_WIDTH};
    GLuint curHeight{DEFAULT_HEIGHT};

    // time
    float deltaTime{0.0f};
    float lastFrame{0.0f};

    // this map will store the state for each key being pressed on the keyboard
    // TODO: Replace this with a more flexible keyboard/input handling class at some point
    std::unordered_map<int, bool> keyStates;

    // variables for transformation matrices to convert between the different coordinate spaces
    glm::mat4 model;      // from local to world space
    glm::mat4 view;       // from world to view space
    glm::mat4 projection; // from view to clip space

    std::unique_ptr<Camera> camera;
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

    // prototypes
    int initGlfw();
    int initGlad();
    void initGl();
    void initImgui();
    void initScene();

    GLuint createTexture(const std::string &path, GLenum glTextureIndex, GLint wrappingMode);

    void moveCamera();
    void drawScene();
    void drawImgui();

    template <class T>
    void updatePointLightAttribute(std::string attribute, T &value);

    void framebufferSizeCallback(GLFWwindow *window, int width, int height);
    void mouseCallback(GLFWwindow *window, double xPos, double yPos);
    void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);
    void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    void errorCallback(int error, const char *description);

    int initGlfw()
    {
        glfwSetErrorCallback(errorCallback);
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return Renderer::INIT_FAIL_GLFW_INIT;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "E", NULL, NULL);
        if (window == NULL)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return Renderer::INIT_FAIL_GLFW_CREATE_WINDOW;
        }
        glfwMakeContextCurrent(window);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        glfwSetCursorPosCallback(window, mouseCallback);
        glfwSetScrollCallback(window, scrollCallback);
        glfwSetKeyCallback(window, keyboardCallback);
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

        // enable vsync
        glfwSwapInterval(1);
        return 0;
    }

    int initGlad()
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return Renderer::INIT_FAIL_GLAD;
        }
        return 0;
    }

    void initGl()
    {
        glViewport(0, 0, curWidth, curHeight);

        // enable depth testing through z-buffer
        glEnable(GL_DEPTH_TEST);

        // set clear color (background color)
        // state setting, call once
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    }

    void initImgui()
    {
        // create imgui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // setup ini path
        static std::string iniPath = DirectoryHelper::getInstance().locateConfig("imgui.ini", true);
        ImGui::GetIO().IniFilename = iniPath.c_str();

        // configure style
        ImGui::StyleColorsDark();

        // setup platform/renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    void initScene()
    {
        DirectoryHelper &directoryHelper = DirectoryHelper::getInstance();

        // create two triangles with one vao for each
        // clang-format off
        GLfloat vertices[] = {
            // position           normal               texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            -0.5f , 0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
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

        cubePositions = {
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

        pointLightPositions = {
            glm::vec3( 0.7f,  0.2f,  2.0f),
            glm::vec3( 2.3f, -3.3f, -4.0f),
            glm::vec3(-4.0f,  2.0f, -12.0f),
            glm::vec3( 0.0f,  0.0f, -3.0f)
        };
        // clang-format on

        // vertex array object
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

        // create textures
        diffuseMap = createTexture(
            directoryHelper.locateData("textures/container2.png"),
            GL_TEXTURE0,
            GL_REPEAT);
        specularMap = createTexture(
            directoryHelper.locateData("textures/container2_specular.png"),
            GL_TEXTURE1,
            GL_REPEAT);
        emissionMap = createTexture(
            directoryHelper.locateData("textures/matrix.jpg"),
            GL_TEXTURE2,
            GL_REPEAT);

        // configure shader programs
        lightingShader = std::unique_ptr<Shader>(new Shader(
            directoryHelper.locateData("shaders/06_normalTexCoord.vert"),
            directoryHelper.locateData("shaders/06_multipleLights.frag")));
        lightingShader->setInt("material.diffuse", 0);
        lightingShader->setInt("material.specular", 1);
        lightingShader->setInt("material.emission", 2);
        lightingShader->setFloat("material.shininess", material.shininess);

        // directional light
        lightingShader->setFloat("directionalLight.ambient", directionalLight.ambient);
        lightingShader->setFloat("directionalLight.diffuse", directionalLight.diffuse);
        lightingShader->setFloat("directionalLight.specular", directionalLight.specular);

        // point lights
        for (std::size_t i = 0; i < pointLightPositions.size(); i++)
        {
            std::ostringstream pointLightIdentifier;
            pointLightIdentifier << "pointLights[" << i << "]";
            std::string pointLightIdentifierStr = pointLightIdentifier.str();
            lightingShader->setFloat(pointLightIdentifierStr + ".ambient", pointLight.ambient);
            lightingShader->setFloat(pointLightIdentifierStr + ".diffuse", pointLight.diffuse);
            lightingShader->setFloat(pointLightIdentifierStr + ".specular", pointLight.specular);
            lightingShader->setFloat(pointLightIdentifierStr + ".constant", pointLight.constant);
            lightingShader->setFloat(pointLightIdentifierStr + ".linear", pointLight.linear);
            lightingShader->setFloat(pointLightIdentifierStr + ".quadratic", pointLight.quadratic);
        }

        // spotlight
        // we are simulating a flashlight that's shining from the player's viewpoint
        lightingShader->setFloat("spotLight.position", spotLight.position);
        lightingShader->setFloat("spotLight.direction", spotLight.direction);
        lightingShader->setFloat("spotLight.ambient", spotLight.ambient);
        lightingShader->setFloat("spotLight.diffuse", spotLight.diffuse);
        lightingShader->setFloat("spotLight.specular", spotLight.specular);
        lightingShader->setFloat("spotLight.constant", spotLight.constant);
        lightingShader->setFloat("spotLight.linear", spotLight.linear);
        lightingShader->setFloat("spotLight.quadratic", spotLight.quadratic);
        lightingShader->setFloat("spotLight.cutOff", spotLight.cutOff);
        lightingShader->setFloat("spotLight.outerCutOff", spotLight.outerCutOff);

        // shader for the light source cubes
        lightSourceShader = std::unique_ptr<Shader>(new Shader(
            DirectoryHelper::getInstance().locateData("shaders/04_normalCorrected.vert"),
            DirectoryHelper::getInstance().locateData("shaders/04_color.frag")));
        lightSourceShader->setFloat("iColor", pointLight.cubeColor);

        // set up light VAO
        glGenVertexArrays(1, &lightVao);
        glBindVertexArray(lightVao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // camera slightly off to the side and looking down from above
        camera = std::unique_ptr<Camera>(new Camera(
            glm::vec3(1.0f, 1.0f, 6.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            -10.0f,
            -100.0f));
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
            camera->move(CameraDirection::FORWARD, deltaTime);
        }

        if (keyStates[GLFW_KEY_S])
        {
            camera->move(CameraDirection::BACKWARD, deltaTime);
        }

        if (keyStates[GLFW_KEY_A])
        {
            camera->move(CameraDirection::LEFT, deltaTime);
        }

        if (keyStates[GLFW_KEY_D])
        {
            camera->move(CameraDirection::RIGHT, deltaTime);
        }
    }

    void drawScene()
    {
        // render background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // calculate new view and projection
        view = camera->calculateView();
        projection = glm::perspective(glm::radians(camera->getFov()), (GLfloat)curWidth / (GLfloat)curHeight, 0.1f, 100.0f);

        // update object shader
        lightingShader->use();
        lightingShader->setFloat("view", view);
        lightingShader->setFloat("projection", projection);

        // move emission texture based on time for a cool effect 😎
        lightingShader->setFloat("material.emissionVerticalOffset", -glfwGetTime() / 5.0);

        // calculate the direction of the directional light in view space
        directionalLight.direction =
            glm::normalize(glm::vec3(view * glm::vec4(directionalLight.worldDirection, 0.0)));
        lightingShader->setFloat("directionalLight.direction", directionalLight.direction);

        // calculate the view positions of the point lights
        for (std::size_t i = 0; i < pointLightPositions.size(); i++)
        {
            std::ostringstream pointLightIdentifier;
            pointLightIdentifier << "pointLights[" << i << "].position";
            glm::vec3 viewPosition = glm::vec3(view * glm::vec4(pointLightPositions[i], 1.0));
            lightingShader->setFloat(pointLightIdentifier.str(), viewPosition);
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
        for (std::size_t i = 0; i < cubePositions.size(); i++)
        {
            model = glm::translate(identityMatrix, cubePositions[i]);
            model = glm::rotate(model, (float)glfwGetTime() * glm::radians(20.0f * i), glm::vec3(1.0f, 0.3f, 0.5f));
            lightingShader->setFloat("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        // update light shader
        lightSourceShader->use();
        lightSourceShader->setFloat("view", view);
        lightSourceShader->setFloat("projection", projection);

        // draw light sources
        glBindVertexArray(lightVao);
        for (glm::vec3 &pointLightPosition : pointLightPositions)
        {
            model = glm::translate(identityMatrix, pointLightPosition);
            model = glm::scale(model, glm::vec3(0.2f));
            lightSourceShader->setFloat("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);
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

            if (ImGui::ColorEdit4("Clear color", glm::value_ptr(clearColor)))
            {
                glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
            }

            if (ImGui::Button("Show demo window"))
            {
                imguiState.showDemoWindow = !imguiState.showDemoWindow;
            }

            if (ImGui::CollapsingHeader("Material"))
            {
                if (ImGui::SliderFloat("Shininess##Material",
                                       &material.shininess,
                                       0.0f,
                                       128.0f,
                                       NULL,
                                       ImGuiSliderFlags_Logarithmic))
                {
                    lightingShader->setFloat("material.shininess", material.shininess);
                }
            }

            if (ImGui::CollapsingHeader("Directional light"))
            {
                if (ImGui::Button("Turn off##Directional light"))
                {
                    glm::vec3 zero(0.0f);
                    directionalLight.ambient = zero;
                    directionalLight.diffuse = zero;
                    directionalLight.specular = zero;
                    lightingShader->setFloat("directionalLight.ambient", directionalLight.ambient);
                    lightingShader->setFloat("directionalLight.diffuse", directionalLight.diffuse);
                    lightingShader->setFloat("directionalLight.specular", directionalLight.specular);
                }

                if (ImGui::DragFloat3("Direction (world space)##Directional light",
                                      glm::value_ptr(directionalLight.worldDirection),
                                      0.01f, -1.0f, 1.0f))
                {
                    // no need to update the shader
                    // the view direction of the light will be calculated next frame automatically
                }

                ImGui::Text("Direction: x:%f y:%f z:%f",
                            directionalLight.direction.x,
                            directionalLight.direction.y,
                            directionalLight.direction.z);

                if (ImGui::ColorEdit3("Ambient##Directional light",
                                      glm::value_ptr(directionalLight.ambient)))
                {
                    lightingShader->setFloat("directionalLight.ambient", directionalLight.ambient);
                }

                if (ImGui::ColorEdit3("Diffuse##Directional light",
                                      glm::value_ptr(directionalLight.diffuse)))
                {
                    lightingShader->setFloat("directionalLight.diffuse", directionalLight.diffuse);
                }

                if (ImGui::ColorEdit3("Specular##Directional light",
                                      glm::value_ptr(directionalLight.specular)))
                {
                    lightingShader->setFloat(
                        "directionalLight.specular", directionalLight.specular);
                }
            }

            if (ImGui::CollapsingHeader("Point lights"))
            {
                if (ImGui::Button("Turn off##Point lights"))
                {
                    glm::vec3 zero(0.0f);
                    pointLight.ambient = zero;
                    pointLight.diffuse = zero;
                    pointLight.specular = zero;
                    updatePointLightAttribute("ambient", pointLight.ambient);
                    updatePointLightAttribute("diffuse", pointLight.diffuse);
                    updatePointLightAttribute("specular", pointLight.specular);
                }

                if (ImGui::ColorEdit3("Cube color##Point lights",
                                      glm::value_ptr(pointLight.cubeColor)))
                {
                    lightSourceShader->setFloat("iColor", pointLight.cubeColor);
                }

                if (ImGui::ColorEdit3("Ambient##Point lights", glm::value_ptr(pointLight.ambient)))
                {
                    updatePointLightAttribute("ambient", pointLight.ambient);
                }

                if (ImGui::ColorEdit3("Diffuse##Point lights", glm::value_ptr(pointLight.diffuse)))
                {
                    updatePointLightAttribute("diffuse", pointLight.diffuse);
                }

                if (ImGui::ColorEdit3("Specular##Point lights",
                                      glm::value_ptr(pointLight.specular)))
                {
                    updatePointLightAttribute("specular", pointLight.specular);
                }

                if (ImGui::DragFloat(
                        "Constant##Point lights", &pointLight.constant, 0.01f, 0.0f, 200.0f))
                {
                    updatePointLightAttribute("constant", pointLight.constant);
                }

                if (ImGui::DragFloat("Linear##Point lights", &pointLight.linear,
                                     0.001f, 0.0f, 1.0f))
                {
                    updatePointLightAttribute("linear", pointLight.linear);
                }

                if (ImGui::DragFloat(
                        "Quadratic##Point lights", &pointLight.quadratic, 0.001f, 0.0f, 1.0f))
                {
                    updatePointLightAttribute("quadratic", pointLight.quadratic);
                }
            }

            if (ImGui::CollapsingHeader("Spotlight"))
            {
                if (ImGui::Button("Turn off##Spotlight"))
                {
                    glm::vec3 zero(0.0f);
                    spotLight.ambient = zero;
                    spotLight.diffuse = zero;
                    spotLight.specular = zero;
                    lightingShader->setFloat("spotLight.ambient", spotLight.ambient);
                    lightingShader->setFloat("spotLight.diffuse", spotLight.diffuse);
                    lightingShader->setFloat("spotLight.specular", spotLight.specular);
                }

                ImGui::Text("Position: x:%f y:%f z:%f",
                            spotLight.position.x,
                            spotLight.position.y,
                            spotLight.position.z);

                ImGui::Text("Direction: x:%f y:%f z:%f",
                            spotLight.direction.x,
                            spotLight.direction.y,
                            spotLight.direction.z);

                if (ImGui::ColorEdit3("Ambient##Spotlight", glm::value_ptr(spotLight.ambient)))
                {
                    lightingShader->setFloat("spotLight.ambient", spotLight.ambient);
                }

                if (ImGui::ColorEdit3("Diffuse##Spotlight", glm::value_ptr(spotLight.diffuse)))
                {
                    lightingShader->setFloat("spotLight.diffuse", spotLight.diffuse);
                }

                if (ImGui::ColorEdit3("Specular##Spotlight", glm::value_ptr(spotLight.specular)))
                {
                    lightingShader->setFloat("spotLight.specular", spotLight.specular);
                }

                if (ImGui::DragFloat(
                        "Constant##Spotlight", &spotLight.constant, 0.01f, 0.0f, 200.0f))
                {
                    lightingShader->setFloat("spotLight.constant", spotLight.constant);
                }

                if (ImGui::DragFloat("Linear##Spotlight", &spotLight.linear, 0.001f, 0.0f, 1.0f))
                {
                    lightingShader->setFloat("spotLight.linear", spotLight.linear);
                }

                if (ImGui::DragFloat(
                        "Quadratic##Spotlight", &spotLight.quadratic, 0.001f, 0.0f, 1.0f))
                {
                    lightingShader->setFloat("spotLight.quadratic", spotLight.quadratic);
                }

                if (ImGui::DragFloat("Cut Off##Spotlight", &spotLight.cutOff, 0.001f, 0.0f, 1.0f))
                {
                    lightingShader->setFloat("spotLight.cutOff", spotLight.cutOff);
                }

                if (ImGui::DragFloat(
                        "Outer Cut Off##Spotlight", &spotLight.outerCutOff, 0.001f, 0.0f, 1.0f))
                {
                    lightingShader->setFloat("spotLight.outerCutOff", spotLight.outerCutOff);
                }
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

    template <class T>
    void updatePointLightAttribute(std::string attribute, T &value)
    {
        // apply an attribute to all point lights
        for (std::size_t i = 0; i < pointLightPositions.size(); i++)
        {
            std::string identifier = "pointLights[" + std::to_string(i) + "]." + attribute;
            lightingShader->setFloat(identifier, value);
        }
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
            camera->rotate(xPos, yPos);
        }
    }

    void scrollCallback(GLFWwindow *window, double xOffset, double yOffset)
    {
        if (!ImGui::GetIO().WantCaptureMouse)
        {
            camera->zoom(yOffset);
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

} // namespace

int Renderer::init()
{
    if (int ret = initGlfw())
    {
        return ret;
    }
    if (int ret = initGlad())
    {
        return ret;
    }
    initGl();
    initImgui();
    initScene();
    return 0;
}

void Renderer::deinit()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Renderer::isRunning()
{
    return !glfwWindowShouldClose(window);
}

void Renderer::renderFrame()
{
    glfwPollEvents();

    // keep record of time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    moveCamera();
    drawScene();
    drawImgui();

    // swap buffers
    glfwSwapBuffers(window);
}
