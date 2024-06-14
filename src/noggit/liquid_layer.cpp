// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/liquid_layer.hpp>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/Misc.h>
#include <noggit/Selection.h>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <string>

namespace
{
  inline math::vector_2d default_uv(int px, int pz)
  {
    return {static_cast<float>(px) / 4.f, static_cast<float>(pz) / 4.f};
  }
}

liquid_layer::liquid_layer(math::vector_3d const& base, float height, int liquid_id)
  : _liquid_id(liquid_id)
  , _liquid_vertex_format(0)
  , _minimum(height)
  , _maximum(height)
  , _subchunks(0)
  , pos(base)
{
  create_vertices(height);

  changeLiquidID(_liquid_id);
  update_min_max();
}

liquid_layer::liquid_layer(math::vector_3d const& base, mclq& liquid, int liquid_id)
  : _liquid_id(liquid_id)
  , _minimum(liquid.min_height)
  , _maximum(liquid.max_height)
  , _subchunks(0)
  , pos(base)
{
  changeLiquidID(_liquid_id);

  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      misc::set_bit(_subchunks, x, z, !liquid.tiles[z * 8 + x].dont_render);
    }
  }

  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
      mclq_vertex const& v = liquid.vertices[z * 9 + x];

      liquid_vertex lv;

      if (_liquid_vertex_format == 1)
      {
        lv.depth = 1.f;
        lv.uv = { static_cast<float>(v.magma.x) / 255.f, static_cast<float>(v.magma.y) / 255.f };
      }
      else
      {
        lv.depth = static_cast<float>(v.water.depth) / 255.f;
        lv.uv = default_uv(x, z);
      }

      // sometimes there's garbage data on unused tiles that mess things up
      lv.position = { pos.x + UNITSIZE * x, std::clamp(v.height, _minimum, _maximum), pos.z + UNITSIZE * z };

      _vertices.push_back(lv);
    }
  }

  update_min_max();
}

liquid_layer::liquid_layer(MPQFile &f, std::size_t base_pos, math::vector_3d const& base, MH2O_Information const& info, std::uint64_t infomask)
  : _liquid_id(info.liquid_id)
  , _liquid_vertex_format(info.liquid_vertex_format)
  , _minimum(info.minHeight)
  , _maximum(info.maxHeight)
  , _subchunks(0)
  , pos(base)
{
  int offset = 0;
  for (int z = 0; z < info.height; ++z)
  {
    for (int x = 0; x < info.width; ++x)
    {
      setSubchunk(x + info.xOffset, z + info.yOffset, (infomask >> offset) & 1);
      offset++;
    }
  }

  create_vertices(_minimum);

  if (info.ofsHeightMap)
  {
    f.seek(base_pos + info.ofsHeightMap);

    if (_liquid_vertex_format == 0 || _liquid_vertex_format == 1)
    {
      for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
      {
        for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
        {
          float h;
          f.read(&h, sizeof(float));

          _vertices[z * 9 + x].position.y = std::clamp(h, _minimum, _maximum);
        }
      }
    }

    if (_liquid_vertex_format == 1)
    {
      for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
      {
        for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
        {
          mh2o_uv uv;
          f.read(&uv, sizeof(mh2o_uv));

          _vertices[z * 9 + x].uv =
            { static_cast<float>(uv.x) / 255.f
            , static_cast<float>(uv.y) / 255.f
            };
        }
      }
    }

    if (_liquid_vertex_format == 0 || _liquid_vertex_format == 2)
    {
      for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
      {
        for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
        {
          std::uint8_t depth;
          f.read(&depth, sizeof(std::uint8_t));
          _vertices[z * 9 + x].depth = static_cast<float>(depth) / 255.f;
        }
      }
    }
  }

  changeLiquidID(_liquid_id); // to update the liquid type
  update_min_max();
}

liquid_layer::liquid_layer(math::vector_3d const& base, noggit::liquid_layer_data const& data)
  : pos(base)
  , _vertices(data.vertices)
  , _subchunks(data.subchunk_mask)
{
  changeLiquidID(data.liquid_id);
  update_min_max();
}

liquid_layer::liquid_layer(liquid_layer&& other)
  : _liquid_id(other._liquid_id)
  , _liquid_vertex_format(other._liquid_vertex_format)
  , _minimum(other._minimum)
  , _maximum(other._maximum)
  , _center(other._center)
  , _subchunks(other._subchunks)
  , _vertices(other._vertices)
  , _indices_by_lod(other._indices_by_lod)
  , _indices_count_by_lod(other._indices_count_by_lod)
  , _fatigue_enabled(other._fatigue_enabled)
  , _need_data_update(other._need_data_update)
  , pos(other.pos)
{
  // update liquid type and vertex format
  changeLiquidID(_liquid_id);
}

liquid_layer::liquid_layer(liquid_layer const& other)
  : _liquid_id(other._liquid_id)
  , _minimum(other._minimum)
  , _maximum(other._maximum)
  , _center(other._center)
  , _subchunks(other._subchunks)
  , _vertices(other._vertices)
  , _indices_by_lod(other._indices_by_lod)
  , _indices_count_by_lod(other._indices_count_by_lod)
  , _fatigue_enabled(other._fatigue_enabled)
  , _need_data_update(other._need_data_update)
  , pos(other.pos)
{
  // update liquid type and vertex format
  changeLiquidID(_liquid_id);
}

liquid_layer& liquid_layer::operator= (liquid_layer&& other)
{
  std::swap(_liquid_id, other._liquid_id);
  std::swap(_minimum, other._minimum);
  std::swap(_maximum, other._maximum);
  std::swap(_center, other._center);
  std::swap(_subchunks, other._subchunks);
  std::swap(_vertices, other._vertices);
  std::swap(_indices_by_lod, other._indices_by_lod);
  std::swap(_indices_count_by_lod, other._indices_count_by_lod);
  std::swap(_fatigue_enabled, other._fatigue_enabled);
  std::swap(_need_data_update, other._need_data_update);
  std::swap(pos, other.pos);

  // update liquid type and vertex format
  changeLiquidID(_liquid_id);
  other.changeLiquidID(other._liquid_id);

  return *this;
}

liquid_layer& liquid_layer::operator=(liquid_layer const& other)
{
  _liquid_vertex_format = other._liquid_vertex_format;
  _minimum = other._minimum;
  _maximum = other._maximum;
  _center = other._center;
  _subchunks = other._subchunks;
  _vertices = other._vertices;
  _indices_by_lod = other._indices_by_lod;
  _indices_count_by_lod = other._indices_count_by_lod;
  _fatigue_enabled = other._fatigue_enabled;
  pos = other.pos;

  // update liquid type and vertex format
  changeLiquidID(_liquid_id);

  return *this;
}

void liquid_layer::copy_data(noggit::chunk_data& data) const
{
  noggit::liquid_layer_data lld;

  lld.liquid_id = _liquid_id;
  lld.liquid_type = _liquid_type;
  lld.subchunk_mask = _subchunks;
  lld.vertices = _vertices;

  data.liquid_layers.push_back(lld);
}

void liquid_layer::create_vertices(float height)
{
  _vertices.clear();

  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
      _vertices.emplace_back(math::vector_3d(pos.x + UNITSIZE * x, height, pos.z + UNITSIZE * z)
        , default_uv(x, z)
        , 1.f
      );
    }
  }
}

void liquid_layer::save(util::sExtendableArray& adt, int base_pos, int& info_pos, int& current_pos) const
{
  int min_x = 9, min_z = 9, max_x = 0, max_z = 0;
  bool filled = true;

  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      if (hasSubchunk(x, z))
      {
        min_x = std::min(x, min_x);
        min_z = std::min(z, min_z);
        max_x = std::max(x + 1, max_x);
        max_z = std::max(z + 1, max_z);
      }
      else
      {
        filled = false;
      }
    }
  }

  MH2O_Information info;
  std::uint64_t mask = 0;

  info.liquid_id = _liquid_id;
  info.liquid_vertex_format = _liquid_vertex_format;
  info.minHeight = _minimum;
  info.maxHeight = _maximum;
  info.xOffset = min_x;
  info.yOffset = min_z;
  info.width = max_x - min_x;
  info.height = max_z - min_z;

  if (filled)
  {
    info.ofsInfoMask = 0;
  }
  else
  {
    std::uint64_t value = 1;
    for (int z = info.yOffset; z < info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x < info.xOffset + info.width; ++x)
      {
        if (hasSubchunk(x, z))
        {
          mask |= value;
        }
        value <<= 1;
      }
    }

    if (mask > 0)
    {
      info.ofsInfoMask = current_pos - base_pos;
      adt.Insert(current_pos, 8, reinterpret_cast<char*>(&mask));
      current_pos += 8;
    }
  }

  int vertices_count = (info.width + 1) * (info.height + 1);
  info.ofsHeightMap = current_pos - base_pos;

  if (_liquid_vertex_format == 0 || _liquid_vertex_format == 1)
  {
    adt.Extend(vertices_count * sizeof(float));

    for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
      {
        memcpy(adt.GetPointer<char>(current_pos).get(), &_vertices[z * 9 + x].position.y, sizeof(float));
        current_pos += sizeof(float);
      }
    }
  }
  // no heightmap/depth data for fatigue chunks
  else if(_fatigue_enabled)
  {
    info.ofsHeightMap = 0;
  }

  if (_liquid_vertex_format == 1)
  {
    adt.Extend(vertices_count * sizeof(mh2o_uv));

    for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
      {
        mh2o_uv uv;
        uv.x = static_cast<std::uint16_t>(std::min(_vertices[z * 9 + x].uv.x * 255.f, 65535.f));
        uv.y = static_cast<std::uint16_t>(std::min(_vertices[z * 9 + x].uv.y * 255.f, 65535.f));

        memcpy(adt.GetPointer<char>(current_pos).get(), &uv, sizeof(mh2o_uv));
        current_pos += sizeof(mh2o_uv);
      }
    }
  }

  if (_liquid_vertex_format == 0 || (_liquid_vertex_format == 2 && !_fatigue_enabled))
  {
    adt.Extend(vertices_count * sizeof(std::uint8_t));

    for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
      {
        std::uint8_t depth = static_cast<std::uint8_t>(std::min(_vertices[z * 9 + x].depth * 255.0f, 255.f));
        memcpy(adt.GetPointer<char>(current_pos).get(), &depth, sizeof(std::uint8_t));
        current_pos += sizeof(std::uint8_t);
      }
    }
  }

  memcpy(adt.GetPointer<char>(info_pos).get(), &info, sizeof(MH2O_Information));
  info_pos += sizeof(MH2O_Information);
}

void liquid_layer::changeLiquidID(int id)
{
  _liquid_id = id;

  try
  {
    DBCFile::Record lLiquidTypeRow = gLiquidTypeDB.getByID(_liquid_id);

    _liquid_type = lLiquidTypeRow.getInt(LiquidTypeDB::Type);

    switch (_liquid_type)
    {
    case 2: // magma
      _mclq_liquid_type = 6;
      _liquid_vertex_format = 1;
      break;
    case 3: // slime
      _mclq_liquid_type = 3;
      _liquid_vertex_format = 1;
      break;
    case 1: // ocean
      _liquid_vertex_format = 2;
      _mclq_liquid_type = 1;
      break;
    default: // river
      _liquid_vertex_format = 0;
      _mclq_liquid_type = 4;
      break;
    }
  }
  catch (...)
  {
  }
}

bool liquid_layer::check_fatigue() const
{
  // only oceans have fatigue
  if (_liquid_type != 1)
  {
    return false;
  }

  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      if (!(hasSubchunk(x, z) && subchunk_at_max_depth(x, z)))
      {
        return false;
      }
    }
  }

  return true;
}

mclq liquid_layer::to_mclq(MH2O_Attributes& attributes) const
{
  mclq mclq_data;

  mclq_data.min_height = _minimum;
  mclq_data.max_height = _maximum;

  for (int i = 0; i < 8 * 8; ++i)
  {
    if (hasSubchunk(i % 8, i / 8))
    {
      mclq_data.tiles[i].liquid_type = _mclq_liquid_type & 0x7;
      mclq_data.tiles[i].dont_render = 0;
      mclq_data.tiles[i].fishable = (attributes.fishable >> i) & 1;
      mclq_data.tiles[i].fatigue = (attributes.fatigue >> i) & 1;
    }
    else
    {
      mclq_data.tiles[i].liquid_type = 7;
      mclq_data.tiles[i].dont_render = 1;
      mclq_data.tiles[i].fishable = 0;
      mclq_data.tiles[i].fatigue = 0;
    }
  }

  for (int i = 0; i < 9 * 9; ++i)
  {
    mclq_data.vertices[i].height = _vertices[i].position.y;

    // magma and slime
    if (_liquid_type == 2 || _liquid_type == 3)
    {
      mclq_data.vertices[i].magma.x = static_cast<std::uint16_t>(std::min(_vertices[i].uv.x * 255.f, 65535.f));
      mclq_data.vertices[i].magma.y = static_cast<std::uint16_t>(std::min(_vertices[i].uv.y * 255.f, 65535.f));
    }
    else
    {
      mclq_data.vertices[i].water.depth = static_cast<std::uint8_t>(std::clamp(_vertices[i].depth * 255.f, 0.f, 255.f));
    }
  }

  return mclq_data;
}

void liquid_layer::update_attributes(MH2O_Attributes& attributes)
{
  if (check_fatigue())
  {
    attributes.fishable = -1;
    attributes.fatigue = -1;

    _fatigue_enabled = true;
  }
  else
  {
    for (int z = 0; z < 8; ++z)
    {
      for (int x = 0; x < 8; ++x)
      {
        if (hasSubchunk(x, z))
        {
          misc::set_bit(attributes.fishable, x, z, true);

          // only oceans have fatigue
          // warning: not used by TrinityCore
          if (_liquid_type == 1 && subchunk_at_max_depth(x, z))
          {
            misc::set_bit(attributes.fatigue, x, z, true);
          }
        }
      }
    }
  }
}

void liquid_layer::update_indices()
{
  _indices_by_lod.clear();

  int offset = 9 * 9 * _index_in_tile;

  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      size_t p = z * 9 + x;

      for (int lod_level = 0; lod_level < lod_count; ++lod_level)
      {
        int n = 1 << lod_level;
        if ((z % n) == 0 && (x % n) == 0)
        {
          if (hasSubchunk(x, z, n))
          {
            _indices_by_lod[lod_level].emplace_back(offset + p);
            _indices_by_lod[lod_level].emplace_back(offset + p + n * 9);
            _indices_by_lod[lod_level].emplace_back(offset + p + n * 9 + n);
            _indices_by_lod[lod_level].emplace_back(offset + p + n * 9 + n);
            _indices_by_lod[lod_level].emplace_back(offset + p + n);
            _indices_by_lod[lod_level].emplace_back(offset + p);
          }
        }
        else
        {
          break;
        }
      }
    }
  }

  for (int i = 0; i < lod_count; ++i)
  {
    _indices_count_by_lod[i] = _indices_by_lod[i].size();
  }
}

void liquid_layer::crop(MapChunk* chunk)
{
  if (_maximum < chunk->getMinHeight())
  {
    _subchunks = 0;
  }
  else
  {
    for (int z = 0; z < 8; ++z)
    {
      for (int x = 0; x < 8; ++x)
      {
        if (hasSubchunk(x, z))
        {
          int water_index = 9 * z + x, terrain_index = 17 * z + x;

          if ( _vertices[water_index +  0].position.y < chunk->vertices[terrain_index +  0].position.y
            && _vertices[water_index +  1].position.y < chunk->vertices[terrain_index +  1].position.y
            && _vertices[water_index +  9].position.y < chunk->vertices[terrain_index + 17].position.y
            && _vertices[water_index + 10].position.y < chunk->vertices[terrain_index + 18].position.y
            )
          {
            setSubchunk(x, z, false);
          }
        }
      }
    }
  }

  update_min_max();
}

void liquid_layer::update_opacity(MapChunk* chunk, float factor)
{
  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
      update_vertex_opacity(x, z, chunk, factor);
    }
  }
}

// todo: fix uvs so lava lods don't look weird
void liquid_layer::update_underground_vertices_depth(MapChunk* chunk)
{
  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
      float diff = _vertices[z * 9 + x].position.y - chunk->vertices[z * 17 + x].position.y;

      if (diff < 0.f)
      {
        _vertices[z * 9 + x].depth = 0.f;
      }
      else
      {
        if (x < 8 && z < 8 && !hasSubchunk(x, z))
        {
          _vertices[z * 9 + x].depth = 0.f;
          _vertices[z * 9 + x + 1].depth = 0.f;
          _vertices[(z + 1) * 9 + x].depth = 0.f;
          _vertices[(z + 1) * 9 + (x + 1)].depth = 0.f;
        }
      }
    }
  }
}

void liquid_layer::intersect(math::ray const& ray, selection_result* results)
{
  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      if (hasSubchunk(x, z))
      {
        int id0 = z * 9 + x;
        int id1 = z * 9 + x + 1;
        int id2 = (z + 1) * 9 + x;
        int id3 = (z + 1) * 9 + (x + 1);
        math::vector_3d const& v0 = _vertices[id0].position;
        math::vector_3d const& v1 = _vertices[id1].position;
        math::vector_3d const& v2 = _vertices[id2].position;
        math::vector_3d const& v3 = _vertices[id3].position;

        if (auto dist = ray.intersect_triangle(v0, v1, v2))
        {
          results->emplace_back
          (* dist
            , selected_liquid_layer_type
            ( this
            , std::make_tuple(id0, id1, id2)
            , ray.position(*dist)
            , _liquid_id
            )
          );
        }
        else if (auto dist = ray.intersect_triangle(v2, v3, v1))
        {
          results->emplace_back
          (* dist
            , selected_liquid_layer_type
            ( this
            , std::make_tuple(id2, id3, id1)
            , ray.position(*dist)
            , _liquid_id
            )
          );
        }
      }
    }
  }
}

int liquid_layer::mclq_flag_ordering() const
{
  switch (_mclq_liquid_type)
  {
  case 6: return 2;  // lava
  case 3: return 3;  // slime
  case 1: return 1;  // ocean
  default: return 0; // river

  }
}

bool liquid_layer::subchunk_at_max_depth(int x, int z) const
{
  for (int id_z = z; id_z <= z + 1; ++id_z)
  {
    for (int id_x = x; id_x <= x + 1; ++id_x)
    {
      if (_vertices[id_x + 9 * id_z].depth < 1.f)
      {
        return false;
      }
    }
  }

  return true;
}

bool liquid_layer::hasSubchunk(int x, int z, int size) const
{
  for (int pz = z; pz < z + size; ++pz)
  {
    for (int px = x; px < x + size; ++px)
    {
      if ((_subchunks >> (pz * 8 + px)) & 1)
      {
        return true;
      }
    }
  }
  return false;
}

void liquid_layer::setSubchunk(int x, int z, bool water)
{
  misc::set_bit(_subchunks, x, z, water);
}

void liquid_layer::paintLiquid( math::vector_3d const& cursor_pos
                              , float radius
                              , bool add
                              , math::radians const& angle
                              , math::radians const& orientation
                              , bool lock
                              , math::vector_3d const& origin
                              , bool override_height
                              , MapChunk* chunk
                              , float opacity_factor
                              )
{
  math::vector_3d ref ( lock
                      ? origin
                      : math::vector_3d (cursor_pos.x, cursor_pos.y, cursor_pos.z)
                      );

  int id = 0;

  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      if (misc::getShortestDist(cursor_pos, _vertices[id].position, UNITSIZE) <= radius)
      {
        if (add)
        {
          for (int index : {id, id + 1, id + 9, id + 10})
          {
            bool no_subchunk = !hasSubchunk(x, z);
            bool in_range = misc::dist(cursor_pos, _vertices[index].position) <= radius;

            if (no_subchunk || (in_range && override_height))
            {
              _vertices[index].position.y = misc::angledHeight(ref, _vertices[index].position, angle, orientation);
            }
            if (no_subchunk || in_range)
            {
              update_vertex_opacity(index % 9, index / 9, chunk, opacity_factor);
            }
          }
        }
        setSubchunk(x, z, add);
      }

      id++;
    }
    // to go to the next row of subchunks
    id++;
  }

  update_min_max();
}

void liquid_layer::update_min_max()
{
  _minimum = std::numeric_limits<float>::max();
  _maximum = std::numeric_limits<float>::lowest();
  int x = 0, z = 0;

  for (liquid_vertex& v : _vertices)
  {
    if (hasSubchunk(std::min(x, 7), std::min(z, 7)))
    {
      _maximum = std::max(_maximum, v.position.y);
      _minimum = std::min(_minimum, v.position.y);
    }

    if (++x == 9)
    {
      z++;
      x = 0;
    }
  }

  _center = math::vector_3d(pos.x + CHUNKSIZE * 0.5f, (_maximum + _minimum) * 0.5f, pos.z + CHUNKSIZE * 0.5f);

  // lvf = 2 means the liquid height is 0, switch to lvf 0 when that's not the case
  if (_liquid_vertex_format == 2 && (!misc::float_equals(0.f, _minimum) || !misc::float_equals(0.f, _maximum)))
  {
    _liquid_vertex_format = 0;
  }
  // use lvf 2 when possible to save space
  else if (_liquid_vertex_format == 0 && misc::float_equals(0.f, _minimum) && misc::float_equals(0.f, _maximum))
  {
    _liquid_vertex_format = 2;
  }

  _fatigue_enabled = check_fatigue();
}

void liquid_layer::copy_subchunk_height(int x, int z, liquid_layer const& from)
{
  int id = 9 * z + x;

  for (int index : {id, id + 1, id + 9, id + 10})
  {
    _vertices[index].position.y = from._vertices[index].position.y;
  }

  setSubchunk(x, z, true);
}

void liquid_layer::update_vertex_opacity(int x, int z, MapChunk* chunk, float factor)
{
  float diff = _vertices[z * 9 + x].position.y - chunk->vertices[z * 17 + x].position.y;
  _vertices[z * 9 + x].depth = std::clamp((diff + 1.0f) * factor, 0.f, 1.f);
}

int liquid_layer::get_lod_level(math::vector_3d const& camera_pos) const
{
  auto const dist ((_center - camera_pos).length());

  float f = std::min(1.f, dist / (500.f * lod_count));
  return std::clamp(static_cast<int>(f * lod_count), 0, lod_count - 1);
}

void liquid_layer::set_lod_level(int lod_level, std::vector<void*>& indices_offsets, std::vector<int>& indices_count)
{
  if (lod_level != _current_lod_level)
  {
    _current_lod_level = lod_level;
    _current_lod_indices_count = _indices_count_by_lod[_current_lod_level];

    indices_count[_index_in_tile] = _indices_count_by_lod[_current_lod_level];
    indices_offsets[_index_in_tile] = static_cast<char*>(0) + _index_in_tile * indice_buffer_size_required + lod_level_offset[_current_lod_level];
  }
}

void liquid_layer::update_data(liquid_render& render)
{
  update_indices();

  liquid_layer_ubo_data data = render.ubo_data(_liquid_id);
  int offset = 0;

  for (int i = 0; i < lod_count; ++i)
  {
    gl.bufferSubData(GL_ELEMENT_ARRAY_BUFFER, _index_in_tile * indice_buffer_size_required + offset, _indices_by_lod.at(i).size() * sizeof(liquid_indice), _indices_by_lod.at(i).data());

    offset += max_indices_per_lod_level[i] * sizeof(liquid_indice);
  }

  gl.bufferSubData(GL_ARRAY_BUFFER, _index_in_tile * vertex_buffer_size_required, _vertices.size() * sizeof(liquid_vertex), _vertices.data());
  gl.bufferSubData(GL_UNIFORM_BUFFER, _index_in_tile * sizeof(liquid_layer_ubo_data), sizeof(liquid_layer_ubo_data), &data);

  _indices_by_lod.clear();

  _need_data_update = false;
}

void liquid_layer::update_indices_info(std::vector<void*>& indices_offsets, std::vector<int>& indices_count)
{
  indices_count.push_back(_indices_count_by_lod[_current_lod_level]);
  indices_offsets.push_back(static_cast<char*>(0) + _index_in_tile * indice_buffer_size_required + lod_level_offset[_current_lod_level]);
}
