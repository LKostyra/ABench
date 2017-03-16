#include "PCH.hpp"
#include <iostream>
#include "Common/Window.hpp"
#include "Renderer/Renderer.hpp"

ABench::Common::Window gWindow;

uint32_t windowWidth = 800;
uint32_t windowHeight = 600;

int main()
{
    gWindow.Init();
    if (!gWindow.Open(300, 300, windowWidth, windowHeight, "ABench"))
        return -1;

    bool debug = false;

#ifdef _DEBUG
    debug = true;
#endif

    ABench::Renderer::Renderer rend;
    if (!rend.Init(gWindow, debug, false))
        return -1;

    while(gWindow.IsOpen())
    {
        gWindow.ProcessMessages();
        rend.Draw();
    }

    system("PAUSE");

    return 0;
}
