// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/texture.hpp>

#include <vector>
#include <optional>
#include <map>

namespace noggit
{
  class texture_array_handler
  {
  public:
    texture_array_handler(int base_texture_unit) : _base_texture_unit(base_texture_unit) {}

    ~texture_array_handler();

    void bind();
    void bind_layer(int array_index, int texture_unit = 0);

    // return the texture array handle and the position in the array
    std::pair<std::uint64_t, int> get_texture_position(std::string const& normalized_filename);
    std::pair<std::uint64_t, int> get_texture_position_normalize_filename(std::string const& tileset_filename);

    int array_count() const { return _texture_arrays.size(); }

    // todo: grab that info beforehand to have it on creation
    void set_max_array_size(int max_layer_count) { _max_layer_count = max_layer_count; }
  private:
    std::optional<std::pair<int, int>> find_next_available_spot(int width, int height, GLuint format);


    // return the array's handle
    GLuint64 create_next_array(int width, int height, GLuint format);

    int _max_layer_count = 256;
    int _base_texture_unit;
    int _texture_count = 0;

    std::vector<opengl::texture_array> _texture_arrays;
    std::vector<std::pair<int, int>> _texture_size_for_array;
    std::vector<int> _texture_count_in_array;
    std::vector<int> _array_capacity;
    std::vector<GLuint> _array_format;

    std::map<std::string, std::pair<int, int>> _texture_positions;

    // todo: make that variable depending on size, and add the option to reupload an array
    // if there isn't enough space and
    // how many textures are stored per opengl::texture_array
    static constexpr int textures_per_array = 24;
  };
}
