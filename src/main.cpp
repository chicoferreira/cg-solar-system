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
    model.LoadFrom3dFormat(file);
    return std::make_optional(model);
}

int main(int, char **)
{
    auto model_opt = ReadModelFromFile("assets/models/sphere.3d");
    if (!model_opt)
    {
        std::cerr << "Failed to read model from file\n";
        return 1;
    }

    Engine engine(1280, 720, {5, 5, 5});
    if (!engine.Init())
        return 1;

    engine.AddModel(model_opt.value());
    
    auto merda = ReadModelFromFile("assets/models/plane.3d").value();
    engine.AddModel(merda);

    engine.Run();

    engine.Shutdown();

    return 0;
}
