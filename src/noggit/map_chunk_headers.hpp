// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/MapHeaders.h>

#include <math/vector_3d.hpp>
#include <math/vector_4d.hpp>

#include <noggit/tile_index.hpp>
#include <noggit/alphamap.hpp>

#include <array>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

struct chunk_shadow
{
  std::uint64_t data[64];
};

struct chunk_shader_data
{
  // use ints to match the layout in glsl
  int has_shadow;
  int is_textured;
  int cant_paint;
  int draw_impassible_flag;
  math::vector_4d tex_animations[4]; // anim direction + anim speed, 4th value is padding
  math::vector_4d areaid_color;
  int tex_array_index[4] = { 0,0,0,0 };
  int tex_index_in_array[4] = { 0,0,0,0 };
  int is_copied = 0;
  int is_in_paste_zone = 0;
  int pad_1, pad_2;
};

struct chunk_vertex
{
  math::vector_3d position;
  math::vector_3d normal;
  math::vector_3d color;
};

namespace noggit
{
  class chunk_data
  {
  public:
    math::vector_3d origin;
    std::array<chunk_vertex, 145> vertices;
    std::uint32_t area_id;
    std::uint32_t holes;

    tile_index adt_id;
    int id_x, id_z;

    mcnk_flags flags;
    bool use_vertex_colors;
    std::optional<chunk_shadow> shadows;
    std::array<std::uint8_t, 16> low_quality_texture_map;
    std::array<std::uint8_t, 8> disable_doodads_map;

    int texture_count;
    std::array<std::string, 4> textures;
    std::array<ENTRY_MCLY, 4> texture_flags;
    std::array<Alphamap, 3> alphamaps;

    // index of the chunk in the world
    int world_id_x() const { return adt_id.x * 16 + id_x; }
    int world_id_z() const { return adt_id.z * 16 + id_z; }

    bool operator==(chunk_data const& other)
    {
      return adt_id == other.adt_id && id_x == other.id_x && id_z == other.id_z;
    }
  };

  struct chunk_override_params
  {
    bool height;
    bool textures;
    bool alphamaps;
    bool shadows;
    bool area_id;
    bool holes;
  };
}

using chunk_indice = uint16_t;
static const int mapbufsize = 9 * 9 + 8 * 8; // chunk size
