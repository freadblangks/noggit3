// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/chunk_mover.hpp>

#include <noggit/ModelInstance.h>
#include <noggit/WMOInstance.h>
#include <noggit/World.h>

#include <algorithm>

namespace noggit
{
  std::array<int, mapbufsize> chunk_mover::chunk_vertex_rot_90_lookup = chunk_mover::make_chunk_vertex_rot_90_lookup();

  chunk_mover::chunk_mover(World* world)
    : _world(world)
    , _height_ofs_property(0.f)
  {

  }


  void chunk_mover::add_to_selection(selection_type selection, bool from_multi_select)
  {
    if (selection.which() == eEntry_WMO)
    {
      WMOInstance* wmo_instance = boost::get<selected_wmo_type>(selection);
      std::uint32_t uid = wmo_instance->mUniqueID;

      if (_selected_models.find(uid) == _selected_models.end())
      {
        noggit::selected_model_data smd;
        smd.position = wmo_instance->pos;
        smd.rotation = wmo_instance->dir;
        smd.name = wmo_instance->wmo->filename;

        _selected_models[uid] = smd;
      }
    }
    else if (selection.which() == eEntry_Model)
    {
      ModelInstance* model_instance = boost::get<selected_model_type>(selection);
      std::uint32_t uid = model_instance->uid;

      if (_selected_models.find(uid) == _selected_models.end())
      {
        noggit::selected_model_data smd;
        smd.position = model_instance->pos;
        smd.rotation = model_instance->dir;
        smd.scale = model_instance->scale;
        smd.name = model_instance->model->filename;

        _selected_models[uid] = smd;
      }
    }
    else if (selection.which() == eEntry_MapChunk)
    {
      MapChunk* chunk = boost::get<selected_chunk_type>(selection).chunk;
      tile_index const& adt_index = chunk->mt->index;
      int index = chunk->chunk_index();
      int id = (adt_index.x + adt_index.z * 64) * 4096 + index;

      if (_selected_chunks.find(id) == _selected_chunks.end())
      {
        _selected_chunks.emplace(id, chunk->get_chunk_data());
        chunk->set_copied(true);
      }
    }

    // to avoid updating it each time when adding multiple things at once
    if (!from_multi_select)
    {
      update_selection_infos();
    }
  }

  void chunk_mover::add_to_selection(std::vector<selection_type> selection)
  {
    for (selection_type& entry : selection)
    {
      add_to_selection(entry, true);
    }

    update_selection_infos();
  }

  void chunk_mover::remove_from_selection(selection_type selection, bool from_multi_select)
  {
    if (selection.which() == eEntry_WMO)
    {
      WMOInstance* wmo_instance = boost::get<selected_wmo_type>(selection);
      std::uint32_t uid = wmo_instance->mUniqueID;
      _selected_models.erase(uid);
    }
    else if (selection.which() == eEntry_Model)
    {
      ModelInstance* model_instance = boost::get<selected_model_type>(selection);
      std::uint32_t uid = model_instance->uid;
      _selected_models.erase(uid);
    }
    else if (selection.which() == eEntry_MapChunk)
    {
      MapChunk* chunk = boost::get<selected_chunk_type>(selection).chunk;
      tile_index const& adt_index = chunk->mt->index;
      int index = chunk->chunk_index();
      int id = (adt_index.x + adt_index.z * 64) * 4096 + index;

      chunk->set_copied(false);

      _selected_chunks.erase(id);
    }

    // to avoid updating it each time when adding multiple things at once
    if (!from_multi_select)
    {
      update_selection_infos();
    }
  }

  void chunk_mover::remove_from_selection(std::vector<selection_type> selection)
  {
    for (selection_type& entry : selection)
    {
      remove_from_selection(entry, true);
    }

    update_selection_infos();
  }

  void chunk_mover::clear_selection()
  {
    for (auto const& it : _selected_chunks)
    {
      math::vector_3d pos = it.second.origin + math::vector_3d(5.f, 0.f, 5.f);

      MapChunk* chunk = _world->get_chunk_at(pos);

      if (chunk)
      {
        chunk->set_copied(false);
      }
    }

    _selected_chunks.clear();
    _selected_models.clear();
    update_selection_infos();
  }

  void chunk_mover::apply(bool preview_only)
  {
    if (!_selection_info || !_last_cursor_chunk || !_override_params)
    {
      return;
    }

    math::vector_2i center = _selection_info->center();

    int ofs_x = _last_cursor_chunk->x - center.x;
    int ofs_z = _last_cursor_chunk->y - center.y;

    static const math::vector_3d chunk_center_ofs(CHUNKSIZE * 0.5f, 0.f, CHUNKSIZE * 0.5f);
    math::vector_3d offset = math::vector_3d(ofs_x * CHUNKSIZE, _height_ofs_property.get(), ofs_z * CHUNKSIZE);

    std::unordered_map<int, chunk_data> const& chunks = _target_chunks.empty() ? _selected_chunks : _target_chunks;

    for (auto const& it : chunks)
    {
      // don't use a ref, otherwise the coordinates get changed each time
      // and we don't want that, and rotations prevent us from simply passing the offset
      // to the overridden chunk
      auto cd = it.second;
      cd.origin += offset;

      cd.world_id_x += ofs_x;
      cd.world_id_z += ofs_z;

      for (chunk_vertex& v : cd.vertices)
      {
        v.position += offset;
      }

      for (liquid_layer_data& lld : cd.liquid_layers)
      {
        for (liquid_vertex& lv : lld.vertices)
        {
          lv.position += offset;
        }
      }

      MapChunk* chunk = _world->get_chunk_at(cd.origin + chunk_center_ofs);

      if (chunk)
      {
        if (preview_only)
        {
          chunk->set_preview_data(cd, _override_params.value());
        }
        else
        {
          chunk->override_data(cd, _override_params.value());
          chunk->mt->changed.store(true);
        }
      }
    }

    if (!preview_only)
    {
      if (_override_params->fix_gaps)
      {
        fix_gaps();
      }

      recalc_normals_around_selection();

      for (auto& it : _selected_models)
      {
        it.second.position += offset;
      }
    }
  }

  void chunk_mover::update_selection_target(math::vector_3d const& cursor_pos, bool force_update)
  {
    if (!_selection_info)
    {
      return;
    }

    MapChunk* chunk = _world->get_chunk_at(cursor_pos);

    if (chunk)
    {
      int px = chunk->px + chunk->mt->index.x * 16;
      int pz = chunk->py + chunk->mt->index.z * 16;

      math::vector_2i pos = { px, pz };
      math::vector_2i const& size = _target_info.has_value() > 0 ? _target_info->size : _selection_info->size;
      std::unordered_map<int, bool> const& grid = _target_info.has_value() > 0 ? _target_info->grid_data : _selection_info->grid_data;


      if (_last_cursor_chunk != pos || force_update)
      {
        clear_selection_target_display();

        _last_cursor_chunk = pos;

        // move to the start
        px -= size.x / 2;
        pz -= size.y / 2;

        math::vector_3d orig(px * CHUNKSIZE + 5.f, 0.f, pz * CHUNKSIZE + 5.f);

        for (int x = 0; x < size.x; ++x)
        {
          for (int z = 0; z < size.y; ++z)
          {
            int id = x + z * size.x;

            if (grid.at(id))
            {
              MapChunk* chunk = _world->get_chunk_at(orig + math::vector_3d(x * CHUNKSIZE, 0.f, z * CHUNKSIZE));

              if (chunk)
              {
                chunk->set_is_in_paste_zone(true);
              }
            }
          }
        }

        apply(true);
      }
    }
  }

  void chunk_mover::rotate_90_deg()
  {
    if (!_selection_info)
    {
      _target_info.reset();
      return;
    }

    clear_selection_target_display();

    if (_target_chunks.empty())
    {
      _target_chunks = std::unordered_map<int, chunk_data>(_selected_chunks);
    }
    if (!_target_info)
    {
      _target_info.emplace(_selection_info.value());
    }

    _target_info->size = _target_info->size.yx();

    math::vector_2i previous_start = _target_info->start;
    math::vector_2i selection_center = _selection_info->center();

    _target_info->start =selection_center - (_target_info->size / 2);

    std::unordered_map<int, bool> rotated_grid;

    for (int i = 0; i < _target_info->size.x * _target_info->size.y; ++i)
    {
      rotated_grid[i] = false;
    }

    for (auto& it : _target_chunks)
    {
      // no ref, we need to keep the original data intact for now
      chunk_data cd = it.second;
      chunk_data const& orig = it.second;

      int old_grid_px = orig.world_id_x - previous_start.x;
      int old_grid_pz = orig.world_id_z - previous_start.y;

      int new_grid_px = old_grid_pz;
      int new_grid_pz = old_grid_px;

      if (_target_info->size.y > 1)
      {
        new_grid_pz = _target_info->size.y - new_grid_pz - 1;
      }

      int n_id = new_grid_px + new_grid_pz * _target_info->size.x;
      rotated_grid[n_id] = true;

      int px = _target_info->start.x + new_grid_px;
      int pz = _target_info->start.y + new_grid_pz;

      cd.world_id_x = px;
      cd.world_id_z = pz;

      float diff_x = (orig.world_id_x  - cd.world_id_x) * CHUNKSIZE;
      float diff_z = (orig.world_id_z  - cd.world_id_z) * CHUNKSIZE;

      cd.origin.x -= diff_x;
      cd.origin.z -= diff_z;

      for (int i = 0; i < mapbufsize; ++i)
      {
        int lookup = chunk_vertex_rot_90_lookup[i];
        cd.vertices[lookup] = orig.vertices[i];
        cd.vertices[lookup].position.x = orig.vertices[lookup].position.x - diff_x;
        cd.vertices[lookup].position.z = orig.vertices[lookup].position.z - diff_z;
      }

      for (int layer = 0; layer < cd.texture_count - 1; ++layer)
      {
        Alphamap const& orig_amap = orig.alphamaps[layer];
        Alphamap& amap = cd.alphamaps[layer];

        for (int x = 0; x < 64; ++x)
        {
          int inv_x = 63 - x;

          for (int z = 0; z < 64; ++z)
          {
            amap.setAlpha(inv_x * 64 + z, orig_amap.getAlpha(z * 64 + x));
          }
        }
      }

      for (int layer = 0; layer < cd.texture_count; ++layer)
      {
        // animation enabled
        if (cd.texture_flags[layer].flags & 0x40)
        {
          int rotation_flags = cd.texture_flags[layer].flags & 0x7;
          // clear rotation values
          cd.texture_flags[layer].flags &= ~0x7;

          // add 90° to the animation rotation
          rotation_flags -= 2;
          if (rotation_flags < 0)
          {
            rotation_flags += 8;
          }

          cd.texture_flags[layer].flags |= rotation_flags & 0x7;
        }
      }

      cd.liquid_attributes.fatigue = std::uint64_t(0);
      cd.liquid_attributes.fishable = std::uint64_t(0);

      for (int x = 0; x < 8; ++x)
      {
        int inv_x = 7 - x;

        for (int z = 0; z < 8; ++z)
        {
          int shift_orig = z * 8 + x;
          int shift_rot = inv_x * 8 + z;

          cd.liquid_attributes.fatigue |= (orig.liquid_attributes.fatigue >> shift_orig) << shift_rot;
          cd.liquid_attributes.fishable |= (orig.liquid_attributes.fishable >> shift_orig) << shift_rot;
        }
      }

      for (int layer = 0; layer < cd.liquid_layer_count; ++layer)
      {
        liquid_layer_data const& orig_layer = orig.liquid_layers.at(layer);
        liquid_layer_data& target_layer = cd.liquid_layers.at(layer);

        target_layer.subchunk_mask = std::uint64_t(0);

        for (int x = 0; x < 8; ++x)
        {
          int inv_x = 7 - x;

          for (int z = 0; z < 8; ++z)
          {
            target_layer.subchunk_mask |= (orig_layer.subchunk_mask >> (z * 8 + x)) << (inv_x * 8 + z);
          }
        }

        for (int x = 0; x < 9; ++x)
        {
          int inv_x = 8 - x;

          for (int z = 0; z < 9; ++z)
          {
            int id_orig = z * 9 + x;
            int id_rot = inv_x * 9 + z;

            target_layer.vertices[id_rot] = orig_layer.vertices[id_orig];
            target_layer.vertices[id_rot].position.x = orig_layer.vertices.at(id_rot).position.x - diff_x;
            target_layer.vertices[id_rot].position.z = orig_layer.vertices.at(id_rot).position.z - diff_z;

            // uvs are fixed for water type liquids
            if (target_layer.liquid_type == 0 || target_layer.liquid_type == 1)
            {
              target_layer.vertices[id_rot].uv = orig_layer.vertices.at(id_rot).uv;
            }
          }
        }
      }

      if (cd.shadows)
      {
        cd.shadows->data.fill(std::uint64_t(0));

        for (int x = 0; x < 64; ++x)
        {
          int inv_x = 63 - x;

          for (int z = 0; z < 64; ++z)
          {
            if (orig.shadows->data[z] & std::uint64_t(1) << x)
            {
              cd.shadows->data[inv_x] |= std::uint64_t(1) << z;
            }
          }
        }
      }

      cd.holes = 0;

      for (int x = 0; x < 4; ++x)
      {
        int inv_x = 3 - x;

        for (int z = 0; z < 4; ++z)
        {
          if (orig.holes & (1 << ((z * 4) + x)))
          {
            cd.holes |= (1 << ((inv_x * 4) + z));
          }
        }
      }

      cd.low_quality_texture_map.fill(0);
      cd.disable_doodads_map.fill(0);

      for (int x = 0; x < 8; ++x)
      {
        int inv_x = 7 - x;

        for (int z = 0; z < 8; ++z)
        {
          cd.disable_doodads_map[inv_x] |= (orig.disable_doodads_map[z] >> x & 0x1) << z;
          cd.low_quality_texture_map[inv_x] |= (orig.low_quality_texture_map[z] >> x & 0x3) << z;
        }
      }

      it.second = cd;
    }

    _target_info->grid_data = rotated_grid;

    if (_last_cursor_chunk)
    {
      update_selection_target(math::vector_3d(_last_cursor_chunk->x * CHUNKSIZE + 5.f, 0.f, _last_cursor_chunk->y * CHUNKSIZE + 5.f), true);
    }
  }

  void chunk_mover::mirror(bool horizontal)
  {
    if (!_selection_info)
    {
      _target_info.reset();
      return;
    }

    clear_selection_target_display();

    if (_target_chunks.empty())
    {
      _target_chunks = std::unordered_map<int, chunk_data>(_selected_chunks);
    }
    if (!_target_info)
    {
      _target_info.emplace(_selection_info.value());
    }

    math::vector_2i center = _target_info->center();
    math::vector_2i const& start = _target_info->start;
    math::vector_2i const& size = _target_info->size;

    std::unordered_map<int, bool> mirrored_grid;

    for (int i = 0; i < _target_info->size.x * _target_info->size.y; ++i)
    {
      mirrored_grid[i] = false;
    }

    for (auto& it : _target_chunks)
    {
      // no ref, we need to keep the original data intact for now
      chunk_data cd = it.second;
      chunk_data const& orig = it.second;

      int old_grid_px = orig.world_id_x - start.x;
      int old_grid_pz = orig.world_id_z - start.y;

      int new_grid_px = horizontal && size.x > 1 ? size.x - 1 - old_grid_px : old_grid_px;
      int new_grid_pz = horizontal || size.y < 2 ? old_grid_pz : size.y - 1 - old_grid_pz;

      int n_id = new_grid_px + new_grid_pz * size.x;
      mirrored_grid[n_id] = true;

      int px = start.x + new_grid_px;
      int pz = start.y + new_grid_pz;

      cd.world_id_x = px;
      cd.world_id_z = pz;

      float diff_x = (orig.world_id_x - cd.world_id_x) * CHUNKSIZE;
      float diff_z = (orig.world_id_z - cd.world_id_z) * CHUNKSIZE;

      cd.origin.x -= diff_x;
      cd.origin.z -= diff_z;

      for (int x = 0; x < 9; ++x)
      {
        int nx = horizontal ? 8 - x : x;

        for (int z = 0; z < 9; ++z)
        {
          int nz = horizontal ? z : 8 - z;

          int old_id = x + z * 17;
          int n_id = nx + nz * 17;

          cd.vertices[n_id] = orig.vertices[old_id];
          cd.vertices[n_id].position.x = orig.vertices[n_id].position.x - diff_x;
          cd.vertices[n_id].position.z = orig.vertices[n_id].position.z - diff_z;

          // inside vertices
          if (x < 8 && z < 8)
          {
            int in_nx = horizontal ? 7 - x : x;
            int in_nz = horizontal ? z : 7 - z;

            int in_old_id = (z + 1) * 9 + z * 8 + x;
            int in_new_id = (in_nz + 1) * 9 + in_nz * 8 + in_nx;

            cd.vertices[in_new_id] = orig.vertices[in_old_id];
            cd.vertices[in_new_id].position.x = orig.vertices[in_new_id].position.x - diff_x;
            cd.vertices[in_new_id].position.z = orig.vertices[in_new_id].position.z - diff_z;
          }
        }
      }


      for (int layer = 0; layer < cd.texture_count - 1; ++layer)
      {
        Alphamap const& orig_amap = orig.alphamaps[layer];
        Alphamap& amap = cd.alphamaps[layer];

        for (int x = 0; x < 64; ++x)
        {
          int nx = horizontal ? 63 - x : x;

          for (int z = 0; z < 64; ++z)
          {
            int nz = horizontal ? z : 63 - z;

            amap.setAlpha(nz * 64 + nx, orig_amap.getAlpha(z * 64 + x));
          }
        }
      }

      for (int layer = 0; layer < cd.texture_count; ++layer)
      {
        // animation enabled
        if (cd.texture_flags[layer].flags & 0x40)
        {
          int rotation_flags = cd.texture_flags[layer].flags & 0x7;
          // clear rotation values
          cd.texture_flags[layer].flags &= ~0x7;

          if (horizontal)
          {
            rotation_flags = 8 - rotation_flags;
          }
          else
          {
            rotation_flags = 4 - rotation_flags;
          }

          if (rotation_flags < 0)
          {
            rotation_flags += 8;
          }

          cd.texture_flags[layer].flags |= rotation_flags & 0x7;
        }
      }

      cd.liquid_attributes.fatigue = std::uint64_t(0);
      cd.liquid_attributes.fishable = std::uint64_t(0);

      for (int x = 0; x < 8; ++x)
      {
        int nx = horizontal ? 7 - x : x;

        for (int z = 0; z < 8; ++z)
        {
          int nz = horizontal ? z : 7 - z;

          int shift_orig = z * 8 + x;
          int shift_mirror = nz * 8 + nx;

          cd.liquid_attributes.fatigue |= (orig.liquid_attributes.fatigue >> shift_orig) << shift_mirror;
          cd.liquid_attributes.fishable |= (orig.liquid_attributes.fishable >> shift_orig) << shift_mirror;
        }
      }

      for (int layer = 0; layer < cd.liquid_layer_count; ++layer)
      {
        liquid_layer_data const& orig_layer = orig.liquid_layers.at(layer);
        liquid_layer_data& target_layer = cd.liquid_layers.at(layer);

        target_layer.subchunk_mask = std::uint64_t(0);

        for (int x = 0; x < 8; ++x)
        {
          int nx = horizontal ? 7 - x : x;

          for (int z = 0; z < 8; ++z)
          {
            int nz = horizontal ? z : 7 - z;
            target_layer.subchunk_mask |= (orig_layer.subchunk_mask >> (z * 8 + x)) << (nz * 8 + nx);
          }
        }

        for (int x = 0; x < 9; ++x)
        {
          int nx = horizontal ? 8 - x : x;

          for (int z = 0; z < 9; ++z)
          {
            int nz = horizontal ? z : 8 - z;
            int id_orig = z * 9 + x;
            int id_mirror = nz * 9 + nx;

            target_layer.vertices[id_mirror] = orig_layer.vertices[id_orig];
            target_layer.vertices[id_mirror].position.x = orig_layer.vertices.at(id_mirror).position.x - diff_x;
            target_layer.vertices[id_mirror].position.z = orig_layer.vertices.at(id_mirror).position.z - diff_z;

            // uvs are fixed for water type liquids
            if (target_layer.liquid_type == 0 || target_layer.liquid_type == 1)
            {
              target_layer.vertices[id_mirror].uv = orig_layer.vertices.at(id_mirror).uv;
            }
          }
        }
      }

      if (cd.shadows)
      {
        cd.shadows->data.fill(std::uint64_t(0));

        for (int x = 0; x < 64; ++x)
        {
          int nx = horizontal ? 63 - x : x;

          for (int z = 0; z < 64; ++z)
          {
            int nz = horizontal ? z : 63 - z;

            if (orig.shadows->data[z] & std::uint64_t(1) << x)
            {
              cd.shadows->data[nz] |= std::uint64_t(1) << nx;
            }
          }
        }
      }

      cd.holes = 0;

      for (int x = 0; x < 4; ++x)
      {
        int nx = horizontal ? 3 - x : x;

        for (int z = 0; z < 4; ++z)
        {
          int nz = horizontal ? z : 3 - z;

          if (orig.holes & (1 << ((z * 4) + x)))
          {
            cd.holes |= (1 << ((nz * 4) + nx));
          }
        }
      }

      cd.low_quality_texture_map.fill(0);
      cd.disable_doodads_map.fill(0);

      for (int x = 0; x < 8; ++x)
      {
        int nx = horizontal ? 7 - x : x;

        for (int z = 0; z < 8; ++z)
        {
          int nz = horizontal ? z : 7 - z;

          cd.disable_doodads_map[nz] |= (orig.disable_doodads_map[z] >> x & 0x1) << nx;
          cd.low_quality_texture_map[nz] |= (orig.low_quality_texture_map[z] >> x & 0x3) << nx;
        }
      }

      it.second = cd;
    }

    _target_info->grid_data = mirrored_grid;

    if (_last_cursor_chunk)
    {
      update_selection_target(math::vector_3d(_last_cursor_chunk->x * CHUNKSIZE + 5.f, 0.f, _last_cursor_chunk->y * CHUNKSIZE + 5.f), true);
    }
  }

  void chunk_mover::clear_selection_target_display()
  {
    if (_selection_info && _last_cursor_chunk)
    {
      int sx = _selection_info->size.x;
      int sz = _selection_info->size.y;

      if (_target_info)
      {
        sx = std::max(sx, _target_info->size.x);
        sz = std::max(sz, _target_info->size.y);
      }

      int px = _last_cursor_chunk->x - (sx / 2);
      int pz = _last_cursor_chunk->y - (sz / 2);

      math::vector_3d orig(px * CHUNKSIZE + 5.f, 0.f, pz * CHUNKSIZE + 5.f);

      for (int x = 0; x < sx; ++x)
      {
        for (int z = 0; z < sz; ++z)
        {
          MapChunk* chunk = _world->get_chunk_at(orig + math::vector_3d(x * CHUNKSIZE, 0.f, z * CHUNKSIZE));

          if (chunk)
          {
            chunk->set_is_in_paste_zone(false);
          }
        }
      }
    }
  }

  void chunk_mover::update_selection_infos()
  {
    clear_selection_target_display();

    int min_x = 64 * 16, max_x = 0;
    int min_z = 64 * 16, max_z = 0;

    for (auto const& it : _selected_chunks)
    {
      auto const& cd = it.second;
      int x = cd.world_id_x;
      int z = cd.world_id_z;

      min_x = std::min(x, min_x);
      max_x = std::max(x, max_x);
      min_z = std::min(z, min_z);
      max_z = std::max(z, max_z);
    }

    // for a 2x2 cube the size would be 1x1 otherwise
    if (min_x <= max_x && min_z <= max_z)
    {
      max_x += 1;
      max_z += 1;

      cm_selection_info cmsi;

      cmsi.start = { min_x, min_z };
      cmsi.size  = { max_x - min_x, max_z - min_z };

      // initialize the grid
      for (int i = 0; i < cmsi.size.x * cmsi.size.y; ++i)
      {
        cmsi.grid_data[i] = false;
      }

      for (auto const& it : _selected_chunks)
      {
        auto const& cd = it.second;
        int x = cd.world_id_x - min_x;
        int z = cd.world_id_z - min_z;

        int id = x + z * cmsi.size.x;

        cmsi.grid_data[id] = true;

        _selection_info = cmsi;
      }
    }
    else
    {
      _selection_info.reset();
    }

    _target_chunks.clear();
    _target_info.reset();
  }

  void chunk_mover::recalc_normals_around_selection()
  {
    if (!_selection_info || !_last_cursor_chunk)
    {
      return;
    }

    math::vector_2i const& size = _target_info->size.x > 0 ? _target_info->size : _selection_info->size;

    // update normals around the target area too
    int px = _last_cursor_chunk->x - (size.x / 2);
    int pz = _last_cursor_chunk->y - (size.y / 2);

    math::vector_3d orig(px * CHUNKSIZE + 5.f, 0.f, pz * CHUNKSIZE + 5.f);

    for (int x = -1; x < size.x + 1; ++x)
    {
      for (int z = -1; z < size.y + 1; ++z)
      {
        MapChunk* chunk = _world->get_chunk_at(orig + math::vector_3d(x * CHUNKSIZE, 0.f, z * CHUNKSIZE));

        if (chunk)
        {
          _world->recalc_norms(chunk);
        }
      }
    }
  }

  void chunk_mover::fix_gaps()
  {
    if (!_selection_info || !_last_cursor_chunk)
    {
      return;
    }

    math::vector_2i const& size = _target_info ? _target_info->size : _selection_info->size;

    int px = _last_cursor_chunk->x - (size.x / 2);
    int pz = _last_cursor_chunk->y - (size.y / 2);

    math::vector_3d orig(px * CHUNKSIZE + 5.f, 0.f, pz * CHUNKSIZE + 5.f);
    math::vector_3d ofs_left(-CHUNKSIZE, 0.f, 0.f);
    math::vector_3d ofs_up(0.f, 0.f, -CHUNKSIZE);

    for (int x = -1; x <= size.x; ++x)
    {
      for (int z = -1; z <= size.y; ++z)
      {
        MapChunk* chunk = _world->get_chunk_at(orig + math::vector_3d(x * CHUNKSIZE, 0.f, z * CHUNKSIZE));

        if (chunk)
        {
          MapChunk* left = _world->get_chunk_at(orig + math::vector_3d(x * CHUNKSIZE, 0.f, z * CHUNKSIZE) + ofs_left);
          MapChunk* up = _world->get_chunk_at(orig + math::vector_3d(x * CHUNKSIZE, 0.f, z * CHUNKSIZE) + ofs_up);

          if (left)
          {
            chunk->fixGapLeft(left);
          }
          if (up)
          {
            chunk->fixGapAbove(up);
          }
        }
      }
    }
  }
}
