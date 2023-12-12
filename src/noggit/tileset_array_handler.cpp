// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/tileset_array_handler.hpp>

#include <noggit/MPQ.h>
#include <noggit/TextureManager.h>
#include <opengl/context.hpp>

namespace noggit
{
  tileset_array_handler::~tileset_array_handler()
  {
    LogDebug << "Array Count: " << _texture_size_for_array.size() << std::endl;
    std::map<std::pair<int, int>, int> texture_count;

    for (auto& it : _texture_size_for_array)
    {
      texture_count[it] = 0;
    }

    for (int i = 0; i < _texture_size_for_array.size(); ++i)
    {
      texture_count[_texture_size_for_array[i]] += _texture_count_in_array[i];
    }

    for (auto& it : texture_count)
    {
      LogDebug << it.first.first << "x" << it.first.second << " -> " << it.second << std::endl;
    }
  }

  void tileset_array_handler::bind()
  {
    for (int i = 0; i < _texture_arrays.size(); ++i)
    {
      opengl::texture::set_active_texture(i + _base_texture_unit);
      _texture_arrays.at(i).bind();
    }
  }

  std::pair<int, int> tileset_array_handler::get_texture_position_normalize_filename(std::string const& tileset_filename)
  {
    return get_texture_position(mpq::normalized_filename(tileset_filename));
  }

  std::pair<int, int> tileset_array_handler::get_texture_position(std::string const& normalized_filename)
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
      int shift = 0;
      int mipmap_count = tex.layer_count();
      GLint format = tex.texture_format();


      // downsize large texture to save space,
      // texture unit spots and vram
      while ((width > 256 || height > 256) && (shift < mipmap_count - 1))
      {
        height >>= 1;
        width >>= 1;
        shift++;
      }

      auto spot = find_next_available_spot(width, height, format);

      if (!spot)
      {
        pos = { _texture_arrays.size(), 0 };
        // create new texture array and bind it
        create_next_array(width, height, format);
      }
      else
      {
        pos = spot.value();
        bind_layer(pos.first);
      }

      tex.upload_to_currently_bound_array(pos.second, shift);

      _texture_count_in_array[pos.first]++;
      _texture_positions[normalized_filename] = pos;
      _texture_count++;

      return pos;
    }
  }

  std::optional<std::pair<int, int>> tileset_array_handler::find_next_available_spot(int width, int height, GLint format) const
  {
    for (int i = 0; i < _texture_size_for_array.size(); ++i)
    {
      if (_texture_size_for_array[i].first == width && _texture_size_for_array[i].second == height && _texture_count_in_array[i] < textures_per_array && _texture_format[i] == format)
      {
        return std::pair<int, int>(i, _texture_count_in_array[i]);
      }
    }

    return std::nullopt;
  }

  void tileset_array_handler::bind_layer(int array_index)
  {
    opengl::texture::set_active_texture(array_index + _base_texture_unit);
    _texture_arrays[array_index].bind();
  }

  void tileset_array_handler::create_next_array(int width, int height, GLint format)
  {
    int index = _texture_arrays.size();

    _texture_arrays.emplace_back();
    bind_layer(index);

    _texture_size_for_array.emplace_back(width, height);
    _texture_count_in_array.push_back(0);
    _texture_format.push_back(format);

    int texture_level = 0;
    int w = width, h = height;

    while ((w >= 1 && h > 1) || (w > 1 && h >= 1))
    {
      gl.texImage3D(GL_TEXTURE_2D_ARRAY, texture_level++, format, w, h, textures_per_array, 0, GL_RGBA, GL_FLOAT, NULL);
      w = std::max(1, w >> 1);
      h = std::max(1, h >> 1);
    }

    gl.texImage3D(GL_TEXTURE_2D_ARRAY, texture_level, format, 1, 1, textures_per_array, 0, GL_RGBA, GL_FLOAT, NULL);

    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, texture_level);
  }
}
