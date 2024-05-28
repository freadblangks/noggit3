// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/ray.hpp>
#include <math/vector_3d.hpp>
#include <noggit/liquid_chunk.hpp>
#include <noggit/MPQ.h>
#include <noggit/MapHeaders.h>
#include <noggit/Selection.h>
#include <noggit/tool_enums.hpp>
#include <util/sExtendableArray.hpp>
#include <opengl/scoped.hpp>

#include <memory>

class MapTile;

class liquid_tile
{
public:
  liquid_tile(MapTile *pTile, float pXbase, float pZbase, bool use_mclq_green_lava);

  liquid_chunk* getChunk(int x, int z);

  void readFromFile(MPQFile &theFile, size_t basePos);
  void saveToFile(util::sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition);

  void draw ( math::frustum const& frustum
            , const float& cull_distance
            , const math::vector_3d& camera
            , bool camera_moved
            , liquid_render& render
            , opengl::scoped::use_program& water_shader
            , int animtime
            , int layer
            , display_mode display
            );
  bool hasData(size_t layer);
  void CropMiniChunk(int x, int z, MapChunk* chunkTerrain);

  void autoGen(float factor);

  void update_underground_vertices_depth();

  void setType(int type, size_t layer);
  int getType(size_t layer);

  bool has_water() const { return _has_liquids; }
  void set_has_water() { _has_liquids = true; }

  void require_buffer_update() { _need_buffer_update = true; }
  void require_buffer_regen() { _need_buffer_regen = true; }

  bool is_visible() const;

  float min_height() const { return _extents[0].y; }
  float max_height() const { return _extents[1].y; }

  bool need_recalc_extents() const { return _need_recalc_extents; }
  void require_extents_recalc();
  void recalc_extents();

  void intersect(math::ray const& ray, selection_result* results);
private:

  MapTile *tile;
  std::unique_ptr<liquid_chunk> chunks[16][16];

  float xbase;
  float zbase;

  float _radius  = 0.f;
  std::array<math::vector_3d, 2> _extents;
  std::vector<math::vector_3d> _intersect_points;

  bool _need_recalc_extents = true;
  bool _has_liquids = false;
  bool _need_visibility_update = true;

  void update_visibility( const float& cull_distance
                        , const math::frustum& frustum
                        , const math::vector_3d& camera
                        , display_mode display
                        );

private:
  bool _uploaded = false;
  bool _need_buffer_regen = true; // recreate the buffer when a layer is added or removed
  bool _need_buffer_update = true; // update the buffer when a layer is modified

  void upload(opengl::scoped::use_program& water_shader, liquid_render& render);
  void regen_buffer(liquid_render& render);
  void update_buffer(liquid_render& render);

  opengl::scoped::deferred_upload_buffers<1> _ubo;
  GLuint const& _chunks_data_ubo = _ubo[0];

  opengl::scoped::deferred_upload_vertex_arrays<1> _vertex_array;
  GLuint const& _vao = _vertex_array[0];
  opengl::scoped::deferred_upload_buffers<2> _vertex_buffers;
  GLuint const& _vertices_vbo = _vertex_buffers[0];
  GLuint const& _indices_vbo = _vertex_buffers[1];

  std::vector<void*> _indices_offsets;
  std::vector<int> _indices_count;
};
