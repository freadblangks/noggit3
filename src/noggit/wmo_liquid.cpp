// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/World.h>
#include <noggit/wmo_liquid.hpp>
#include <opengl/context.hpp>
#include <opengl/shader.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <string>

namespace
{

  enum liquid_basic_types
  {
    liquid_basic_types_water = 0,
    liquid_basic_types_ocean = 1,
    liquid_basic_types_magma = 2,
    liquid_basic_types_slime = 3,

    liquid_basic_types_MASK = 3,
  };
  enum liquid_types
  {
    LIQUID_WMO_Water = 13,
    LIQUID_WMO_Ocean = 14,
    LIQUID_Green_Lava = 15,
    LIQUID_WMO_Magma = 19,
    LIQUID_WMO_Slime = 20,

    LIQUID_END_BASIC_LIQUIDS = 20,
    LIQUID_FIRST_NONBASIC_LIQUID_TYPE = 21,

    LIQUID_NAXX_SLIME = 21,
  };

  liquid_types to_wmo_liquid(int x, bool ocean)
  {
    liquid_basic_types const basic(static_cast<liquid_basic_types>(x & liquid_basic_types_MASK));
    switch (basic)
    {
      default:
      case liquid_basic_types_water:
        return ocean ? LIQUID_WMO_Ocean : LIQUID_WMO_Water;
      case liquid_basic_types_ocean:
        return LIQUID_WMO_Ocean;
      case liquid_basic_types_magma:
        return LIQUID_WMO_Magma;
      case liquid_basic_types_slime:
        return LIQUID_WMO_Slime;
    }
  }
}

// todo: use material
wmo_liquid::wmo_liquid(MPQFile* f, WMOLiquidHeader const& header, WMOMaterial const&, int group_liquid, bool use_dbc_type, bool is_ocean)
  : pos(math::vector_3d(header.pos.x, header.pos.z, -header.pos.y))
  , xtiles(header.A)
  , ytiles(header.B)
{
  int liquid = initGeometry(f);

  // see: https://wowdev.wiki/WMO#how_to_determine_LiquidTypeRec_to_use
  if (use_dbc_type)
  {
    if (group_liquid < LIQUID_FIRST_NONBASIC_LIQUID_TYPE)
    {
      _liquid_id = to_wmo_liquid(group_liquid - 1, is_ocean);
    }
    else
    {
      _liquid_id = group_liquid;
    }
  }
  else
  {
    if (group_liquid == LIQUID_Green_Lava)
    {
      // todo: investigage
      // This method is most likely wrong since "liquid" is the last SMOLTile's liquid value
      // and it can vary from one liquid tile to another.
      _liquid_id = to_wmo_liquid(liquid, is_ocean);
    }
    else
    {
      if (group_liquid < LIQUID_END_BASIC_LIQUIDS)
      {
        _liquid_id = to_wmo_liquid(group_liquid, is_ocean);
      }
      else
      {
        _liquid_id = group_liquid + 1;
      }
    }
  }
}

wmo_liquid::wmo_liquid(wmo_liquid const& other)
  : pos(other.pos)
  , mTransparency(other.mTransparency)
  , xtiles(other.xtiles)
  , ytiles(other.ytiles)
  , _liquid_id(other._liquid_id)
  , _vertices(other._vertices)
  , indices(other.indices)
  , _uploaded(false)
{

}


int wmo_liquid::initGeometry(MPQFile* f)
{
  int vertex_count = (xtiles + 1) * (ytiles + 1);

  LiquidVertex const* map = reinterpret_cast<LiquidVertex const*>(f->getPointer());
  SMOLTile const* tiles = reinterpret_cast<SMOLTile const*>(f->getPointer() + vertex_count * sizeof(LiquidVertex));
  int last_liquid_id = 0;

  auto const read_uv(
    [&](int index)
    {
      return math::vector_2d( static_cast<float>(map[index].magma_vertex.s) / 255.f
                            , static_cast<float>(map[index].magma_vertex.t) / 255.f
                            );

    }
  );

  std::vector<liquid_vertex> v(vertex_count);


  for (int j = 0; j<ytiles + 1; j++)
  {
    for (int i = 0; i<xtiles + 1; ++i)
    {
      size_t p = j*(xtiles + 1) + i;
      v[p].depth = 1.f;
      v[p].uv = math::vector_2d(i, j);
      v[p].position = math::vector_3d( pos.x + UNITSIZE * i
                                             , map[p].height
                                             , pos.z - UNITSIZE * j
                                             );
    }
  }

  std::uint16_t index (0);

  for (int j = 0; j<ytiles; j++)
  {
    for (int i = 0; i<xtiles; ++i)
    {
      SMOLTile const& tile = tiles[j*xtiles + i];

      // it seems that if (liquid & 8) != 0 => do not render
      if (!(tile.liquid & 0x8))
      {
        last_liquid_id = tile.liquid;

        size_t p = j*(xtiles + 1) + i;

        if (!(tile.liquid & 2))
        {
          v[p].depth = static_cast<float>(map[p].water_vertex.flow1) / 255.0f;
          v[p + 1].depth = static_cast<float>(map[p + 1].water_vertex.flow1) / 255.0f;
          v[p + xtiles + 1 + 1].depth = static_cast<float>(map[p + xtiles + 1 + 1].water_vertex.flow1) / 255.0f;
          v[p + xtiles + 1].depth = static_cast<float>(map[p + xtiles + 1].water_vertex.flow1) / 255.0f;
        }
        else
        {
          v[p].uv = read_uv(p);
          v[p + 1].uv = read_uv(p + 1);
          v[p + xtiles + 1 + 1].uv = read_uv(p + xtiles + 1 + 1);
          v[p + xtiles + 1].uv = read_uv(p + xtiles + 1);
        }

        _vertices.push_back(v[p]);
        _vertices.push_back(v[p + 1]);
        _vertices.push_back(v[p + xtiles + 1 + 1]);
        _vertices.push_back(v[p + xtiles + 1]);

        indices.emplace_back(index);
        indices.emplace_back(index+1);
        indices.emplace_back(index+2);

        indices.emplace_back(index+2);
        indices.emplace_back(index+3);
        indices.emplace_back(index);

        index += 4;
      }
    }
  }

  _indices_count = indices.size();

  return last_liquid_id;
}

void wmo_liquid::upload(opengl::scoped::use_program& water_shader, liquid_render& render)
{
  _buffer.upload();
  _vertex_array.upload();

  auto ubo_data = render.ubo_data(_liquid_id);

  gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>(_indices_buffer, indices, GL_STATIC_DRAW);
  gl.bufferData<GL_ARRAY_BUFFER>(_vertices_buffer, _vertices.size() * sizeof(liquid_vertex), _vertices.data(), GL_STATIC_DRAW);
  gl.bufferData<GL_UNIFORM_BUFFER>(_liquid_ubo, sizeof(liquid_layer_ubo_data), &ubo_data, GL_STATIC_DRAW);

  opengl::scoped::index_buffer_manual_binder indices_binder (_indices_buffer);

  {
    opengl::scoped::vao_binder const _ (_vao);

    water_shader.attrib(_, "position", _vertices_buffer, 3, GL_FLOAT, GL_FALSE, sizeof(liquid_vertex), static_cast<char*>(0) + offsetof(liquid_vertex, position));
    water_shader.attrib(_, "tex_coord", _vertices_buffer, 2, GL_FLOAT, GL_FALSE, sizeof(liquid_vertex), static_cast<char*>(0) + offsetof(liquid_vertex, uv));
    water_shader.attrib(_, "depth", _vertices_buffer, 1, GL_FLOAT, GL_FALSE, sizeof(liquid_vertex), static_cast<char*>(0) + offsetof(liquid_vertex, depth));

    indices_binder.bind();
  }

  _uploaded = true;
}

void wmo_liquid::draw ( math::matrix_4x4 const& transform
                      , liquid_render& render
                      )
{
  opengl::scoped::use_program water_shader(render.shader_program());

  if (!_uploaded)
  {
    upload(water_shader, render);
  }

  water_shader.uniform ("transform", transform);

  opengl::scoped::vao_binder const _ (_vao);

  gl.bindBufferBase(GL_UNIFORM_BUFFER, 0, _liquid_ubo);

  gl.drawElements (GL_TRIANGLES, _indices_count, GL_UNSIGNED_SHORT, opengl::index_buffer_is_already_bound{});
}
