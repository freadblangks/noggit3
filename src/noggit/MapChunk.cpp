// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/frustum.hpp>
#include <math/quaternion.hpp>
#include <math/vector_3d.hpp>
#include <noggit/Brush.h>
#include <noggit/TileWater.hpp>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapHeaders.h>
#include <noggit/Misc.h>
#include <noggit/World.h>
#include <noggit/alphamap.hpp>
#include <noggit/texture_set.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/TexturingGUI.h>
#include <opengl/scoped.hpp>

#include <algorithm>
#include <iostream>
#include <map>

MapChunk::MapChunk(MapTile *maintile, MPQFile *f, bool bigAlpha, tile_mode mode)
  : _mode(mode)
  , mt(maintile)
  , use_big_alphamap(bigAlpha)
{
  uint32_t fourcc;
  uint32_t size;

  size_t base = f->getPos();

  hasMCCV = false;

  // - MCNK ----------------------------------------------
  {
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCNK');

    f->read(&header, 0x80);

    header_flags.value = header.flags;
    areaID = header.areaid;

    zbase = header.zpos;
    xbase = header.xpos;
    ybase = header.ypos;

    px = header.ix;
    py = header.iy;

    holes = header.holes;

    // correct the x and z values ^_^
    zbase = zbase*-1.0f + ZEROPOINT;
    xbase = xbase*-1.0f + ZEROPOINT;

    vmin = math::vector_3d(9999999.0f, 9999999.0f, 9999999.0f);
    vmax = math::vector_3d(-9999999.0f, -9999999.0f, -9999999.0f);
  }

  texture_set = std::make_unique<TextureSet>(header, f, base, maintile, bigAlpha, !!header_flags.flags.do_not_fix_alpha_map, mode == tile_mode::uid_fix_all, _texture_set_need_update, _animated_texture_count);

  // - MCVT ----------------------------------------------
  {
    f->seek(base + header.ofsHeight);
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCVT');

    chunk_vertex* cv_ptr = vertices.data();

    // vertices
    for (int j = 0; j < 17; ++j)
    {
      for (int i = 0; i < ((j % 2) ? 8 : 9); ++i)
      {
        float h, xpos, zpos;
        f->read(&h, 4);
        xpos = i * UNITSIZE;
        zpos = j * 0.5f * UNITSIZE;
        if (j % 2)
        {
          xpos += UNITSIZE * 0.5f;
        }
        cv_ptr->position = math::vector_3d(xbase + xpos, ybase + h, zbase + zpos);

        vmin.y = std::min(vmin.y, cv_ptr->position.y);
        vmax.y = std::max(vmax.y, cv_ptr->position.y);

        cv_ptr++;
      }
    }

    vmin.x = xbase;
    vmin.z = zbase;
    vmax.x = xbase + 8 * UNITSIZE;
    vmax.z = zbase + 8 * UNITSIZE;

    update_intersect_points();

    // use absolute y pos in vertices
    ybase = 0.0f;
    header.ypos = 0.0f;
  }
  // - MCNR ----------------------------------------------
  {
    f->seek(base + header.ofsNormal);
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCNR');

    char nor[3];
    chunk_vertex* cv_ptr = vertices.data();
    for (int i = 0; i< mapbufsize; ++i)
    {
      f->read(nor, 3);
      cv_ptr->normal = math::vector_3d(nor[0] / 127.0f, nor[2] / 127.0f, nor[1] / 127.0f);
      cv_ptr++;
    }
  }
  // - MCSH ----------------------------------------------
  if(header.ofsShadow && header.sizeShadow)
  {
    f->seek(base + header.ofsShadow);
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCSH');

    uint8_t compressed_shadow_map[64 * 64 / 8];

    // shadow map 64 x 64
    f->read(compressed_shadow_map, 0x200);
    f->seekRelative(-0x200);

    _chunk_shadow = std::make_unique<chunk_shadow>();

    uint8_t *p, *c;
    p = _chunk_shadow->_shadow_map;
    c = compressed_shadow_map;
    for (int i = 0; i<64 * 8; ++i)
    {
      for (int b = 0x01; b != 0x100; b <<= 1)
      {
        *p++ = ((*c) & b) ? 85 : 0;
      }
      c++;
    }

    if (!header_flags.flags.do_not_fix_alpha_map)
    {
      for (std::size_t i(0); i < 64; ++i)
      {
        _chunk_shadow->_shadow_map[i * 64 + 63] = _chunk_shadow->_shadow_map[i * 64 + 62];
        _chunk_shadow->_shadow_map[63 * 64 + i] = _chunk_shadow->_shadow_map[62 * 64 + i];
      }
      _chunk_shadow->_shadow_map[63 * 64 + 63] = _chunk_shadow->_shadow_map[62 * 64 + 62];
    }

    // no need to use an empty shadowmap
    if (shadow_map_is_empty())
    {
      clear_shadows();
    }
    else
    {
      mt->set_shadowmap_required();
    }
  }

  // - MCCV ----------------------------------------------
  if(header.ofsMCCV)
  {
    f->seek(base + header.ofsMCCV);
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCCV');

    if (!(header_flags.flags.has_mccv))
    {
      header_flags.flags.has_mccv = 1;
    }

    hasMCCV = true;

    unsigned char t[4];
    for (int i = 0; i < mapbufsize; ++i)
    {
      f->read(t, 4);
      vertices[i].color = math::vector_3d((float)t[2] / 127.0f, (float)t[1] / 127.0f, (float)t[0] / 127.0f);
    }
  }
  else
  {
    math::vector_3d mccv_default(1.f, 1.f, 1.f);
    for (int i = 0; i < mapbufsize; ++i)
    {
      vertices[i].color = mccv_default;
    }
  }

  if (header.sizeLiquid > 8)
  {
    f->seek(base + header.ofsLiquid);

    f->read(&fourcc, 4);
    f->seekRelative(4); // ignore the size here, the valid size is in the header

    assert(fourcc == 'MCLQ');

    int layer_count = (header.sizeLiquid - 8) / sizeof(mclq);
    std::vector<mclq> layers(layer_count);
    f->read(layers.data(), sizeof(mclq)*layer_count);

    mt->Water.getChunk(px, py)->from_mclq(layers);
    // remove the liquid flags as it'll be saved as MH2O
    header_flags.value &= ~(0xF << 2);
  }

  // no need to create indexes when applying the uid fix
  if (_mode == tile_mode::edit)
  {
    initStrip();
  }

  vcenter = (vmin + vmax) * 0.5f;
}

int MapChunk::indexLoD(int z, int x)
{
  return (z + 1) * 9 + z * 8 + x;
}

int MapChunk::indexNoLoD(int z, int x)
{
  return z * 8 + z * 9 + x;
}

void MapChunk::update_intersect_points()
{
  // update the center of the chunk and visibility when the vertices changed
  vcenter = (vmin + vmax) * 0.5f;
  _need_visibility_update = true;

  _intersect_points.clear();
  _intersect_points = misc::intersection_points(vmin, vmax);
}

int MapChunk::get_lod_level(math::vector_3d const& camera_pos, display_mode display) const
{
  float dist = display == display_mode::in_2D
             ? std::abs(camera_pos.y - vcenter.y)
             : (camera_pos - vcenter).length();

  if (dist < 500.f)
  {
    return 0;
  }
  else
  {
    // todo improve
    return 1 + std::min(lod_count - 1, static_cast<int>(dist / 1000.f));
  }
}

std::vector<uint8_t> MapChunk::compressed_shadow_map() const
{
  std::vector<uint8_t> shadow_map(64 * 64 / 8);

  if (_chunk_shadow)
  {
    for (int i = 0; i < 64 * 64; ++i)
    {
      if (_chunk_shadow->_shadow_map[i])
      {
        shadow_map[i / 8] |= 1 << i % 8;
      }
    }
  }

  return shadow_map;
}

bool MapChunk::shadow_map_is_empty() const
{
  if (!_chunk_shadow)
  {
    return true;
  }

  for (int i = 0; i < 64 * 64; ++i)
  {
    if (_chunk_shadow->_shadow_map[i])
    {
      return false;
    }
  }

  return true;
}

int MapChunk::indices_count(int lod_level) const
{
  int count = 0;

  if (lod_level == 0)
  {
    for (int x = 0; x < 8; ++x)
    {
      for (int y = 0; y < 8; ++y)
      {
        if (!isHole(x / 2, y / 2))
        {
          count += 12;
        }
      }
    }
  }
  else
  {
    int step = 1 << (lod_level - 1);

    for (int x = 0; x < 8; x += step)
    {
      for (int y = 0; y < 8; y += step)
      {
        if (!isHole(x / 2, y / 2))
        {
          count += 6;
        }
      }
    }
  }

  return count;
}

std::vector<StripType> MapChunk::strip_without_holes = {};

void MapChunk::initStrip()
{
  bool init_strip_without_holes = strip_without_holes.size() == 0;

  _indice_strips.clear();

  std::array<int, indice_buffer_count> index_count;

  for (int lod_level = 0; lod_level < indice_buffer_count; ++lod_level)
  {
    int count = indices_count(lod_level);
    _indices_count_per_lod_level[lod_level] = count;
    _indice_strips[lod_level] = std::vector<StripType>(count, 0);
    index_count[lod_level] = 0;
  }



  for (int x = 0; x<8; ++x)
  {
    for (int y = 0; y<8; ++y)
    {
      if (init_strip_without_holes)
      {
        strip_without_holes.emplace_back(indexLoD(y, x)); //9
        strip_without_holes.emplace_back(indexNoLoD(y, x)); //0
        strip_without_holes.emplace_back(indexNoLoD(y + 1, x)); //17
        strip_without_holes.emplace_back(indexLoD(y, x)); //9
        strip_without_holes.emplace_back(indexNoLoD(y + 1, x)); //17
        strip_without_holes.emplace_back(indexNoLoD(y + 1, x + 1)); //18
        strip_without_holes.emplace_back(indexLoD(y, x)); //9
        strip_without_holes.emplace_back(indexNoLoD(y + 1, x + 1)); //18
        strip_without_holes.emplace_back(indexNoLoD(y, x + 1)); //1
        strip_without_holes.emplace_back(indexLoD(y, x)); //9
        strip_without_holes.emplace_back(indexNoLoD(y, x + 1)); //1
        strip_without_holes.emplace_back(indexNoLoD(y, x)); //0
      }

      if (isHole(x / 2, y / 2))
        continue;

      // todo: better hole check ?
      for (int lod_level = 1; lod_level < indice_buffer_count; ++lod_level)
      {
        int n = 1 << (lod_level-1);
        if ((x % n) == 0 && (y % n) == 0)
        {
          int current_index = index_count[lod_level];

          _indice_strips[lod_level][current_index + 0] = (vertex_offset() + indexNoLoD(y, x)); //0
          _indice_strips[lod_level][current_index + 1] = (vertex_offset() + indexNoLoD(y + n, x)); //17
          _indice_strips[lod_level][current_index + 2] = (vertex_offset() + indexNoLoD(y + n, x + n)); //18
          _indice_strips[lod_level][current_index + 3] = (vertex_offset() + indexNoLoD(y + n, x + n)); //18
          _indice_strips[lod_level][current_index + 4] = (vertex_offset() + indexNoLoD(y, x + n)); //1
          _indice_strips[lod_level][current_index + 5] = (vertex_offset() + indexNoLoD(y, x)); //0

          index_count[lod_level] += 6;
        }
      }

      int start = vertex_offset() + indexNoLoD(y, x);
      int current_index = index_count[0];

      static std::array<int, 12> triangles = { 9,0,17, 9,17,18, 9,18,1, 9,1,0 };

      for (int i = 0; i < 12; ++i)
      {
        _indice_strips[0][current_index + i] = start + triangles[i];
      }

      index_count[0] += 12;
    }
  }



  _need_indice_buffer_update = true;
}

bool MapChunk::GetVertex(float x, float z, math::vector_3d *V)
{
  float xdiff, zdiff;

  xdiff = x - xbase;
  zdiff = z - zbase;

  const int row = static_cast<int>(zdiff / (UNITSIZE * 0.5f) + 0.5f);
  const int column = static_cast<int>((xdiff - UNITSIZE * 0.5f * (row % 2)) / UNITSIZE + 0.5f);
  if ((row < 0) || (column < 0) || (row > 16) || (column >((row % 2) ? 8 : 9)))
    return false;

  *V = vertices[17 * (row / 2) + ((row % 2) ? 9 : 0) + column].position;
  return true;
}

float MapChunk::getHeight(int x, int z)
{
  if (x > 9 || z > 9 || x < 0 || z < 0) return 0.0f;
  return vertices[indexNoLoD(x, z)].position.y;
}

float MapChunk::getMinHeight()
{
  float min (vertices[0].position.y);

  for (auto&& vertex : vertices)
  {
    min = std::min (min, vertex.position.y);
  }

  return min;
}

boost::optional<float> MapChunk::get_exact_height_at(math::vector_3d const& pos)
{
  if (pos.x < vmin.x || pos.x > vmax.x || pos.z < vmin.z || pos.z > vmax.z)
  {
    return boost::none;
  }

  // put the ray above the max height to be sure always hit the terrain
  math::ray ray({pos.x, vmax.y + 1.f, pos.z}, {0.f, -1.f, 0.f});

  float diff_x = pos.x - xbase;
  float diff_z = pos.z - zbase;

  int idx = static_cast<int>(diff_x / UNITSIZE);
  int idz = static_cast<int>(diff_z / UNITSIZE);

  float dx = std::fmod(diff_x, UNITSIZE);
  float dz = std::fmod(diff_z, UNITSIZE);

  int id_0 = dx > dz
           ? indexNoLoD(idz, idx+1)
           : indexNoLoD(idz+1, idx)
           ;
  int id_1 = (UNITSIZE - dx) > dz
           ? indexNoLoD(idz, idx)
           : indexNoLoD(idz+1, idx+1)
           ;
  int id_center = indexLoD(idz, idx);

  auto dist = ray.intersect_triangle(vertices[id_0].position, vertices[id_1].position, vertices[id_center].position);

  if (dist)
  {
    return ray.position(dist.get()).y;
  }
  else
  {
    return boost::none;
  }
}

void MapChunk::clearHeight()
{
  for (int i = 0; i < mapbufsize; ++i)
  {
    vertices[i].position.y = 0.0f;
  }

  vmin.y = 0.0f;
  vmax.y = 0.0f;

  update_intersect_points();

  _need_vao_update = true;
}

bool MapChunk::is_visible ( const float& cull_distance
                          , const math::frustum& frustum
                          , const math::vector_3d& camera
                          , display_mode display
                          ) const
{
  static const float chunk_radius = std::sqrt (CHUNKSIZE * CHUNKSIZE / 2.0f); //was (vmax - vmin).length() * 0.5f;

  float dist = display == display_mode::in_3D
             ? (camera - vcenter).length() - chunk_radius
             : std::abs(camera.y - vmax.y);

  return dist < cull_distance && frustum.intersects (_intersect_points);
}

void MapChunk::update_visibility ( const float& cull_distance
                                 , const math::frustum& frustum
                                 , const math::vector_3d& camera
                                 , display_mode display
                                 )
{
  auto lod = get_lod_level(camera, display);

  _is_visible = is_visible(cull_distance, frustum, camera, display);
  _need_visibility_update = false;
  _need_lod_update |= lod != _lod_level;
  _lod_level = lod;
}

void MapChunk::update_shader_data ( bool show_unpaintable_chunks
                                  , bool draw_paintability_overlay
                                  , bool draw_chunk_flag_overlay
                                  , bool draw_areaid_overlay
                                  , std::map<int, misc::random_color>& area_id_colors
                                  , int animtime
                                  , noggit::tileset_array_handler& tileset_handler
                                  , bool force_update
                                  )
{
  chunk_shader_data csd;

  int texture_count = texture_set->num();

  if (texture_count)
  {
    texture_set->update_adt_alphamap_if_necessary(px, py);

    for (int i = 0; i < texture_count; ++i)
    {
      if (_texture_set_need_update)
      {
        std::pair<int, int> param = tileset_handler.get_texture_position(texture_set->texture(i));

        csd.tex_array_index[i] = param.first;
        csd.tex_index_in_array[i] = param.second;
      }
      else // no change, reuse the old values
      {
        csd.tex_array_index[i] = _shader_data.tex_array_index[i];
        csd.tex_index_in_array[i] = _shader_data.tex_index_in_array[i];
      }

      if (_animated_texture_count > 0)
      {
        math::vector_2d uv_anim = texture_set->anim_uv_offset(i, animtime);
        csd.tex_animations[i] = math::vector_4d(uv_anim.x, uv_anim.y, 0.f, 0.f);
      }
    }
  }

  // normalize values "bool" values
  csd.is_textured = texture_count ? 1 : 0;
  csd.has_shadow = _chunk_shadow ? 1 : 0;

  // todo: only check if the textures have changed on the chunk
  // or the current selected texture has changed
  if (show_unpaintable_chunks && draw_paintability_overlay)
  {
    bool cant_paint = texture_count == 4
                   && noggit::ui::selected_texture::get()
                   && !canPaintTexture(*noggit::ui::selected_texture::get());

    csd.cant_paint = cant_paint ? 1 : 0;
  }
  else
  {
    csd.cant_paint = 0;
  }

  csd.draw_impassible_flag = (draw_chunk_flag_overlay && header_flags.flags.impass) ? 1 : 0;

  if (draw_areaid_overlay)
  {
    csd.areaid_color = (math::vector_4d)area_id_colors[areaID];
  }

  gl.bufferSubData(GL_UNIFORM_BUFFER, (sizeof(chunk_shader_data) * (py * 16 + px)), sizeof(chunk_shader_data), &csd);

  _texture_set_need_update = false;
  _shader_data_need_update = false;
  _shader_data = csd;
}

void MapChunk::draw ( math::frustum const& frustum
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
                    )
{
  if(_shader_data_need_update || _texture_set_need_update || _animated_texture_count > 0)
  {
    update_shader_data( show_unpaintable_chunks
                    , draw_paintability_overlay
                    , draw_chunk_flag_overlay
                    , draw_areaid_overlay
                    , area_id_colors
                    , animtime
                    , tileset_handler
                    );
  }

  if (_need_indice_buffer_update)
  {
    auto size = _indice_strips[0].size();
    gl.bufferSubData(GL_ELEMENT_ARRAY_BUFFER, indices_offset() * sizeof(StripType), _indice_strips[0].size() * sizeof(StripType), _indice_strips[0].data());

    _need_indice_buffer_update = false;
  }

  if (_need_vao_update)
  {
    gl.bufferSubData(GL_ARRAY_BUFFER, vertex_offset() * sizeof(chunk_vertex),  mapbufsize * sizeof(chunk_vertex), vertices.data());
    _need_vao_update = false;
  }
}

void MapChunk::intersect (math::ray const& ray, selection_result* results)
{
  if (!ray.intersect_bounds (vmin, vmax))
  {
    return;
  }

  for (int i (0); i < strip_without_holes.size(); i += 3)
  {
    if ( auto distance = ray.intersect_triangle ( vertices[strip_without_holes[i + 0]].position
                                                , vertices[strip_without_holes[i + 1]].position
                                                , vertices[strip_without_holes[i + 2]].position
                                                )
       )
    {
      results->emplace_back
        ( *distance
        , selected_chunk_type
            ( this
            , std::make_tuple ( strip_without_holes[i]
                              , strip_without_holes[i + 1]
                              , strip_without_holes[i + 2]
                              )
            , ray.position (*distance)
            )
        );
    }
  }
}

void MapChunk::updateVerticesData()
{
  vmin.y = std::numeric_limits<float>::max();
  vmax.y = std::numeric_limits<float>::lowest();

  for (int i(0); i < mapbufsize; ++i)
  {
    vmin.y = std::min(vmin.y, vertices[i].position.y);
    vmax.y = std::max(vmax.y, vertices[i].position.y);
  }

  update_intersect_points();

  _need_vao_update = true;

  // update adt extents each time the min/max height of a chunk might have changed
  mt->chunk_height_changed();
}

void MapChunk::recalcNorms (std::function<boost::optional<float> (float, float)> height)
{
  auto point
  (
    [&] (math::vector_3d& v, float xdiff, float zdiff)
    {
      return math::vector_3d
             ( v.x + xdiff
             , height (v.x + xdiff, v.z + zdiff).get_value_or (v.y)
             , v.z + zdiff
             );
    }
  );

  float const half_unit = UNITSIZE / 2.f;

  for (int i = 0; i<mapbufsize; ++i)
  {
    math::vector_3d const P1 (point(vertices[i].position, -half_unit, -half_unit));
    math::vector_3d const P2 (point(vertices[i].position,  half_unit, -half_unit));
    math::vector_3d const P3 (point(vertices[i].position,  half_unit,  half_unit));
    math::vector_3d const P4 (point(vertices[i].position, -half_unit,  half_unit));

    math::vector_3d const N1 ((P2 - vertices[i].position) % (P1 - vertices[i].position));
    math::vector_3d const N2 ((P3 - vertices[i].position) % (P2 - vertices[i].position));
    math::vector_3d const N3 ((P4 - vertices[i].position) % (P3 - vertices[i].position));
    math::vector_3d const N4 ((P1 - vertices[i].position) % (P4 - vertices[i].position));

    math::vector_3d Norm (N1 + N2 + N3 + N4);
    Norm.normalize();

    Norm.x = std::floor(Norm.x * 127) / 127;
    Norm.y = std::floor(Norm.y * 127) / 127;
    Norm.z = std::floor(Norm.z * 127) / 127;

    //! \todo: find out why recalculating normals without changing the terrain result in slightly different normals
    vertices[i].normal = {-Norm.z, Norm.y, -Norm.x};
  }

   _need_vao_update = true;
}

bool MapChunk::changeTerrain(math::vector_3d const& pos, float change, float radius, int BrushType, float inner_radius)
{
  float dist, xdiff, zdiff;
  bool changed = false;

  for (int i = 0; i < mapbufsize; ++i)
  {
    xdiff = vertices[i].position.x - pos.x;
    zdiff = vertices[i].position.z - pos.z;
    if (BrushType == eTerrainType_Quadra)
    {
      if ((std::abs(xdiff) < std::abs(radius / 2)) && (std::abs(zdiff) < std::abs(radius / 2)))
      {
        dist = std::sqrt(xdiff*xdiff + zdiff*zdiff);
        vertices[i].position.y += change * (1.0f - dist * inner_radius / radius);
        changed = true;
      }
    }
    else
    {
      dist = std::sqrt(xdiff*xdiff + zdiff*zdiff);
      if (dist < radius)
      {
        changed = true;

        switch (BrushType)
        {
          case eTerrainType_Flat:
            vertices[i].position.y += change;
            break;
          case eTerrainType_Linear:
            vertices[i].position.y += change * (1.0f - dist * (1.0f - inner_radius) / radius);
            break;
          case eTerrainType_Smooth:
            vertices[i].position.y += change / (1.0f + dist / radius);
            break;
          case eTerrainType_Polynom:
            vertices[i].position.y += change*((dist / radius)*(dist / radius) + dist / radius + 1.0f);
            break;
          case eTerrainType_Trigo:
            vertices[i].position.y += change*cos(dist / radius);
            break;
          case eTerrainType_Gaussian:
            vertices[i].position.y += dist < radius * inner_radius ? change * std::exp(-(std::pow(radius * inner_radius / radius, 2) / (2 * std::pow(0.39f, 2)))) : change * std::exp(-(std::pow(dist / radius, 2) / (2 * std::pow(0.39f, 2))));

            break;
          default:
            LogError << "Invalid terrain edit type (" << BrushType << ")" << std::endl;
            changed = false;
            break;
        }
      }
    }
  }
  if (changed)
  {
    updateVerticesData();
  }
  return changed;
}

bool MapChunk::hasColors()
{
  return hasMCCV;
}

void MapChunk::maybe_create_mccv()
{
  if (!hasMCCV)
  {
    for (int i = 0; i < mapbufsize; ++i)
    {
      vertices[i].color = math::vector_3d(1.f, 1.f, 1.f);
    }

    hasMCCV = true;
  }
}

bool MapChunk::ChangeMCCV(math::vector_3d const& pos, math::vector_4d const& color, float change, float radius, bool editMode)
{
  float dist;
  bool changed = false;

  if (!hasMCCV)
  {
    for (int i = 0; i < mapbufsize; ++i)
    {
      vertices[i].color.x = 1.0f; // set default shaders
      vertices[i].color.y = 1.0f;
      vertices[i].color.z = 1.0f;
    }

    changed = true;
    header_flags.flags.has_mccv = 1;
    hasMCCV = true;
  }

  for (int i = 0; i < mapbufsize; ++i)
  {
    dist = misc::dist(vertices[i].position, pos);
    if (dist <= radius)
    {
      float edit = change * (1.0f - dist / radius);
      if (editMode)
      {
        vertices[i].color.x += (color.x / 0.5f - vertices[i].color.x) * edit;
        vertices[i].color.y += (color.y / 0.5f - vertices[i].color.y) * edit;
        vertices[i].color.z += (color.z / 0.5f - vertices[i].color.z) * edit;
      }
      else
      {
        vertices[i].color.x += (1.0f - vertices[i].color.x) * edit;
        vertices[i].color.y += (1.0f - vertices[i].color.y) * edit;
        vertices[i].color.z += (1.0f - vertices[i].color.z) * edit;
      }

      vertices[i].color.x = std::min(std::max(vertices[i].color.x, 0.0f), 2.0f);
      vertices[i].color.y = std::min(std::max(vertices[i].color.y, 0.0f), 2.0f);
      vertices[i].color.z = std::min(std::max(vertices[i].color.z, 0.0f), 2.0f);

      changed = true;
    }
  }

  _need_vao_update = true;

  return changed;
}

math::vector_3d MapChunk::pickMCCV(math::vector_3d const& pos)
{
  float dist;
  float cur_dist = UNITSIZE;

  if (!hasMCCV)
  {
    return math::vector_3d(1.0f, 1.0f, 1.0f);
  }

  int v_index = 0;
  for (int i = 0; i < mapbufsize; ++i)
  {
    dist = misc::dist(vertices[i].position, pos);
    if (dist <= cur_dist)
    {
      cur_dist = dist;
      v_index = i;
    }
  }

  return vertices[v_index].color;

}

bool MapChunk::flattenTerrain ( math::vector_3d const& pos
                              , float remain
                              , float radius
                              , int BrushType
                              , flatten_mode const& mode
                              , math::vector_3d const& origin
                              , math::degrees angle
                              , math::degrees orientation
                              )
{
  bool changed (false);

  for (int i(0); i < mapbufsize; ++i)
  {
	  float const dist(misc::dist(vertices[i].position, pos));

	  if (dist >= radius)
	  {
		  continue;
	  }

	  float const ah(origin.y
		  + ((vertices[i].position.x - origin.x) * math::cos(orientation)
			  + (vertices[i].position.z - origin.z) * math::sin(orientation)
			  ) * math::tan(angle)
	  );

	  if ((!mode.lower && ah < vertices[i].position.y)
		  || (!mode.raise && ah > vertices[i].position.y)
		  )
	  {
		  continue;
	  }

	  if (BrushType == eFlattenType_Origin)
	  {
		  vertices[i].position.y = origin.y;
		  changed = true;
		  continue;
	  }

    vertices[i].position.y = math::interpolation::linear
      ( BrushType == eFlattenType_Flat ? remain
      : BrushType == eFlattenType_Linear ? remain * (1.f - dist / radius)
      : BrushType == eFlattenType_Smooth ? pow (remain, 1.f + dist / radius)
      : throw std::logic_error ("bad brush type")
      , vertices[i].position.y
      , ah
      );

    changed = true;
  }

  if (changed)
  {
    updateVerticesData();
  }

  return changed;
}

bool MapChunk::blurTerrain ( math::vector_3d const& pos
                           , float remain
                           , float radius
                           , int BrushType
                           , flatten_mode const& mode
                           , std::function<boost::optional<float> (float, float)> height
                           )
{
  bool changed (false);

  if (BrushType == eFlattenType_Origin)
  {
    return false;
  }

  for (int i (0); i < mapbufsize; ++i)
  {
    float const dist(misc::dist(vertices[i].position, pos));

    if (dist >= radius)
    {
      continue;
    }

    int Rad = (int)(radius / UNITSIZE);
    float TotalHeight = 0;
    float TotalWeight = 0;
    for (int j = -Rad * 2; j <= Rad * 2; ++j)
    {
      float tz = pos.z + j * UNITSIZE / 2;
      for (int k = -Rad; k <= Rad; ++k)
      {
        float tx = pos.x + k*UNITSIZE + (j % 2) * UNITSIZE / 2.0f;
        float dist2 = misc::dist (tx, tz, vertices[i].position.x, vertices[i].position.z);
        if (dist2 > radius)
          continue;
        auto h (height (tx, tz));
        if (h)
        {
          TotalHeight += (1.0f - dist2 / radius) * h.get();
          TotalWeight += (1.0f - dist2 / radius);
        }
      }
    }

    float target = TotalHeight / TotalWeight;
    float& y = vertices[i].position.y;

    if ((target > y && !mode.raise) || (target < y && !mode.lower))
    {
      continue;
    }

    y = math::interpolation::linear
      ( BrushType == eFlattenType_Flat ? remain
      : BrushType == eFlattenType_Linear ? remain * (1.f - dist / radius)
      : BrushType == eFlattenType_Smooth ? pow (remain, 1.f + dist / radius)
      : throw std::logic_error ("bad brush type")
      , y
      , target
      );

    changed = true;
  }

  if (changed)
  {
    updateVerticesData();
  }

  return changed;
}


void MapChunk::eraseTextures()
{
  texture_set->eraseTextures();
}

void MapChunk::change_texture_flag(scoped_blp_texture_reference const& tex, std::size_t flag, bool add)
{
  texture_set->change_texture_flag(tex, flag, add);
}

int MapChunk::addTexture(scoped_blp_texture_reference texture)
{
  return texture_set->addTexture(std::move (texture));
}

void MapChunk::switchTexture(scoped_blp_texture_reference const& oldTexture, scoped_blp_texture_reference newTexture)
{
  texture_set->replace_texture(oldTexture, std::move (newTexture));
}

bool MapChunk::paintTexture(math::vector_3d const& pos, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture)
{
  return texture_set->paintTexture(xbase, zbase, pos.x, pos.z, brush, strength, pressure, std::move (texture));
}

bool MapChunk::replaceTexture(math::vector_3d const& pos, float radius, scoped_blp_texture_reference const& old_texture, scoped_blp_texture_reference new_texture)
{
  return texture_set->replace_texture(xbase, zbase, pos.x, pos.z, radius, old_texture, std::move (new_texture));
}

bool MapChunk::canPaintTexture(scoped_blp_texture_reference texture)
{
  return texture_set->canPaintTexture(texture);
}

void MapChunk::update_shadows()
{
  if (_chunk_shadow)
  {
    opengl::texture::set_active_texture(1);
    gl.texSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, px + 16 * py, 64, 64, 1, GL_RED, GL_UNSIGNED_BYTE, _chunk_shadow->_shadow_map);
  }
}

void MapChunk::clear_shadows()
{
  _chunk_shadow.reset();
}

bool MapChunk::isHole(int i, int j) const
{
  return (holes & ((1 << ((j * 4) + i)))) != 0;
}

void MapChunk::setHole(math::vector_3d const& pos, bool big, bool add)
{
  if (big)
  {
    holes = add ? 0xFFFFFFFF : 0x0;
  }
  else
  {
    int v = 1 << ((int)((pos.z - zbase) / MINICHUNKSIZE) * 4 + (int)((pos.x - xbase) / MINICHUNKSIZE));
    holes = add ? (holes | v) : (holes & ~v);
  }

  initStrip();
}

void MapChunk::setAreaID(int ID)
{
  areaID = ID;
}

int MapChunk::getAreaID()
{
  return areaID;
}


void MapChunk::setFlag(bool changeto, uint32_t flag)
{
  if (changeto)
  {
    header_flags.value |= flag;
  }
  else
  {
    header_flags.value &= ~flag;
  }
}

void MapChunk::save(util::sExtendableArray &lADTFile, int &lCurrentPosition, int &lMCIN_Position, std::map<std::string, int> &lTextures, std::vector<WMOInstance> &lObjectInstances, std::vector<ModelInstance>& lModelInstances)
{
  int lID;
  int lMCNK_Size = 0x80;
  int lMCNK_Position = lCurrentPosition;
  lADTFile.Extend(8 + 0x80);  // This is only the size of the header. More chunks will increase the size.
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCNK', lMCNK_Size);
  lADTFile.GetPointer<MCIN>(lMCIN_Position + 8)->mEntries[py * 16 + px].offset = lCurrentPosition; // check this

                                                                                                   // MCNK data
  lADTFile.Insert(lCurrentPosition + 8, 0x80, reinterpret_cast<char*>(&(header)));
  auto const lMCNK_header = lADTFile.GetPointer<MapChunkHeader>(lCurrentPosition + 8);

  header_flags.flags.do_not_fix_alpha_map = 1;

  lMCNK_header->flags = header_flags.value;
  lMCNK_header->holes = holes;
  lMCNK_header->areaid = areaID;

  lMCNK_header->nLayers = -1;
  lMCNK_header->nDoodadRefs = -1;
  lMCNK_header->ofsHeight = -1;
  lMCNK_header->ofsNormal = -1;
  lMCNK_header->ofsLayer = -1;
  lMCNK_header->ofsRefs = -1;
  lMCNK_header->ofsAlpha = -1;
  lMCNK_header->sizeAlpha = -1;
  lMCNK_header->ofsShadow = -1;
  lMCNK_header->sizeShadow = -1;
  lMCNK_header->nMapObjRefs = -1;
  lMCNK_header->ofsMCCV = -1;

  //! \todo  Implement sound emitter support. Or not.
  lMCNK_header->ofsSndEmitters = 0;
  lMCNK_header->nSndEmitters = 0;

  lMCNK_header->ofsLiquid = 0;
  //! \todo Is this still 8 if no chunk is present? Or did they correct that?
  lMCNK_header->sizeLiquid = 8;

  lMCNK_header->ypos = vertices[0].position.y;

  memset(lMCNK_header->low_quality_texture_map, 0, 0x10);

  std::vector<uint8_t> lod_texture_map = texture_set->lod_texture_map();

  for (int i = 0; i < lod_texture_map.size(); ++i)
  {
    const size_t array_index(i / 4);
    // it's a uint2 array so we need to write the uint2 in the order they will be on disk,
    // this means writing to the highest bits of the uint8 first
    const size_t bit_index((3 - ((i) % 4)) * 2);

    lMCNK_header->low_quality_texture_map[array_index] |= ((lod_texture_map[i] & 3) << bit_index);
  }

  lCurrentPosition += 8 + 0x80;

  // MCVT
  int lMCVT_Size = mapbufsize * 4;

  lADTFile.Extend(8 + lMCVT_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCVT', lMCVT_Size);

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsHeight = lCurrentPosition - lMCNK_Position;

  auto const lHeightmap = lADTFile.GetPointer<float>(lCurrentPosition + 8);

  for (int i = 0; i < mapbufsize; ++i)
    lHeightmap[i] = vertices[i].position.y - vertices[0].position.y;

  lCurrentPosition += 8 + lMCVT_Size;
  lMCNK_Size += 8 + lMCVT_Size;

  // MCCV
  int lMCCV_Size = 0;
  if (hasMCCV)
  {
    lMCCV_Size = mapbufsize * sizeof(unsigned int);
    lADTFile.Extend(8 + lMCCV_Size);
    SetChunkHeader(lADTFile, lCurrentPosition, 'MCCV', lMCCV_Size);
    lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsMCCV = lCurrentPosition - lMCNK_Position;

    auto const lmccv = lADTFile.GetPointer<unsigned int>(lCurrentPosition + 8);

    for (int i = 0; i < mapbufsize; ++i)
    {
      lmccv[i] = (((unsigned char)(vertices[i].color.z * 127.0f) & 0xFF) << 0)
               + (((unsigned char)(vertices[i].color.y * 127.0f) & 0xFF) <<  8)
               + (((unsigned char)(vertices[i].color.x * 127.0f) & 0xFF) << 16);
    }

    lCurrentPosition += 8 + lMCCV_Size;
    lMCNK_Size += 8 + lMCCV_Size;
  }
  else
  {
    lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsMCCV = 0;
  }

  // MCNR
  int lMCNR_Size = mapbufsize * 3;

  lADTFile.Extend(8 + lMCNR_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCNR', lMCNR_Size);

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsNormal = lCurrentPosition - lMCNK_Position;

  auto const lNormals = lADTFile.GetPointer<char>(lCurrentPosition + 8);

  for (int i = 0; i < mapbufsize; ++i)
  {
    lNormals[i * 3 + 0] = static_cast<char>(vertices[i].normal.x * 127);
    lNormals[i * 3 + 1] = static_cast<char>(vertices[i].normal.z * 127);
    lNormals[i * 3 + 2] = static_cast<char>(vertices[i].normal.y * 127);
  }

  lCurrentPosition += 8 + lMCNR_Size;
  lMCNK_Size += 8 + lMCNR_Size;
  //        }

  // Unknown MCNR bytes
  // These are not in as we have data or something but just to make the files more blizzlike.
  //        {
  lADTFile.Extend(13);
  lCurrentPosition += 13;
  lMCNK_Size += 13;
  //        }

  // MCLY
  //        {
  size_t lMCLY_Size = texture_set->num() * 0x10;

  lADTFile.Extend(8 + lMCLY_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCLY', lMCLY_Size);

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsLayer = lCurrentPosition - lMCNK_Position;
  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->nLayers = texture_set->num();

  std::vector<std::vector<uint8_t>> alphamaps = texture_set->save_alpha(use_big_alphamap);
  int lMCAL_Size = 0;

  // MCLY data
  for (size_t j = 0; j < texture_set->num(); ++j)
  {
    auto const lLayer = lADTFile.GetPointer<ENTRY_MCLY>(lCurrentPosition + 8 + 0x10 * j);

    lLayer->textureID = lTextures.find(texture_set->texture(j))->second;
    lLayer->flags = texture_set->flag(j);
    lLayer->ofsAlpha = lMCAL_Size;
    lLayer->effectID = texture_set->effect(j);

    if (j == 0)
    {
      lLayer->flags &= ~(FLAG_USE_ALPHA | FLAG_ALPHA_COMPRESSED);
    }
    else
    {
      lLayer->flags |= FLAG_USE_ALPHA;
      //! \todo find out why compression fuck up textures ingame
      lLayer->flags &= ~FLAG_ALPHA_COMPRESSED;

      lMCAL_Size += alphamaps[j - 1].size();
    }
  }

  lCurrentPosition += 8 + lMCLY_Size;
  lMCNK_Size += 8 + lMCLY_Size;
  //        }

  // MCRF
  //        {
  std::list<int> lDoodadIDs;
  std::list<int> lObjectIDs;

  math::vector_3d lChunkExtents[2];
  lChunkExtents[0] = math::vector_3d(xbase, 0.0f, zbase);
  lChunkExtents[1] = math::vector_3d(xbase + CHUNKSIZE, 0.0f, zbase + CHUNKSIZE);

  // search all wmos that are inside this chunk
  lID = 0;
  for(auto const& wmo : lObjectInstances)
  {
    if (wmo.isInsideRect(lChunkExtents))
    {
      lObjectIDs.push_back(lID);
    }

    lID++;
  }

  // search all models that are inside this chunk
  lID = 0;
  for(auto const& model : lModelInstances)
  {
    if (model.isInsideRect (lChunkExtents))
    {
      lDoodadIDs.push_back(lID);
    }
    lID++;
  }

  int lMCRF_Size = 4 * (lDoodadIDs.size() + lObjectIDs.size());
  lADTFile.Extend(8 + lMCRF_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCRF', lMCRF_Size);

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsRefs = lCurrentPosition - lMCNK_Position;
  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->nDoodadRefs = lDoodadIDs.size();
  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->nMapObjRefs = lObjectIDs.size();

  // MCRF data
  auto const lReferences = lADTFile.GetPointer<int>(lCurrentPosition + 8);

  lID = 0;
  for (std::list<int>::iterator it = lDoodadIDs.begin(); it != lDoodadIDs.end(); ++it)
  {
    lReferences[lID] = *it;
    lID++;
  }

  for (std::list<int>::iterator it = lObjectIDs.begin(); it != lObjectIDs.end(); ++it)
  {
    lReferences[lID] = *it;
    lID++;
  }

  lCurrentPosition += 8 + lMCRF_Size;
  lMCNK_Size += 8 + lMCRF_Size;
  //        }

  // MCSH
  if (!shadow_map_is_empty())
  {
    header_flags.flags.has_mcsh = 1;

    int lMCSH_Size = 0x200;
    lADTFile.Extend(8 + lMCSH_Size);
    SetChunkHeader(lADTFile, lCurrentPosition, 'MCSH', lMCSH_Size);

    lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsShadow = lCurrentPosition - lMCNK_Position;
    lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->sizeShadow = 0x200;

    auto const lLayer = lADTFile.GetPointer<char>(lCurrentPosition + 8);

    auto shadow_map = compressed_shadow_map();
    memcpy(lLayer.get(), shadow_map.data(), 0x200);

    lCurrentPosition += 8 + lMCSH_Size;
    lMCNK_Size += 8 + lMCSH_Size;
  }
  else
  {
    header_flags.flags.has_mcsh = 0;
    lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsShadow = 0;
    lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->sizeShadow = 0;
  }

  // MCAL
  lADTFile.Extend(8 + lMCAL_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCAL', lMCAL_Size);

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsAlpha = lCurrentPosition - lMCNK_Position;
  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->sizeAlpha = 8 + lMCAL_Size;

  auto lAlphaMaps = lADTFile.GetPointer<char>(lCurrentPosition + 8);

  for (auto alpha : alphamaps)
  {
    memcpy(lAlphaMaps.get(), alpha.data(), alpha.size());
    lAlphaMaps += alpha.size();
  }

  lCurrentPosition += 8 + lMCAL_Size;
  lMCNK_Size += 8 + lMCAL_Size;
  //        }

  //! Don't write anything MCLQ related anymore...


  // MCSE
  int lMCSE_Size = 0;
  lADTFile.Extend(8 + lMCSE_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCSE', lMCSE_Size);

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsSndEmitters = lCurrentPosition - lMCNK_Position;
  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->nSndEmitters = lMCSE_Size / 0x1C;

  lCurrentPosition += 8 + lMCSE_Size;
  lMCNK_Size += 8 + lMCSE_Size;

  lADTFile.GetPointer<sChunkHeader>(lMCNK_Position)->mSize = lMCNK_Size;
  lADTFile.GetPointer<MCIN>(lMCIN_Position + 8)->mEntries[py * 16 + px].size = lMCNK_Size + sizeof (sChunkHeader);
}


bool MapChunk::fixGapLeft(const MapChunk* chunk)
{
  if (!chunk)
    return false;

  bool changed = false;

  for (size_t i = 0; i <= 136; i+= 17)
  {
    float h = chunk->vertices[i + 8].position.y;
    if (vertices[i].position.y != h)
    {
      vertices[i].position.y = h;
      changed = true;
    }
  }

  if (changed)
  {
    updateVerticesData();
  }

  return changed;
}

bool MapChunk::fixGapAbove(const MapChunk* chunk)
{
  if (!chunk)
    return false;

  bool changed = false;

  for (size_t i = 0; i < 9; i++)
  {
    float h = chunk->vertices[i + 136].position.y;
    if (vertices[i].position.y != h)
    {
      vertices[i].position.y = h;
      changed = true;
    }
  }

  if (changed)
  {
    updateVerticesData();
  }

  return changed;
}


void MapChunk::selectVertex(math::vector_3d const& pos, float radius, std::set<math::vector_3d*>& selected_vertices)
{
  if (misc::getShortestDist(pos.x, pos.z, xbase, zbase, CHUNKSIZE) > radius)
  {
    return;
  }

  for (int i = 0; i < mapbufsize; ++i)
  {
    if (misc::dist(pos.x, pos.z, vertices[i].position.x, vertices[i].position.z) <= radius)
    {
      selected_vertices.emplace(&vertices[i].position);
    }
  }
}

void MapChunk::selectVertex(math::vector_3d const& pos1, math::vector_3d const& pos2, std::set<math::vector_3d*>& selected_vertices)
{
  for(int i = 0; i< mapbufsize; ++i)
  {
    if(
      pos1.x<=vertices[i].position.x && pos2.x>=vertices[i].position.x &&
      pos1.z<=vertices[i].position.z && pos2.z>=vertices[i].position.z
    )
    {
      selected_vertices.emplace(&vertices[i].position);
    }
  }
}

void MapChunk::fixVertices(std::set<math::vector_3d*>& selected)
{
  std::vector<int> ids ={ 0, 1, 17, 18 };
  // iterate through each "square" of vertices
  for (int i = 0; i < 64; ++i)
  {
    int not_selected = 0, count = 0, mid_vertex = ids[0] + 9;
    float h = 0.0f;

    for (int& index : ids)
    {
      if (selected.find(&vertices[index].position) == selected.end())
      {
        not_selected = index;
      }
      else
      {
        count++;
      }
      h += vertices[index].position.y;
      index += (((i+1) % 8) == 0) ? 10 : 1;
    }

    if (count == 2)
    {
      vertices[mid_vertex].position.y = h * 0.25f;
    }
    else if (count == 3)
    {
      vertices[mid_vertex].position.y = (h - vertices[not_selected].position.y) / 3.0f;
    }
  }
}

bool MapChunk::isBorderChunk(std::set<math::vector_3d*>& selected)
{
  for (int i = 0; i < mapbufsize; ++i)
  {
    // border chunk if at least a vertex isn't selected
    if (selected.find(&vertices[i].position) == selected.end())
    {
      return true;
    }
  }

  return false;
}

ChunkWater* MapChunk::liquid_chunk() const
{
  return mt->Water.getChunk(px, py);
}

void MapChunk::update_alphamap()
{
  texture_set->update_adt_alphamap_if_necessary(px, py);
}
