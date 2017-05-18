#pragma once

#include "Common.hpp"
#include "KeyCodes.hpp"


namespace ABench {
namespace Common {

class Window
{
#ifdef WIN32
    HINSTANCE mInstance;
    HWND mHWND;
    std::wstring mClassName;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#elif defined(__linux__) | defined(__LINUX__)
    xcb_connection_t* mConnection;
    xcb_window_t mWindow;
    xcb_screen_t* mScreen;
    xcb_intern_atom_reply_t* mDeleteReply;
    int mConnScreen;

    void ProcessMessages();
#else
#error "Target platform not supported"
#endif

    int mWidth;
    int mHeight;
    int mMouseDownX;
    int mMouseDownY;
    bool mOpened;
    bool mKeys[255];
    bool mMouseButtons[3];

protected:
    ABENCH_INLINE bool IsKeyPressed(KeyCode key) const
    {
        return mKeys[static_cast<std::underlying_type<KeyCode>::type>(key)];
    }

    ABENCH_INLINE bool IsMouseKeyPressed(int key) const
    {
        return mMouseButtons[key];
    }

    // callbacks
    virtual void OnInit();
    virtual void OnOpen();
    virtual void OnClose();
    virtual void OnKeyDown(KeyCode key);
    virtual void OnKeyUp(KeyCode key);
    virtual void OnUpdate(float deltaTime);
    virtual void OnMouseDown(int key);
    virtual void OnMouseMove(int x, int y, int deltaX, int deltaY);
    virtual void OnMouseUp(int key);

public:
    Window();
    ~Window();

    bool Init();
    bool Open(int x, int y, int width, int height, const std::string& title);
    bool SetTitle(const std::wstring& title);
    bool SetTitle(const std::string& title);
    void Update(float deltaTime);
    void MouseButtonDown(int button, int x, int y);
    void MouseButtonUp(int button);
    void MouseMove(int x, int y);
    void Close();

#ifdef WIN32
    ABENCH_INLINE HINSTANCE GetInstance() const
    {
        return mInstance;
    }

    ABENCH_INLINE HWND GetHandle() const
    {
        return mHWND;
    }
#elif defined(__linux__) | defined(__LINUX__)
    ABENCH_INLINE xcb_connection_t* GetConnection() const
    {
        return mConnection;
    }

    ABENCH_INLINE xcb_window_t GetWindow() const
    {
        return mWindow;
    }
#else
#error "Target platform not supported"
#endif

    ABENCH_INLINE uint32_t GetWidth() const
    {
        return mWidth;
    }

    ABENCH_INLINE uint32_t GetHeight() const
    {
        return mHeight;
    }

    ABENCH_INLINE bool IsOpen() const
    {
        return mOpened;
    }
};

} // namespace Common
} // namespace ABench
