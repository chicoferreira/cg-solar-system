#include <fstream>
#include <iostream>
#include <optional>

#include "Engine.h"

std::optional<Model> ReadModelFromFile(const std::string &file_path)
{
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        return std::nullopt;
    }

    Model model;
    model.ReadFromFile(file);
    return std::make_optional(model);
}

// Main code
int main(int, char **)
{
    const auto model_opt = ReadModelFromFile("assets/models/plane.3d");
    if (!model_opt)
    {
        std::cerr << "Failed to read model from file\n";
        return 1;
    }

    auto model = model_opt.value();

    std::cout << "Model positions: ";
    for (const auto &[x, y, z] : model.GetPositions())
    {
        std::cout << x << " " << y << " " << z << " ";
    }

    Engine engine(1280, 720);
    if (!engine.Init())
        return 1;

    engine.AddModel(model);

    engine.Run();

    engine.Shutdown();

    return 0;
}
