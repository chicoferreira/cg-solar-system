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
    BEZIER,
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
    Command{BEZIER, "bezier", "<file> <tesselation>", 2},
    Command{
        SOLAR_SYSTEM,
        "solar-system",
        "<sun size scale factor> <planet distance scale factor> <scene scale factor>",
        3
    },
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

#define PARSE_FLOAT(varname, value)                                                                                    \
    float varname;                                                                                                     \
    try                                                                                                                \
    {                                                                                                                  \
        varname = std::stof(value);                                                                                    \
    }                                                                                                                  \
    catch (const std::logic_error &)                                                                                   \
    {                                                                                                                  \
        std::cout << "Couldn't parse float " << #varname << " from '" << value << "'. Needed in " << cmd.name          \
                  << " command." << std::endl;                                                                         \
        return;                                                                                                        \
    }

#define PARSE_INT(varname, value)                                                                                      \
    int varname;                                                                                                       \
    try                                                                                                                \
    {                                                                                                                  \
        varname = std::stoi(value);                                                                                    \
    }                                                                                                                  \
    catch (const std::logic_error &)                                                                                   \
    {                                                                                                                  \
        std::cout << "Couldn't parse integer " << #varname << " from '" << value << "'. Needed in " << cmd.name        \
                  << " command." << std::endl;                                                                         \
        return;                                                                                                        \
    }

void runGenerator(const Command &cmd, char *args[])
{
    switch (cmd.type)
    {
        case PLANE:
            {
                PARSE_FLOAT(length, args[0])
                PARSE_INT(divisions, args[1])
                generator::SaveModel(generator::GeneratePlane(length, divisions), args[2]);
                break;
            }
        case SPHERE:
            {
                PARSE_FLOAT(radius, args[0])
                PARSE_INT(slices, args[1])
                PARSE_INT(stacks, args[2])
                generator::SaveModel(generator::GenerateSphere(radius, slices, stacks), args[3]);
                break;
            }
        case CONE:
            {
                PARSE_FLOAT(radius, args[0])
                PARSE_FLOAT(height, args[1])
                PARSE_INT(slices, args[2])
                PARSE_INT(stacks, args[3])
                generator::SaveModel(generator::GenerateCone(radius, height, slices, stacks), args[4]);
                break;
            }
        case BOX:
            {
                PARSE_FLOAT(length, args[0])
                PARSE_INT(divisions, args[1])
                generator::SaveModel(generator::GenerateBox(length, divisions), args[2]);
                break;
            }
        case CYLINDER:
            {
                PARSE_FLOAT(radius, args[0])
                PARSE_FLOAT(height, args[1])
                PARSE_INT(slices, args[2])
                generator::SaveModel(generator::GenerateCylinder(radius, height, slices), args[3]);
                break;
            }
        case BEZIER:
            {
                PARSE_INT(tesselation, args[1])
                generator::SaveModel(generator::GenerateBezier(args[0], tesselation), args[2]);
                break;
            }
        case SOLAR_SYSTEM:
            {
                PARSE_FLOAT(sun_size_scaling, args[0])
                PARSE_FLOAT(planet_distance_scaling, args[1])
                PARSE_FLOAT(scene_scaling, args[2])
                generator::solarsystem::GenerateSolarSystem(sun_size_scaling, planet_distance_scaling, scene_scaling);
                break;
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

    runGenerator(*cmd, argv + 2);
}
