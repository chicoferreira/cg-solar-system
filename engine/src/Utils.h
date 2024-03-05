#ifndef UTILS_H
#define UTILS_H
#include <filesystem>
#include <string>
#include <vector>
#include <optional>

namespace utils
{
    std::optional<std::filesystem::path>
    FindFile(const std::vector<std::string> &file_paths, const std::string &file_name);
} // namespace utils

#endif // UTILS_H
