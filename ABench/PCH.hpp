#pragma once

#ifdef WIN32
// WinAPI & other Windows internals
#include <Windows.h>
#include <windowsx.h>
#elif defined(__linux__) || defined(__LINUX__)
#include <time.h> // for Timer
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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

// C library
#include <cassert>

// STL
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <memory>
#include <functional>
#include <unordered_map>
#include <map>
#include <array>
