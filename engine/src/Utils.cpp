#include "Utils.h"

std::optional<std::filesystem::path>
utils::FindFile(const std::vector<std::string> &file_paths, const std::string &file_name)
{
    for (const auto &file_path : file_paths)
    {
        std::filesystem::path path(file_path);
        path /= file_name;
        if (std::filesystem::exists(path))
            return path;
    }

    return std::nullopt;
}
