#include "PCH.hpp"
#include "Common/Window.hpp"

using namespace ABench::Common;

const uint32_t TEST_WINDOW_WIDTH = 100;
const uint32_t TEST_WINDOW_HEIGHT = 100;
const std::string TEST_WINDOW_NAME = "ABenchTest window";
const float TEST_WINDOW_UPDATE_DELTA_TIME = 1.0f;

class CallbackTestWindow: public Window
{
    bool mOnInitCalled;
    bool mOnOpenCalled;
    bool mOnCloseCalled;
    bool mOnUpdateCalled;

protected:
    void OnInit() override
    {
        mOnInitCalled = true;
    }

    void OnOpen() override
    {
        mOnOpenCalled = true;
    }

    void OnClose() override
    {
        mOnCloseCalled = true;
    }

    void OnUpdate(float deltaTime) override
    {
        EXPECT_EQ(deltaTime, TEST_WINDOW_UPDATE_DELTA_TIME);
        mOnUpdateCalled = true;
    }

    /* TODO testing event callbacks requires a platform-specific test class
            which will emit these events to our Window class
    void OnKeyDown(KeyCode key) override
    {
    }

    void OnKeyUp(KeyCode key) override
    {
    }

    void OnMouseDown(int key) override
    {
    }

    void OnMouseMove(int x, int y, int deltaX, int deltaY) override
    {
    }

    void OnMouseUp(int key) override
    {
    }
    */

public:
    bool OnInitCalled()
    {
        return mOnInitCalled;
    }

    bool OnOpenCalled()
    {
        return mOnOpenCalled;
    }

    bool OnCloseCalled()
    {
        return mOnCloseCalled;
    }

    bool OnUpdateCalled()
    {
        return mOnUpdateCalled;
    }
};

TEST(Window, Constructor)
{
    Window w;
    EXPECT_FALSE(w.IsOpen());
}

TEST(Window, Init)
{
    Window w;
    EXPECT_TRUE(w.Init());
}

TEST(Window, OpenWithoutInit)
{
    Window w;
    w.SetInvisible(true);
    EXPECT_FALSE(w.Open(0, 0, TEST_WINDOW_WIDTH, TEST_WINDOW_HEIGHT, TEST_WINDOW_NAME));
}

TEST(Window, Open)
{
    Window w;
    w.SetInvisible(true);
    EXPECT_TRUE(w.Init());
    EXPECT_FALSE(w.IsOpen());
    EXPECT_TRUE(w.Open(0, 0, TEST_WINDOW_WIDTH, TEST_WINDOW_HEIGHT, TEST_WINDOW_NAME));
    EXPECT_TRUE(w.IsOpen());
}

TEST(Window, OnInitCallback)
{
    CallbackTestWindow w;
    EXPECT_FALSE(w.OnInitCalled());
    w.Init();
    EXPECT_TRUE(w.OnInitCalled());
}

TEST(Window, OnOpenCallback)
{
    // just-created window shouldn't call any callbacks
    CallbackTestWindow w;
    EXPECT_FALSE(w.OnInitCalled());
    EXPECT_FALSE(w.OnOpenCalled());

    // Init should call OnInit only
    w.Init();
    EXPECT_TRUE(w.OnInitCalled());
    EXPECT_FALSE(w.OnOpenCalled());

    // Open should call OnOpen
    EXPECT_TRUE(w.Open(0, 0, TEST_WINDOW_WIDTH, TEST_WINDOW_HEIGHT, TEST_WINDOW_NAME));
    EXPECT_TRUE(w.OnOpenCalled());
}

TEST(Window, OnUpdateCallback)
{
    CallbackTestWindow w;
    EXPECT_FALSE(w.OnUpdateCalled());

    EXPECT_TRUE(w.Init());
    EXPECT_FALSE(w.OnUpdateCalled());

    EXPECT_TRUE(w.Open(0, 0, TEST_WINDOW_WIDTH, TEST_WINDOW_HEIGHT, TEST_WINDOW_NAME));
    EXPECT_FALSE(w.OnUpdateCalled());

    w.Update(TEST_WINDOW_UPDATE_DELTA_TIME);
    EXPECT_TRUE(w.OnUpdateCalled());
}

TEST(Window, OnCloseCallback)
{
    CallbackTestWindow w;
    EXPECT_FALSE(w.OnCloseCalled());

    EXPECT_TRUE(w.Init());
    EXPECT_FALSE(w.OnCloseCalled());

    // Calling Close() without Open() should not trigger OnClose()
    w.Close();
    EXPECT_FALSE(w.OnCloseCalled());

    EXPECT_TRUE(w.Open(0, 0, TEST_WINDOW_WIDTH, TEST_WINDOW_HEIGHT, TEST_WINDOW_NAME));
    EXPECT_FALSE(w.OnCloseCalled());

    w.Close();
    EXPECT_TRUE(w.OnCloseCalled());
}
