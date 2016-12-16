#include "../PCH.hpp"
#include "Window.hpp"
#include "Common.hpp"


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

    default: break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool Window::Init()
{
    if (mInstance)
        return true;

    const void* addr = static_cast<const void*>(this);
    std::wstringstream ss;
    ss << addr;
    mClassName = L"ABench_Class_" + ss.str();

    mInstance = GetModuleHandle(0);
    if (!mInstance)
    {
        // LOG ERROR
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
        // LOG ERROR
        return false;
    }

#ifndef _DEBUG
    // hide console on non-debug modes
    HWND mConsole = GetConsoleWindow();
    ShowWindow(mConsole, SW_HIDE);
#endif

    return true;
}

bool Window::Open(int x, int y, int width, int height, const std::string& title)
{
    if (mOpened)
        return false; // we cannot open a new window - we are already opened

    // Convert title to UTF16
    std::wstring wideTitle;
    if (!UTF8ToUTF16(title, wideTitle))
    {
        return false;
    }

    // TODO FULLSCREEN

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
        // LOG ERROR
        return false;
    }

    SetWindowLongPtr(mHWND, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    SetWindowText(mHWND, wideTitle.c_str());

    ShowWindow(mHWND, SW_SHOW);
    UpdateWindow(mHWND);
    SetFocus(mHWND);

    mOpened = true;
    mWidth = width;
    mHeight = height;

    return true;
}

void Window::ProcessMessages()
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
}

void Window::Close()
{
    if (!mOpened)
        return;

    DestroyWindow(mHWND);
    mOpened = false;
}

HINSTANCE Window::GetInstance() const
{
    return mInstance;
}

HWND Window::GetHandle() const
{
    return mHWND;
}


// checks

bool Window::IsOpen() const
{
    return mOpened;
}


// callbacks

void Window::OnClose()
{
}

} // namespace Common
} // namespace ABench
