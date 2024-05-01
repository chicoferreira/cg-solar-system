#include "Utils.h"
namespace engine::utils
{
    std::optional<std::filesystem::path>
    FindFile(const std::vector<std::string> &file_paths, const std::string &file_name)
    {
        for (const auto &file_path : file_paths)
        {
            std::filesystem::path path(file_path);
            path /= file_name;
            if (exists(path))
                return path;
        }

        return std::nullopt;
    }

    OperatingSystem getOS()
    {
#ifdef _WIN32
        return OperatingSystem::WINDOWS;
#elif __APPLE__
        return OperatingSystem::MACOS;
#elif __linux__
        return OperatingSystem::LINUX;
#else
        return OperatingSystem::UNKNOWN;
#endif
    }

    const char *GetOSName(OperatingSystem os)
    {
        switch (os)
        {
            case OperatingSystem::WINDOWS:
                return "Windows";
            case OperatingSystem::LINUX:
                return "Linux";
            case OperatingSystem::MACOS:
                return "MacOS";
            default:
                return "Unknown";
        }
    }

} // namespace engine::utils
