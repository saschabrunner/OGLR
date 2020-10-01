#include "Renderer.h"

int main()
{
    Renderer renderer;

    while (renderer.isRunning())
    {
        renderer.renderFrame();
    }

    return 0;
}
