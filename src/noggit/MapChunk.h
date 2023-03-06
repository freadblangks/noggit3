// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/quaternion.hpp> // math::vector_4d
#include <noggit/Misc.h>
#include <noggit/ModelInstance.h>
#include <noggit/Selection.h>
#include <noggit/TextureManager.h>
#include <noggit/WMOInstance.h>
#include <noggit/map_enums.hpp>
#include <noggit/texture_set.hpp>
#include <noggit/tileset_array_handler.hpp>
#include <noggit/tool_enums.hpp>
#include <opengl/scoped.hpp>
#include <opengl/texture.hpp>
#include <util/sExtendableArray.hpp>

#include <map>
#include <memory>

class MPQFile;
namespace math
{
  class frustum;
  struct vector_4d;
}
class Brush;
class ChunkWater;
class MapTile;

using StripType = uint16_t;
static const int mapbufsize = 9 * 9 + 8 * 8; // chunk size
// 8x8 squares with 4 triangls
static const int max_indices_count = 8 * 8 * 4 * 3;

struct chunk_shadow
{
  uint8_t _shadow_map[64 * 64];
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
};

struct chunk_vertex
{
  math::vector_3d position;
  math::vector_3d normal;
  math::vector_3d color;
};

class MapChunk
{
private:
  chunk_shader_data _shader_data;
  tile_mode _mode;

  bool hasMCCV;

  int holes;

  unsigned int areaID;

  std::unique_ptr<chunk_shadow> _chunk_shadow;

  static std::vector<StripType> strip_without_holes; // it's always the same, no need to recreate it each time

  std::map<int, std::vector<StripType>> _indice_strips;
  std::map<int, int> _indices_count_per_lod_level;

  std::vector<uint8_t> compressed_shadow_map() const;
  bool shadow_map_is_empty() const;

  int indices_count(int lod_level) const;
  void initStrip();

  int indexNoLoD(int z, int x);
  int indexLoD(int z, int x);

  std::vector<math::vector_3d> _intersect_points;

  void update_intersect_points();

  int get_lod_level( math::vector_3d const& camera_pos
                                    , display_mode display
                                    ) const;

  bool _uploaded = false;
  bool _need_indice_buffer_update = true;
  bool _need_lod_update = true;
  bool _need_vao_update = true;

  static constexpr int lod_count = 2;
  static constexpr int indice_buffer_count = lod_count + 1;

public:
  MapChunk(MapTile* mt, MPQFile* f, bool bigAlpha, tile_mode mode);

  MapTile *mt;
  math::vector_3d vmin, vmax, vcenter;
  int px, py;

  int chunk_index() const { return px + 16 * py; }
  int vertex_offset() const { return chunk_index() * mapbufsize; }
  int indices_offset() const { return chunk_index() * max_indices_count; }

  MapChunkHeader header;

  float xbase, ybase, zbase;

  mcnk_flags header_flags;
  bool use_big_alphamap;

  std::unique_ptr<TextureSet> texture_set;

  std::array<chunk_vertex, mapbufsize> vertices;

  bool is_visible ( const float& cull_distance
                  , const math::frustum& frustum
                  , const math::vector_3d& camera
                  , display_mode display
                  ) const;

  int indices_count() const { return _indices_count_per_lod_level.at(0); }

  bool is_currently_visible() const { return _is_visible; }


  void update_visibility ( const float& cull_distance
                         , const math::frustum& frustum
                         , const math::vector_3d& camera
                         , display_mode display
                         );
private:
  bool _is_visible = true; // visible by default
  bool _need_visibility_update = true;
  int _lod_level = 0;

  bool _shader_data_need_update = true;
  // store those here to avoid having to overhead during the draw call preparations
  // which was significant when rendering a large map
  bool _texture_set_need_update = true;
  int _animated_texture_count = 0;
public:
  void update_shader_data ( bool show_unpaintable_chunks
                          , bool draw_paintability_overlay
                          , bool draw_chunk_flag_overlay
                          , bool draw_areaid_overlay
                          , std::map<int, misc::random_color>& area_id_colors
                          , int animtime
                          , noggit::tileset_array_handler& tileset_handler
                          , bool force_update = false
                          );

  void draw ( math::frustum const& frustum
            , opengl::scoped::use_program& mcnk_shader
            , GLuint const& tex_coord_vbo
            , const float& cull_distance
            , const math::vector_3d& camera
            , bool need_visibility_update
            , bool show_unpaintable_chunks
            , bool draw_paintability_overlay
            , bool draw_chunk_flag_overlay
            , bool draw_areaid_overlay
            , std::map<int, misc::random_color>& area_id_colors
            , int animtime
            , display_mode display
            , noggit::tileset_array_handler& tileset_handler
            );
  //! \todo only this function should be public, all others should be called from it

  void intersect (math::ray const&, selection_result*);
  bool ChangeMCCV(math::vector_3d const& pos, math::vector_4d const& color, float change, float radius, bool editMode);
  //! Initialize MCCV to 1,1,1, do nothing if already exists.
  void maybe_create_mccv();
  bool hasColors();
  math::vector_3d pickMCCV(math::vector_3d const& pos);

  ChunkWater* liquid_chunk() const;

  void updateVerticesData();
  void recalcNorms (std::function<boost::optional<float> (float, float)> height);

  //! \todo implement Action stack for these
  bool changeTerrain(math::vector_3d const& pos, float change, float radius, int BrushType, float inner_radius);
  bool flattenTerrain(math::vector_3d const& pos, float remain, float radius, int BrushType, flatten_mode const& mode, const math::vector_3d& origin, math::degrees angle, math::degrees orientation);
  bool blurTerrain ( math::vector_3d const& pos, float remain, float radius, int BrushType, flatten_mode const& mode
                   , std::function<boost::optional<float> (float, float)> height
                   );

  void selectVertex(math::vector_3d const& pos, float radius, std::set<math::vector_3d*>& selected_vertices);
  void fixVertices(std::set<math::vector_3d*>& selected);
  // for the vertex tool
  bool isBorderChunk(std::set<math::vector_3d*>& selected);

  //! \todo implement Action stack for these
  bool paintTexture(math::vector_3d const& pos, Brush *brush, float strength, float pressure, scoped_blp_texture_reference texture);
  bool replaceTexture(math::vector_3d const& pos, float radius, scoped_blp_texture_reference const& old_texture, scoped_blp_texture_reference new_texture);
  bool canPaintTexture(scoped_blp_texture_reference texture);
  int addTexture(scoped_blp_texture_reference texture);
  void switchTexture(scoped_blp_texture_reference const& oldTexture, scoped_blp_texture_reference newTexture);
  void eraseTextures();
  void change_texture_flag(scoped_blp_texture_reference const& tex, std::size_t flag, bool add);

  void update_shadows();
  void clear_shadows();

  //! \todo implement Action stack for these
  bool isHole(int i, int j) const;
  void setHole(math::vector_3d const& pos, bool big, bool add);

  void setFlag(bool value, uint32_t);

  int getAreaID();
  void setAreaID(int ID);

  bool GetVertex(float x, float z, math::vector_3d *V);
  float getHeight(int x, int z);
  float getMinHeight() const { return vmin.y; }
  boost::optional<float> get_exact_height_at(math::vector_3d const& pos);

  void clearHeight();

  //! \todo this is ugly create a build struct or sth
  void save(util::sExtendableArray &lADTFile, int &lCurrentPosition, int &lMCIN_Position, std::map<std::string, int> &lTextures, std::vector<WMOInstance> &lObjectInstances, std::vector<ModelInstance>& lModelInstances);

  // fix the gaps with the chunk to the left
  bool fixGapLeft(const MapChunk* chunk);
  // fix the gaps with the chunk above
  bool fixGapAbove(const MapChunk* chunk);

  void selectVertex(math::vector_3d const& minPos, math::vector_3d const& maxPos, std::set<math::vector_3d*>& selected_vertices);

  void update_alphamap();
};
