#pragma once

namespace ABench {
namespace Common {

class Window
{
    HINSTANCE mInstance;
    HWND mHWND;
    int mWidth;
    int mHeight;
    bool mOpened;
    std::wstring mClassName;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
    Window();
    ~Window();

    bool Init();
    bool Open(int x, int y, int width, int height, const std::string& title);
    void ProcessMessages();
    void Close();

    HINSTANCE GetInstance() const;
    HWND GetHandle() const;

    bool IsOpen() const;

protected: // callbacks
    virtual void OnClose();
};

} // namespace Common
} // namespace ABench
