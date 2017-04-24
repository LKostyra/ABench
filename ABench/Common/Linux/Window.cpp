#include "../../PCH.hpp"
#include "../Window.hpp"
#include "../Common.hpp"
#include "../Logger.hpp"


namespace ABench {
namespace Common {

Window::Window()
    : mWidth(0)
    , mHeight(0)
    , mOpened(false)
{
}

Window::~Window()
{
    OnClose();
    Close();
}

bool Window::Init()
{
    return false;
}

bool Window::Open(int x, int y, int width, int height, const std::string& title)
{
    UNUSED(x); UNUSED(y); UNUSED(width); UNUSED(height); UNUSED(title);
    return false;
}

bool Window::SetTitle(const std::wstring& title)
{
    UNUSED(title);
    return false;
}

bool Window::SetTitle(const std::string& title)
{
    // Convert title to UTF16
    std::wstring wideTitle;
    if (!UTF8ToUTF16(title, wideTitle))
    {
        return false;
    }

    return SetTitle(wideTitle);
}

void Window::Update(float deltaTime)
{
    // TODO message loop
    OnUpdate(deltaTime);
}

void Window::Close()
{
    if (!mOpened)
        return;

    mOpened = false;
}


// callbacks

void Window::OnInit()
{
}

void Window::OnOpen()
{
}

void Window::OnClose()
{
}

void Window::OnKeyDown(int key)
{
    UNUSED(key);
}

void Window::OnKeyUp(int key)
{
    UNUSED(key);
}

void Window::OnUpdate(float deltaTime)
{
    UNUSED(deltaTime);
}

void Window::OnMouseDown(int key)
{
    UNUSED(key);
}

void Window::OnMouseMove(int deltaX, int deltaY)
{
    UNUSED(deltaX);
    UNUSED(deltaY);
}

void Window::OnMouseUp(int key)
{
    UNUSED(key);
}

} // namespace Common
} // namespace ABench
