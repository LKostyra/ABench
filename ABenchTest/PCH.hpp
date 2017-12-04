#pragma once

// GTest
#include <gtest/gtest.h>

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

// FBX SDK
#include <fbxsdk.h>

// STL
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <functional>
#include <random>
#include <algorithm>
#include <iterator>
