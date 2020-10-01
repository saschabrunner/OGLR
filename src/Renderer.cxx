#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "lib/stb_image.h"

// imgui
#include "lib/imgui/imgui.h"
#include "lib/imgui/imgui_impl_glfw.h"
#include "lib/imgui/imgui_impl_opengl3.h"

#include "DirectoryHelper.h"

// initialize const identity matrix
const glm::mat4 Renderer::identityMatrix(1.0f);

Renderer::Renderer()
    : curWidth(DEFAULT_WIDTH),
      curHeight(DEFAULT_HEIGHT),
      deltaTime(0.0f),
      lastFrame(0.0f),
      // camera slightly off to the side and looking down from above
      camera(
          glm::vec3(1.0f, 1.0f, 6.0f),
          glm::vec3(0.0f, 1.0f, 0.0f),
          -10.0f,
          -100.0f)
{
    initGlfw();
    initGlad();
    initGl();
    initImgui();
    initScene();
}

void Renderer::initGlfw()
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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // create lambda adapters for C style callbacks
    auto mouseCallbackAdapter = [](GLFWwindow *window, double xPos, double yPos) {
        static_cast<Renderer *>(glfwGetWindowUserPointer(window))
            ->mouseCallback(window, xPos, yPos);
    };
    auto scrollCallbackAdapter = [](GLFWwindow *window, double xOffset, double yOffset) {
        static_cast<Renderer *>(glfwGetWindowUserPointer(window))
            ->scrollCallback(window, xOffset, yOffset);
    };
    auto keyboardCallbackAdapter =
        [](GLFWwindow *window, int key, int scancode, int action, int mods) {
            static_cast<Renderer *>(glfwGetWindowUserPointer(window))
                ->keyboardCallback(window, key, scancode, action, mods);
        };
    auto framebufferSizeCallbackAdapter = [](GLFWwindow *window, int width, int height) {
        static_cast<Renderer *>(glfwGetWindowUserPointer(window))
            ->framebufferSizeCallback(window, width, height);
    };

    glfwSetCursorPosCallback(window, mouseCallbackAdapter);
    glfwSetScrollCallback(window, scrollCallbackAdapter);
    glfwSetKeyCallback(window, keyboardCallbackAdapter);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallbackAdapter);

    // enable vsync
    glfwSwapInterval(1);

    // set this as user pointer, so that we can use this later in the callbacks
    glfwSetWindowUserPointer(window, this);
}

void Renderer::initGlad()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }
}

void Renderer::initGl()
{
    glViewport(0, 0, curWidth, curHeight);

    // enable depth testing through z-buffer
    glEnable(GL_DEPTH_TEST);

    // set clear color (background color)
    // state setting, call once
    glClearColor(.01f, .01f, .01f, 1.0f);
}

void Renderer::initImgui()
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

void Renderer::initScene()
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
        DirectoryHelper::getInstance().locateData("textures/container2.png"),
        GL_TEXTURE0,
        GL_REPEAT);
    specularMap = createTexture(
        DirectoryHelper::getInstance().locateData("textures/container2_specular.png"),
        GL_TEXTURE1,
        GL_REPEAT);
    emissionMap = createTexture(
        DirectoryHelper::getInstance().locateData("textures/matrix.jpg"),
        GL_TEXTURE2,
        GL_REPEAT);

    // configure shader programs
    lightingShader = std::unique_ptr<Shader>(new Shader(
        DirectoryHelper::getInstance().locateData("shaders/06_normalTexCoord.vert"),
        DirectoryHelper::getInstance().locateData("shaders/06_multipleLights.frag")));
    lightingShader->setInt("material.diffuse", 0);
    lightingShader->setInt("material.specular", 1);
    lightingShader->setInt("material.emission", 2);
    lightingShader->setFloat("material.shininess", 32.0f);

    // directional light
    lightingShader->setFloat("directionalLight.direction", -0.2f, -1.0f, -0.3f);
    lightingShader->setFloat("directionalLight.ambient", 0.02f, 0.02f, 0.02f);
    lightingShader->setFloat("directionalLight.diffuse", 0.08f, 0.08f, 0.08f);
    lightingShader->setFloat("directionalLight.specular", 0.3f, 0.3f, 0.3f);

    // point lights
    for (int i = 0; i < pointLightPositions.size(); i++)
    {
        std::ostringstream pointLightIdentifier;
        pointLightIdentifier << "pointLights[" << i << "]";
        std::string pointLight = pointLightIdentifier.str();
        lightingShader->setFloat(pointLight + ".ambient", 0.03f, 0.008f, 0.0f);
        lightingShader->setFloat(pointLight + ".diffuse", 0.2f, 0.05f, 0.0f);
        lightingShader->setFloat(pointLight + ".specular", 1.0f, 1.0f, 1.0f);
        lightingShader->setFloat(pointLight + ".constant", 1.0f);
        lightingShader->setFloat(pointLight + ".linear", 0.14f);
        lightingShader->setFloat(pointLight + ".quadratic", 0.07f);
    }

    // spotlight
    // we are simulating a flashlight that's shining from the player's viewpoint
    glm::vec3 spotLightViewPosition(0.0f, 0.0f, 0.0f);
    glm::vec3 spotLightViewDirection(0.0f, 0.0f, -1.0f);
    lightingShader->setFloat("spotLight.position", spotLightViewPosition);
    lightingShader->setFloat("spotLight.direction", spotLightViewDirection);
    lightingShader->setFloat("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    lightingShader->setFloat("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    lightingShader->setFloat("spotLight.specular", 1.0f, 1.0f, 1.0f);
    lightingShader->setFloat("spotLight.constant", 1.0f);
    lightingShader->setFloat("spotLight.linear", 0.09f);
    lightingShader->setFloat("spotLight.quadratic", 0.032f);
    lightingShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f))); // going to be used for dot product, so use cos
    lightingShader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

    // shader for the light source cubes
    lightSourceShader = std::unique_ptr<Shader>(new Shader(
        DirectoryHelper::getInstance().locateData("shaders/04_normalCorrected.vert"),
        DirectoryHelper::getInstance().locateData("shaders/04_color.frag")));
    lightSourceShader->setFloat("iColor", 0.2f, 0.05f, 0.0f);

    // set up light VAO
    glGenVertexArrays(1, &lightVao);
    glBindVertexArray(lightVao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Renderer::~Renderer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

GLuint Renderer::createTexture(const std::string &path, GLenum glTextureUnit, GLint wrappingMode)
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

void Renderer::moveCamera()
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

void Renderer::drawScene()
{
    // render background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // calculate new view and projection
    view = camera.calculateView();
    projection = glm::perspective(glm::radians(camera.getFov()), (GLfloat)curWidth / (GLfloat)curHeight, 0.1f, 100.0f);

    // update object shader
    lightingShader->use();
    lightingShader->setFloat("view", view);
    lightingShader->setFloat("projection", projection);
    lightingShader->setFloat("material.emissionVerticalOffset", -glfwGetTime() / 5.0);

    for (int i = 0; i < pointLightPositions.size(); i++)
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
    for (int i = 0; i < cubePositions.size(); i++)
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
    for (glm::vec3 pointLightPosition : pointLightPositions)
    {
        model = glm::translate(identityMatrix, pointLightPosition);
        model = glm::scale(model, glm::vec3(0.2f));
        lightSourceShader->setFloat("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
}

void Renderer::drawImgui()
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

void Renderer::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    curWidth = width;
    curHeight = height;
    glViewport(0, 0, width, height);
}

void Renderer::mouseCallback(GLFWwindow *window, double xPos, double yPos)
{
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        camera.rotate(xPos, yPos);
    }
}

void Renderer::scrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        camera.zoom(yOffset);
    }
}

void Renderer::keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
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

void Renderer::errorCallback(int error, const char *description)
{
    std::cerr << "Glfw Error " << error << ": " << description << std::endl;
}

bool Renderer::isRunning() const
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