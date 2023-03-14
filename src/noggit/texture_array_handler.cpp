// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/texture_array_handler.hpp>

#include <noggit/MPQ.h>
#include <noggit/TextureManager.h>
#include <opengl/context.hpp>

namespace noggit
{
  void texture_array_handler::bind()
  {
    for (int i = 0; i < _texture_arrays.size(); ++i)
    {
      bind_layer(i);
    }
  }

  std::pair<int, int> texture_array_handler::get_texture_position_normalize_filename(std::string const& tileset_filename)
  {
    return get_texture_position(mpq::normalized_filename(tileset_filename));
  }

  std::pair<int, int> texture_array_handler::get_texture_position(std::string const& normalized_filename)
  {
    auto& it = _texture_positions.find(normalized_filename);

    if (it != _texture_positions.end())
    {
      return it->second;
    }
    else
    {
      std::pair<int, int> pos;

      blp_texture tex(normalized_filename);
      tex.finishLoading();

      int height = tex.height();
      int width = tex.width();

      auto spot = find_next_available_spot(width, height);

      if (!spot)
      {
        pos = { _texture_arrays.size(), 0 };
        // create new texture array and bind it
        create_next_array(width, height);
      }
      else
      {
        pos = spot.value();
        bind_layer(pos.first);
      }

      tex.upload_to_currently_bound_array(pos.second);

      _texture_count_in_array[pos.first]++;
      _texture_positions[normalized_filename] = pos;
      _texture_count++;

      return pos;
    }
  }

  std::optional<std::pair<int, int>> texture_array_handler::find_next_available_spot(int width, int height) const
  {
    std::pair<int, int> dimensions(width, height);

    for (int i = 0; i < _texture_size_for_array.size(); ++i)
    {
      if (_texture_size_for_array[i] == dimensions && _texture_count_in_array[i] < textures_per_array)
      {
        return std::pair<int, int>(i, _texture_count_in_array[i]);
      }
    }

    return std::nullopt;
  }

  void texture_array_handler::bind_layer(int array_index)
  {
    if (array_index > 31)
    {
      return;
    }

    opengl::texture::set_active_texture(array_index + _base_texture_unit);
    _texture_arrays[array_index].bind();
  }

  void texture_array_handler::create_next_array(int width, int height)
  {
    int index = _texture_arrays.size();

    _texture_arrays.emplace_back();
    bind_layer(index);

    _texture_size_for_array.emplace_back(width, height);
    _texture_count_in_array.push_back(0);

    int texture_level = 0;
    int w = width, h = height;

    while ((w >= 1 && h > 1) || (w > 1 && h >= 1))
    {
      gl.texImage3D(GL_TEXTURE_2D_ARRAY, texture_level++, GL_RGBA8, w, h, textures_per_array, 0, GL_RGBA, GL_FLOAT, NULL);
      w = std::max(1, w >> 1);
      h = std::max(1, h >> 1);
    }

    gl.texImage3D(GL_TEXTURE_2D_ARRAY, texture_level, GL_RGBA8, 1, 1, textures_per_array, 0, GL_RGBA, GL_FLOAT, NULL);

    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, texture_level);
  }
}
