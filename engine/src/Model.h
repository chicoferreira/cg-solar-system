#ifndef MODEL_H
#define MODEL_H
#include <cstdint>
#include <istream>
#include <optional>
#include <vector>

#include "Color.h"
#include "Vec.h"

namespace engine::model
{
    class Texture
    {
        uint32_t image_width, image_height;
        std::vector<uint8_t> texture_data;
        uint32_t il_id;

    public:
        Texture(uint32_t image_width, uint32_t image_height, std::vector<uint8_t> &texture_data, uint32_t il_id) :
            image_width(image_width), image_height(image_height), texture_data(std::move(texture_data)), il_id(il_id)
        {
        }
        uint32_t GetWidth() const { return image_width; };
        uint32_t GetHeight() const { return image_height; };
        std::vector<uint8_t> &GetTextureData() { return texture_data; };
        ~Texture();
    };
    std::optional<Texture> LoadTextureFromFile(const std::string &file_path);

    enum class ModelLoadFormat
    {
        OBJ,
        _3D
    };

    std::optional<ModelLoadFormat> GetModelLoadFormat(const std::string &file_path);

    class Model
    {
        std::string m_name;
        std::vector<Vec3f> m_vertex;
        std::vector<Vec3f> m_normals;
        std::vector<Vec2f> m_tex_coords;
        std::vector<uint32_t> m_indexes;

    public:
        explicit Model(std::string name) : m_name(std::move(name)) {}

        explicit Model(std::string name, std::vector<Vec3f> vertex) :
            m_name(std::move(name)), m_vertex(std::move(vertex))
        {
        }

        const std::string &GetName() const { return m_name; }
        std::vector<Vec3f> &GetVertex() { return m_vertex; }
        std::vector<Vec3f> &GetNormals() { return m_normals; }
        std::vector<Vec2f> &GetTexCoords() { return m_tex_coords; }
        std::vector<uint32_t> &GetIndexes() { return m_indexes; }

        void LoadFromObjStream(std::istream &file);
        void LoadFrom3dFormatStream(std::istream &file);
    };

    std::optional<Model> LoadModelFromFile(const std::string &file_path);
    std::optional<Model> LoadModelFromFile(const std::string &file_path, ModelLoadFormat format);
} // namespace engine::model

#endif // MODEL_H
