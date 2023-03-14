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

    void bind();
    std::pair<int, int> get_texture_position(std::string const& normalized_filename);
    std::pair<int, int> get_texture_position_normalize_filename(std::string const& tileset_filename);

    int array_count() const { return _texture_arrays.size(); }
  private:
    std::optional<std::pair<int, int>> find_next_available_spot(int width, int height) const;

    void bind_layer(int array_index);
    void create_next_array(int width, int height);

    int _base_texture_unit;
    int _texture_count = 0;

    std::vector<opengl::texture_array> _texture_arrays;
    std::vector<std::pair<int, int>> _texture_size_for_array;
    std::vector<int> _texture_count_in_array;
    std::map<std::string, std::pair<int, int>> _texture_positions;

    // how many textures are stored per opengl::texture_array
    static constexpr int textures_per_array = 256;
  };
}
