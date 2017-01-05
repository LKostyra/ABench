#pragma once

#include <string>

namespace ABench {
namespace Renderer {

const char* TranslateVkFormatToString(VkFormat format);
const char* TranslateVkPhysicalDeviceTypeToString(VkPhysicalDeviceType type);
const char* TranslateVkQueueFlagsToString(VkQueueFlags flags);
const char* TranslateVkResultToString(VkResult result);

} // namespace Renderer
} // namespace ABench
