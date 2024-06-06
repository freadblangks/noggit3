// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <math/trig.hpp>
#include <noggit/float_property.hpp>
#include <noggit/map_chunk_headers.hpp>
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


  class chunk_mover
  {
  public:
    chunk_mover(World* world);

    void add_to_selection(selection_type selection, bool from_multi_select = false);
    void add_to_selection(std::vector<selection_type> selection);
    void remove_from_selection(selection_type selection, bool from_multi_select = false);
    void remove_from_selection(std::vector<selection_type> selection);

    void apply(chunk_override_params const& params);

    std::pair<int, int> selection_size() const { return _selection_size; }
    std::pair<int, int> selection_center() const { return _selection_center; }

    void update_selection_target(math::vector_3d const& cursor_pos);

    float_property& height_offset_property() { return _height_ofs_property; }

  private:
    void clear_selection_target_display();
    void update_selection_infos();

    void recalc_normals_around_selection();
    void fix_gaps();
  private:
    World* _world;

    float_property _height_ofs_property;

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
