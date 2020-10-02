#ifndef RENDERER_H
#define RENDERER_H

namespace Renderer
{
    constexpr int INIT_FAIL_GLFW_INIT = -1;
    constexpr int INIT_FAIL_GLFW_CREATE_WINDOW = -2;
    constexpr int INIT_FAIL_GLAD = -2;

    /**
     * Initialize the global state of the renderer.
     * @return 0 on success, non zero on failure.
     */
    int init();
    void deinit();

    bool isRunning();
    void renderFrame();
} // namespace Renderer

#endif