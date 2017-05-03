#include "PCH.hpp"
#include "../Window.hpp"
#include "../Logger.hpp"


namespace ABench {
namespace Common {

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
const DWORD WINDOWED_EX_STYLE = WS_EX_WINDOWEDGE;
const DWORD WINDOWED_STYLE = WS_OVERLAPPEDWINDOW;

Window::Window()
    : mInstance(0)
    , mHWND(0)
    , mWidth(0)
    , mHeight(0)
    , mOpened(false)
{
}

Window::~Window()
{
    OnClose();
    Close();
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Window* wnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (wnd == nullptr)
        return DefWindowProc(hWnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_CLOSE:
        wnd->OnClose();
        wnd->Close();
        break;

    case WM_KEYDOWN:
        wnd->mKeys[wParam] = true;
        wnd->OnKeyDown(static_cast<int>(wParam));
        break;

    case WM_KEYUP:
        wnd->mKeys[wParam] = false;
        wnd->OnKeyUp(static_cast<int>(wParam));
        break;

    case WM_LBUTTONDOWN:
        wnd->mMouseButtons[0] = true;
        wnd->OnMouseDown(0);
        break;

    case WM_LBUTTONUP:
        wnd->mMouseButtons[0] = false;
        wnd->OnMouseUp(0);
        break;

    case WM_RBUTTONDOWN:
        wnd->mMouseButtons[1] = true;
        wnd->OnMouseDown(1);
        break;

    case WM_RBUTTONUP:
        wnd->mMouseButtons[1] = false;
        wnd->OnMouseUp(1);
        break;

    case WM_MBUTTONDOWN:
        wnd->mMouseButtons[2] = true;
        wnd->OnMouseDown(2);
        break;

    case WM_MBUTTONUP:
        wnd->mMouseButtons[2] = false;
        wnd->OnMouseUp(2);
        break;

    default: break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool Window::Init()
{
    if (mInstance)
        return true;

    const void* addr = static_cast<const void*>(this);
    std::wstringstream sstream;
    sstream << addr;
    mClassName = L"ABench_Class_" + sstream.str();

    mInstance = GetModuleHandle(0);
    if (!mInstance)
    {
        DWORD error = GetLastError();
        LOGE("Failed to get application's Instance: " << static_cast<int>(error));
        return false;
    }

    WNDCLASSEX wc;
    ZERO_MEMORY(wc);
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = mInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = mClassName.c_str();
    if (!RegisterClassEx(&wc))
    {
        DWORD error = GetLastError();
        LOGE("Failed to register application's class: " << static_cast<int>(error));
        return false;
    }

#ifndef _DEBUG
    // hide console on non-debug modes
    HWND mConsole = GetConsoleWindow();
    ShowWindow(mConsole, SW_HIDE);
#endif

    OnInit();

    return true;
}

bool Window::Open(int x, int y, int width, int height, const std::string& title)
{
    if (mOpened)
        return false; // we cannot open a new window - we are already opened

    // TODO FULLSCREEN
    std::wstring wideTitle;
    if (!UTF8ToUTF16(title, wideTitle))
        return false;

    RECT wr;
    wr.left = (long)x;
    wr.right = x + width;
    wr.top = y;
    wr.bottom = y + height;
    AdjustWindowRectEx(&wr, WINDOWED_STYLE, false, WINDOWED_EX_STYLE);

    mHWND = CreateWindowEx(WINDOWED_EX_STYLE, mClassName.c_str(), wideTitle.c_str(), WINDOWED_STYLE,
                           wr.left, wr.top, (wr.right - wr.left), (wr.bottom - wr.top), nullptr, nullptr,
                           mInstance, nullptr);
    if (!mHWND)
    {
        DWORD error = GetLastError();
        LOGE("Failed to create Window: " << static_cast<int>(error));
        return false;
    }

    SetWindowLongPtr(mHWND, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    ShowWindow(mHWND, SW_SHOW);
    UpdateWindow(mHWND);
    SetFocus(mHWND);

    mOpened = true;
    mWidth = width;
    mHeight = height;

    OnOpen();

    return true;
}

bool Window::SetTitle(const std::wstring& title)
{
    if (!SetWindowText(mHWND, title.c_str()))
    {
        DWORD error = GetLastError();
        LOGE("Failed to set title string: " << static_cast<int>(error));
        return false;
    }

    return true;
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
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            Close();
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    OnUpdate(deltaTime);
}

void Window::Close()
{
    if (!mOpened)
        return;

    DestroyWindow(mHWND);
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
