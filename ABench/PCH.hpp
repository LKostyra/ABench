#pragma once

#ifdef WIN32
// WinAPI & other Windows internals
#include <Windows.h>
#endif

// Vulkan SDK
#include <vulkan/vulkan.h>

// STL
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

// Common defines
#ifdef WIN32
#define ABENCH_INLINE __forceinline
#elif defined(__linux__) | defined(__LINUX__)
#define ABENCH_INLINE __attribute__((always_inline))
#else
#error "Target platform not supported"
#endif
