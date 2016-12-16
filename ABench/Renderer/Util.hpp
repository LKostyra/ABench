#pragma once

#include "Common/Logger.hpp"

#define CHECK_VKRESULT(r, msg) \
    if (r != VK_SUCCESS) \
    { \
        LOGE(msg << ": " << r); \
        return false; \
    }
