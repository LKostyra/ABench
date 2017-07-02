#pragma once

namespace ABench {
namespace Common {
namespace FS {

/**
 * Extract full executable path using platform-specific methods. Returned path has forward slashes.
 */
std::string GetExecutablePath();

/**
 * Acquires parent directory of provided path. Example:
 *   /usr/bin/bash -> /usr/bin
 *   /usr/bin -> /usr
 */
std::string GetParentDir(const std::string& path);

/**
 * Acquires the name of file from provided path. Example:
 *   /usr/bin/bash -> bash
 *   bash -> bash
 */
std::string GetFileName(const std::string& path);

/**
 * Joins two paths together, adding slashes where needed. Examples:
 *   JoinPaths("/usr/bin", "bash") -> /usr/bin/bash
 *   JoinPaths("/usr/bin/", "bash") -> /usr/bin/bash
 *   JoinPaths("/usr/bin/", "/bash") -> /usr/bin/bash
 */
std::string JoinPaths(const std::string& a, const std::string& b);

/**
 * Sets current working directory
 */
bool SetCWD(const std::string& path);

/**
 * Unifies slashes in file path - will convert all '\\' chars into '/'
 */
std::string UnifySlashes(const std::string& path);

} // namespace FS
} // namespace Common
} // namespace ABench
