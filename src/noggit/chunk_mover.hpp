// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <math/trig.hpp>
#include <noggit/MapChunk.h>
#include <noggit/MapHeaders.h>
#include <noggit/Selection.h>
#include <noggit/tile_index.hpp>

#include <array>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

class World;
class MapTile;

namespace noggit
{
  struct selected_model_data
  {
    std::string name;
    math::vector_3d position;
    math::degrees::vec3 rotation;
    float scale = 1.f;
  };

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

  class chunk_mover
  {
  public:
    chunk_mover(World* world);

    void add_to_selection(selection_type selection, bool from_multi_select = false);
    void add_to_selection(std::vector<selection_type> selection);
    void remove_from_selection(selection_type selection, bool from_multi_select = false);
    void remove_from_selection(std::vector<selection_type> selection);

    void set_height_offset(float ofs) { _height_ofs = ofs; }
    void apply();

    std::pair<int, int> selection_size() const { return _selection_size; }
    std::pair<int, int> selection_center() const { return _selection_center; }

    void update_selection_target(math::vector_3d const& cursor_pos);

  private:
    void clear_selection_target_display();
    void update_selection_infos();

  private:
    World* _world;

    float _height_ofs = 0.f;

    // in chunks
    std::pair<int, int> _selection_size = { 0, 0 };
    // both values are the position of the chunk on the map
    // eg: adt index * 64 + chunk index for each axis
    std::pair<int, int> _selection_start;
    std::pair<int, int> _selection_center;
    std::pair<int, int> _last_cursor_chunk = { -1, -1 };

    // <pos in grid, is selected>
    std::unordered_map<int, bool> _selection_grid_data;

    std::unordered_map<int, selected_model_data> _selected_models;
    // <tile index * 4096 + chunk index, chunk data>
    std::unordered_map<int, chunk_data> _selected_chunks;
  };
}
