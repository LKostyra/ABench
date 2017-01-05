#pragma once

#include <iostream>

#define LOG(level, msg) do { \
    std::cerr << "[" << level << "] " << msg << std::endl; \
} while(0)

#define LOGD(msg) LOG("DEBUG", msg);
#define LOGE(msg) LOG("ERROR", msg);
#define LOGI(msg) LOG(" INF ", msg);
#define LOGW(msg) LOG(" WRN ", msg);
