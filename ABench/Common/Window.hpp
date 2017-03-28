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

protected:
    bool mKeys[255];

    // callbacks
    virtual void OnInit();
    virtual void OnOpen();
    virtual void OnClose();
    virtual void OnKeyDown(int key);
    virtual void OnKeyUp(int key);
    virtual void OnUpdate(float deltaTime);

public:
    Window();
    ~Window();

    bool Init();
    bool Open(int x, int y, int width, int height, const std::string& title);
    void Update(float deltaTime);
    void Close();

    __forceinline HINSTANCE GetInstance() const
    {
        return mInstance;
    }

    __forceinline HWND GetHandle() const
    {
        return mHWND;
    }

    __forceinline uint32_t GetWidth() const
    {
        return mWidth;
    }

    __forceinline uint32_t GetHeight() const
    {
        return mHeight;
    }

    __forceinline bool IsOpen() const
    {
        return mOpened;
    }
};

} // namespace Common
} // namespace ABench
