// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/texture_array_handler.hpp>

#include <noggit/MPQ.h>
#include <noggit/TextureManager.h>
#include <opengl/context.hpp>

#include <set>

namespace
{
  // size hints for texture array size
  // the goal is to reduce vram usage and texture array count
  // without resorting to frequent resizing which would be slow
  // as it requires reuploading everything from the original array
  // values based on how many textures were used when loading
  // the 4 big continents maps fully with some added margins
  const std::map<std::pair<int, int>, int> array_layer_count_hint =
  {
    { std::pair<int, int>(8,  8),     16 },
    { std::pair<int, int>(16, 16),    32 },
    { std::pair<int, int>(16, 32),    16 },
    { std::pair<int, int>(16, 64),    32 },
    { std::pair<int, int>(32, 16),    16 },
    { std::pair<int, int>(32, 32),    128 },
    { std::pair<int, int>(32, 64),    96 },
    { std::pair<int, int>(32, 128),   64 },
    { std::pair<int, int>(32, 256),   16 },
    { std::pair<int, int>(64, 16),    16 },
    { std::pair<int, int>(64, 32),    48 },
    { std::pair<int, int>(64, 64),    256 },
    { std::pair<int, int>(64, 128),   256 },
    { std::pair<int, int>(64, 256),   128 },
    { std::pair<int, int>(128, 32),   32 },
    { std::pair<int, int>(128, 64),   256 },
    { std::pair<int, int>(128, 128),  256 }, //442
    { std::pair<int, int>(128, 256),  256 },
    { std::pair<int, int>(256, 32),   16 },
    { std::pair<int, int>(256, 64),   64 },
    { std::pair<int, int>(256, 128),  256 },
    { std::pair<int, int>(256, 256),  512 }, // 2657
    { std::pair<int, int>(512, 512),  16 },
  };
}

namespace noggit
{
  texture_array_handler::~texture_array_handler()
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

  void texture_array_handler::bind()
  {
    for (int i = 0; i < _texture_arrays.size(); ++i)
    {
      bind_layer(i);
    }
  }

  std::pair<std::uint64_t, int> texture_array_handler::get_texture_position_normalize_filename(std::string const& tileset_filename)
  {
    return get_texture_position(mpq::normalized_filename(tileset_filename));
  }

  std::pair<std::uint64_t, int> texture_array_handler::get_texture_position(std::string const& normalized_filename)
  {
    auto& it = _texture_positions.find(normalized_filename);

    if (it != _texture_positions.end())
    {
#ifdef USE_BINDLESS_TEXTURES
      return { _texture_arrays[it->second.first].get_resident_handle(), it->second.second };
#else
      return it->second;
#endif
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
      GLuint format = tex.texture_format();

      auto spot = find_next_available_spot(width, height, format);

      if (!spot)
      {
        pos = {_texture_arrays.size() , 0};
        create_next_array(width, height, format);
      }
      else
      {
        pos = spot.value();
      }

      bind_layer(pos.first);
      tex.upload_to_currently_bound_array(pos.second, shift);

      _texture_count_in_array[pos.first]++;
      _texture_positions[normalized_filename] = pos;
      _texture_count++;


#ifdef USE_BINDLESS_TEXTURES
      return { _texture_arrays[pos.first].get_resident_handle(), pos.second };
#else
      return pos;
#endif

    }
  }

  std::optional<std::pair<int, int>> texture_array_handler::find_next_available_spot(int width, int height, GLuint format)
  {
    std::pair<int, int> dimensions(width, height);

    for (int i = 0; i < _texture_size_for_array.size(); ++i)
    {
      if (_texture_size_for_array[i] == dimensions && _texture_count_in_array[i] < _array_capacity[i] && _array_format[i] == format)
      {
        return std::pair<int, int>(i, _texture_count_in_array[i]);
      }
    }

    return std::nullopt;
  }

  void texture_array_handler::bind_layer(int array_index, int texture_unit)
  {
    opengl::texture::set_active_texture(_base_texture_unit + texture_unit);
    _texture_arrays[array_index].bind();
  }

  GLuint64 texture_array_handler::create_next_array(int width, int height, GLuint format)
  {
    int index = _texture_arrays.size();

    int layer_count = textures_per_array;

    auto it = array_layer_count_hint.find({ width, height });

    if (it != array_layer_count_hint.end())
    {
      layer_count = std::min(_max_layer_count, it->second);
    }

    _texture_arrays.emplace_back();
    _texture_size_for_array.emplace_back(width, height);
    _texture_count_in_array.push_back(0);
    _array_capacity.push_back(layer_count);
    _array_format.push_back(format);

    bind_layer(index);

    int texture_level = 0;
    int w = width, h = height;

    while ((w >= 1 && h > 1) || (w > 1 && h >= 1))
    {
      gl.texImage3D(GL_TEXTURE_2D_ARRAY, texture_level++, format, w, h, layer_count, 0, GL_RGBA, GL_FLOAT, NULL);
      w = std::max(1, w >> 1);
      h = std::max(1, h >> 1);
    }

    gl.texImage3D(GL_TEXTURE_2D_ARRAY, texture_level, format, 1, 1, layer_count, 0, GL_RGBA, GL_FLOAT, NULL);

    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, texture_level);

#ifdef USE_BINDLESS_TEXTURES
    return _texture_arrays[index].get_resident_handle();
#else
    return 0;
#endif
  }
}
