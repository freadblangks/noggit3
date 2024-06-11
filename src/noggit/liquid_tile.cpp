// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/liquid_chunk.hpp>
#include <noggit/Log.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/liquid_tile.hpp>

liquid_tile::liquid_tile(MapTile *pTile, float pXbase, float pZbase, bool use_mclq_green_lava)
  : tile(pTile)
  , xbase(pXbase)
  , zbase(pZbase)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x] = std::make_unique<liquid_chunk> (xbase + CHUNKSIZE * x, zbase + CHUNKSIZE * z, use_mclq_green_lava, this);
    }
  }
}

void liquid_tile::readFromFile(MPQFile &theFile, size_t basePos)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      theFile.seek(basePos + (z * 16 + x) * sizeof(MH2O_Header));
      chunks[z][x]->fromFile(theFile, basePos);

      if (chunks[z][x]->hasData(0))
      {
        _has_liquids = true;
      }
    }
  }
}

void liquid_tile::draw ( math::frustum const& frustum
                     , const float& cull_distance
                     , const math::vector_3d& camera
                     , bool camera_moved
                     , liquid_render& render
                     , opengl::scoped::use_program& water_shader
                     , int animtime
                     , int layer
                     , display_mode display
                     )
{
  if (!_uploaded)
  {
    upload(water_shader, render);
  }

  // update buffers before the visibility check as it trigger
  // a visibility update
  if (_need_buffer_regen)
  {
    regen_buffer(render);
  }
  else if (_need_buffer_update)
  {
    update_buffer(render);
  }

  if (camera_moved || _need_visibility_update)
  {
    update_visibility(cull_distance, frustum, camera, display);
  }

  if (!is_visible())
  {
    return;
  }

  gl.bindBufferBase(GL_UNIFORM_BUFFER, 0, _chunks_data_ubo);

  opengl::scoped::vao_binder const _ (_vao);

  gl.multiDrawElements(GL_TRIANGLES, _indices_count.data(), GL_UNSIGNED_SHORT, _indices_offsets.data(), _indices_count.size());
}

liquid_chunk* liquid_tile::getChunk(int x, int z)
{
  return chunks[z][x].get();
}

void liquid_tile::autoGen(float factor)
{
  _need_buffer_update = true;

  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->autoGen(tile->getChunk(x, z), factor);
    }
  }
}

void liquid_tile::update_underground_vertices_depth()
{
  _need_buffer_update = true;

  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->update_underground_vertices_depth(tile->getChunk(x, z));
    }
  }
}

void liquid_tile::saveToFile(util::sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition)
{
  if (!hasData(0))
  {
    return;
  }

  int ofsW = lCurrentPosition + 0x8; //water Header pos

  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mh2o = lCurrentPosition - 0x14; //setting offset to MH2O data in Header

  int headers_size = 256 * sizeof(MH2O_Header);
  // 8 empty bytes for the chunk header
  lADTFile.Extend(8 + headers_size);
  // set current pos after the mh2o headers
  lCurrentPosition = ofsW + headers_size;
  int header_pos = ofsW;


  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->save(lADTFile, ofsW, header_pos, lCurrentPosition);
    }
  }

  SetChunkHeader(lADTFile, ofsW - 8, 'MH2O', lCurrentPosition - ofsW);
}

bool liquid_tile::hasData(size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      if (chunks[z][x]->hasData(layer))
      {
        return true;
      }
    }
  }

  return false;
}

void liquid_tile::CropMiniChunk(int x, int z, MapChunk* chunkTerrain)
{
  require_extents_recalc();
  _need_buffer_update = true;

  chunks[z][x]->CropWater(chunkTerrain);
}

void liquid_tile::setType(int type, size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->setType(type, layer);
    }
  }
}

int liquid_tile::getType(size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      if (chunks[z][x]->hasData(layer))
      {
        return chunks[z][x]->getType(layer);
      }
    }
  }
  return 0;
}

bool liquid_tile::is_visible() const
{
  return tile->is_visible();
}

void liquid_tile::upload(opengl::scoped::use_program& water_shader, liquid_render& render)
{
  _vertex_array.upload();
  _vertex_buffers.upload();
  _ubo.upload();

  regen_buffer(render);

  opengl::scoped::vao_binder const _ (_vao);

  water_shader.attrib(_, "position", _vertices_vbo, 3, GL_FLOAT, GL_FALSE, sizeof(liquid_vertex), static_cast<char*>(0) + offsetof(liquid_vertex, position));
  water_shader.attrib(_, "tex_coord", _vertices_vbo, 2, GL_FLOAT, GL_FALSE, sizeof(liquid_vertex), static_cast<char*>(0) + offsetof(liquid_vertex, uv));
  water_shader.attrib(_, "depth", _vertices_vbo, 1, GL_FLOAT, GL_FALSE, sizeof(liquid_vertex), static_cast<char*>(0) + offsetof(liquid_vertex, depth));

  _uploaded = true;
}
void liquid_tile::regen_buffer(liquid_render& render)
{
  _indices_offsets.clear();
  _indices_count.clear();

  int total_layer_count = 0;

  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      total_layer_count += chunks[z][x]->layer_count();
    }
  }

  gl.bufferData<GL_ARRAY_BUFFER>(_vertices_vbo, total_layer_count * liquid_layer::vertex_buffer_size_required, NULL, GL_STATIC_DRAW);
  gl.bufferData<GL_ELEMENT_ARRAY_BUFFER>(_indices_vbo, total_layer_count * liquid_layer::indice_buffer_size_required, NULL, GL_STATIC_DRAW);
  gl.bufferData<GL_UNIFORM_BUFFER>(_chunks_data_ubo, total_layer_count * sizeof(liquid_layer_ubo_data), NULL, GL_STATIC_DRAW);

  opengl::scoped::vao_binder const _ (_vao);

  gl.bindBuffer(GL_ARRAY_BUFFER, _vertices_vbo);
  gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indices_vbo);
  gl.bindBuffer(GL_UNIFORM_BUFFER, _chunks_data_ubo);

  int index = 0;

  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->upload_data(index, render);
      chunks[z][x]->update_indices_info(_indices_offsets, _indices_count);
    }
  }

  _has_liquids = _indices_count.size() > 0;

  _need_visibility_update = true;
  require_extents_recalc();

  _need_buffer_regen = false;
  _need_buffer_update = false;
}
void liquid_tile::update_buffer(liquid_render& render)
{
  _indices_offsets.clear();
  _indices_count.clear();

  gl.bindBuffer(GL_ARRAY_BUFFER, _vertices_vbo);
  gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indices_vbo);
  gl.bindBuffer(GL_UNIFORM_BUFFER, _chunks_data_ubo);

  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->update_data(render);
      chunks[z][x]->update_indices_info(_indices_offsets, _indices_count);
    }
  }

  _has_liquids = _indices_count.size() > 0;

  require_extents_recalc();
  _need_visibility_update = true;

  _need_buffer_update = false;
}

void liquid_tile::require_extents_recalc()
{
  _need_recalc_extents = true;

  tile->water_height_changed();
}

void liquid_tile::recalc_extents()
{
  _extents[0] = math::vector_3d::max();
  _extents[1] = math::vector_3d::min();

  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      // only take into account chunks with liquids
      if (chunks[z][x]->layer_count() > 0)
      {
        _extents[0] = math::min(_extents[0], chunks[z][x]->min());
        _extents[1] = math::max(_extents[1], chunks[z][x]->max());
      }
    }
  }

  _radius = (_extents[0] - _extents[1]).length() * 0.5f;

  _intersect_points.clear();
  _intersect_points = misc::intersection_points(_extents[0], _extents[1]);

  _need_recalc_extents = false;

  // notify the adt something changed
  // /!\ make sure the adt's _need_recalc_extents isn't set to false before
  //  the water tile extents are updated
  tile->water_height_changed();

}

void liquid_tile::intersect(math::ray const& ray, selection_result* results)
{
  if (_need_recalc_extents)
  {
    recalc_extents();
  }

  if (!ray.intersect_bounds(_extents[0], _extents[1]))
  {
    return;
  }

  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->intersect(ray, results);
    }
  }
}

void liquid_tile::update_visibility ( const float& cull_distance
                                    , const math::frustum& frustum
                                    , const math::vector_3d& camera
                                    , display_mode display
                                    )
{
  if (_need_recalc_extents)
  {
    recalc_extents();
  }

  _need_visibility_update = false;

  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->update_lod_level(camera, _indices_offsets, _indices_count);
    }
  }
}
