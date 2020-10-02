#include "Renderer.h"

int main()
{
    int initReturnCode = Renderer::init();
    if (initReturnCode)
    {
        return initReturnCode;
    }

    while (Renderer::isRunning())
    {
        Renderer::renderFrame();
    }

    return 0;
}
