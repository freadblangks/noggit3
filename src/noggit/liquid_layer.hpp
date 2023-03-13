// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/trig.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/liquid_render.hpp>
#include <math/vector_2d.hpp>
#include <util/sExtendableArray.hpp>

class MapChunk;

using liquid_indice = std::uint16_t;

// handle liquids like oceans, lakes, rivers, slime, magma
class liquid_layer
{
public:
  liquid_layer() = delete;
  liquid_layer(math::vector_3d const& base, float height, int liquid_id);
  liquid_layer(math::vector_3d const& base, mclq& liquid, int liquid_id);
  liquid_layer(MPQFile &f, std::size_t base_pos, math::vector_3d const& base, MH2O_Information const& info, std::uint64_t infomask);

  liquid_layer(liquid_layer const& other);
  liquid_layer (liquid_layer&&);

  liquid_layer& operator=(liquid_layer&&);
  liquid_layer& operator=(liquid_layer const& other);

  void save(util::sExtendableArray& adt, int base_pos, int& info_pos, int& current_pos) const;

  void update_attributes(MH2O_Attributes& attributes);
  void update_indices();
  void changeLiquidID(int id);

  void crop(MapChunk* chunk);
  void update_opacity(MapChunk* chunk, float factor);


  float min() const { return _minimum; }
  float max() const { return _maximum; }
  int liquidID() const { return _liquid_id; }

  // used for fatigue calculation
  bool subchunk_at_max_depth(int x, int z) const;

  bool hasSubchunk(int x, int z, int size = 1) const;
  void setSubchunk(int x, int z, bool water);

  bool empty() const { return !_subchunks; }
  bool full() const { return _subchunks == std::uint64_t(-1); }
  void clear() { _subchunks = std::uint64_t(0); }

  void paintLiquid(math::vector_3d const& pos
                  , float radius
                  , bool add
                  , math::radians const& angle
                  , math::radians const& orientation
                  , bool lock
                  , math::vector_3d const& origin
                  , bool override_height
                  , MapChunk* chunk
                  , float opacity_factor
                  );

  void copy_subchunk_height(int x, int z, liquid_layer const& from);

  static constexpr int vertex_buffer_size_required = 9 * 9 * sizeof(liquid_vertex);
  static constexpr int indice_buffer_size_required = 8 * 8 * 2 * 3 * sizeof(liquid_indice); // 2 triangle per quad


  void upload_data(int index_in_tile, liquid_render& render)
  {
    _index_in_tile = index_in_tile;
    update_data(render);
  }

  void update_data(liquid_render& render);

  void update_indices_info(std::vector<void*>& indices_offsets, std::vector<int>& indices_count);

  bool has_fatigue() const { return _fatigue_enabled; }
private:
  bool check_fatigue() const;
  bool _fatigue_enabled = false;

  // used to get the offset in the tile's buffers for the layer
  int _index_in_tile;
  bool _need_data_update = true;

private:
  void create_vertices(float height);

  void update_min_max();
  void update_vertex_opacity(int x, int z, MapChunk* chunk, float factor);
  int get_lod_level(math::vector_3d const& camera_pos) const;
  void set_lod_level(int lod_level);

  static int const lod_count = 3;

  int _current_lod_level = -1;
  int _current_lod_indices_count = 0;


  int _liquid_id;
  int _liquid_type;
  int _liquid_vertex_format;
  float _minimum;
  float _maximum;

  std::uint64_t _subchunks;

  std::vector<liquid_vertex> _vertices;

  std::map<int, std::vector<liquid_indice>> _indices_by_lod;

private:
  math::vector_3d pos;

  friend class MapView;
};
