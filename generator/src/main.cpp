#include <filesystem>
#include <iostream>
#include <string>

#include "Generator.h"
#include "SolarSystem.h"

enum CommandType
{
    PLANE,
    SPHERE,
    CONE,
    BOX,
    CYLINDER,
    SOLAR_SYSTEM,
};

struct Command
{
    CommandType type;
    std::string name;
    std::string args;
    size_t arg_count;
};

const auto commands = {
    Command{PLANE, "plane", "<length> <divisions>", 2},
    Command{SPHERE, "sphere", "<radius> <slices> <stacks>", 3},
    Command{CONE, "cone", "<radius> <height> <slices> <stacks>", 4},
    Command{BOX, "box", "<length> <divisions>", 2},
    Command{CYLINDER, "cylinder", "<radius> <height> <slices>", 3},
    Command{SOLAR_SYSTEM, "solar-system", "", 0},
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
    for (const auto &command : commands)
    {
        std::cout << "\tgenerator " << command.name << ' ' << command.args << " <output file>" << std::endl;
    }
}

void printUsage(const Command &cmd)
{
    std::cout << "Usage: generator " << cmd.name << ' ' << cmd.args << " <output file>" << std::endl;
}

void runGenerator(const Command &cmd, char *args[])
{
    switch (cmd.type)
    {
        case PLANE:
            {
                const auto length = std::stof(args[0]);
                const auto divisions = std::stoi(args[1]);
                generator::SaveModel(generator::GeneratePlane(length, divisions), args[2]);
            }
        case SPHERE:
            {
                const auto radius = std::stof(args[0]);
                const auto slices = std::stoi(args[1]);
                const auto stacks = std::stoi(args[2]);
                generator::SaveModel(generator::GenerateSphere(radius, slices, stacks), args[3]);
            }
        case CONE:
            {
                const auto radius = std::stof(args[0]);
                const auto height = std::stof(args[1]);
                const auto slices = std::stoi(args[2]);
                const auto stacks = std::stoi(args[3]);
                generator::SaveModel(generator::GenerateCone(radius, height, slices, stacks), args[4]);
            }
        case BOX:
            {
                const auto length = std::stof(args[0]);
                const auto divisions = std::stoi(args[1]);
                generator::SaveModel(generator::GenerateBox(length, divisions), args[2]);
            }
        case CYLINDER:
            {
                const auto radius = std::stof(args[0]);
                const auto height = std::stof(args[1]);
                const auto slices = std::stoi(args[2]);
                generator::SaveModel(generator::GenerateCylinder(radius, height, slices), args[3]);
            }
        case SOLAR_SYSTEM:
            {
                generator::solarsystem::GenerateSolarSystem();
            }
    }
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

    try
    {
        runGenerator(*cmd, argv + 2);
    }
    catch (...)
    {
        std::cout << "Invalid arguments for command " << cmd->name << std::endl;
        printUsage(*cmd);
        return 1;
    }
}
