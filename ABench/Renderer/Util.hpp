#pragma once

#include "Common/Logger.hpp"
#include "Translations.hpp"

#define RETURN_FALSE_IF_FAILED(r, msg) \
    if (r != VK_SUCCESS) \
    { \
        LOGE(msg << ": " << r << " (" << TranslateVkResultToString(r) << ")"); \
        return false; \
    }

#define RETURN_NULL_HANDLE_IF_FAILED(r, msg) \
    if (r != VK_SUCCESS) \
    { \
        LOGE(msg << ": " << r << " (" << TranslateVkResultToString(r) << ")"); \
        return VK_NULL_HANDLE; \
    }
