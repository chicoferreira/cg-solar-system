#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "Generator.h"

constexpr std::string_view default_folder = "assets/models/";

struct Command
{
    std::string name;
    std::string args;
    size_t arg_count;
};

const auto commands = {
    Command{"plane", "<length> <divisions>", 2},
    Command{"sphere", "<radius> <slices> <stacks>", 3},
    Command{"cone", "<radius> <height> <slices> <stacks>", 4},
    Command{"box", "<length> <divisions>", 2},
};

const Command *getCommand(const char *name)
{
    for (const auto &command : commands)
    {
        if (command.name == name)
        {
            return &command;
        }
    }
    return nullptr;
}

void printHelp()
{
    std::cout << "Usage: generator <command> <args> <output file>" << std::endl;
    std::cout << "Commands:" << std::endl;
    for (const auto &[name, args, _] : commands)
    {
        std::cout << "\tgenerator " << name << ' ' << args << " <output file>" << std::endl;
    }
}

void printUsage(const Command &cmd)
{
    std::cout << "Usage: generator " << cmd.name << ' ' << cmd.args << " <output file>" << std::endl;
}

std::vector<Vec3f> runGenerator(const Command &cmd, char *args[])
{
    if (cmd.name == "plane")
    {
        const auto length = std::stof(args[0]);
        const auto divisions = std::stoi(args[1]);
        return generator::GeneratePlane(length, divisions);
    }
    if (cmd.name == "sphere")
    {
        const auto radius = std::stof(args[0]);
        const auto slices = std::stoi(args[1]);
        const auto stacks = std::stoi(args[2]);
        return generator::GenerateSphere(radius, slices, stacks);
    }
    if (cmd.name == "cone")
    {
        const auto radius = std::stof(args[0]);
        const auto height = std::stof(args[1]);
        const auto slices = std::stoi(args[2]);
        const auto stacks = std::stoi(args[3]);
        return generator::GenerateCone(radius, height, slices, stacks);
    }
    if (cmd.name == "box")
    {
        const auto length = std::stof(args[0]);
        const auto divisions = std::stoi(args[1]);
        return generator::GenerateBox(length, divisions);
    }
    throw std::runtime_error("Unknown command");
}

int main(const int argc, char *argv[])
{
    if (argc < 2)
    {
        printHelp();
        return 1;
    }

    const char *command = argv[1];
    const auto *cmd = getCommand(command);

    if (cmd == nullptr)
    {
        std::cout << "Unknown command: " << command << std::endl;
        printHelp();
        return 1;
    }

    if (argc - 3 != cmd->arg_count)
    {
        std::cout << "Invalid number of arguments for command " << cmd->name << std::endl;
        printUsage(*cmd);
        return 1;
    }

    std::vector<Vec3f> vertices;

    try
    {
        vertices = runGenerator(*cmd, argv + 2);
    }
    catch (...)
    {
        std::cout << "Invalid arguments for command " << cmd->name << std::endl;
        printUsage(*cmd);
        return 1;
    }

    const auto save_file = argv[argc - 1];

    std::filesystem::path path = default_folder;
    path.append(save_file);

    if (!std::filesystem::exists(path.parent_path()))
    {
        std::filesystem::create_directories(path.parent_path());
    }

    // write to file
    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open())
    {
        std::cout << "Failed to open file " << save_file << std::endl;
        return 1;
    }

    file << vertices.size() << std::endl;

    for (const auto &vec : vertices)
    {
        file << vec.x << " " << vec.y << " " << vec.z << std::endl;
    }

    return 0;
}
