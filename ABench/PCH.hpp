#pragma once

#ifdef WIN32
// WinAPI & other Windows internals
#include <Windows.h>
#include <windowsx.h>
#elif defined(__linux__) || defined(__LINUX__)
#include <time.h> // for Timer
#include <xcb/xcb.h>
#else
#error "Target platform not defined"
#endif

// Vulkan SDK
#include <vulkan.h>

// FBX SDK
#include <fbxsdk.h>

// FreeImage
#include <FreeImage.h>

// STL
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <memory>
#include <functional>
#include <type_traits>
