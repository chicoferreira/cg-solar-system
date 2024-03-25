#ifndef UTILS_H
#define UTILS_H
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace engine::utils
{
    std::optional<std::filesystem::path>
    FindFile(const std::vector<std::string> &file_paths, const std::string &file_name);
} // namespace engine::utils

#endif // UTILS_H
