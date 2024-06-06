// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/chunk_mover.hpp>

#include <noggit/ModelInstance.h>
#include <noggit/WMOInstance.h>
#include <noggit/World.h>

#include <algorithm>

namespace noggit
{
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

  void chunk_mover::apply(chunk_override_params const& params)
  {
    if (_selection_size.first <= 0 || _last_cursor_chunk.first < 0)
    {
      return;
    }

    int ofs_x = _last_cursor_chunk.first - _selection_center.first;
    int ofs_z = _last_cursor_chunk.second - _selection_center.second;

    static const math::vector_3d chunk_center_ofs(CHUNKSIZE * 0.5f, 0.f, CHUNKSIZE * 0.5f);
    math::vector_3d offset = math::vector_3d(ofs_x * CHUNKSIZE, _height_ofs_property.get(), ofs_z * CHUNKSIZE);

    for (auto const& it : _selected_chunks)
    {
      // don't use a ref, otherwise the coordinates get changed each time
      // and we don't want that, and rotations prevent us from simply passing the offset
      // to the overridden chunk
      auto cd = it.second;
      cd.origin += offset;

      cd.id_x += ofs_x;
      cd.id_z += ofs_z;

      // clamp the ids to [0;15]
      if (cd.id_x < 0)
      {
        cd.id_x += 16 * (1 + cd.id_x / 16);
      }
      if (cd.id_z < 0)
      {
        cd.id_z += 16 * (1 + cd.id_z / 16);
      }

      cd.id_x = cd.id_x % 16;
      cd.id_z = cd.id_z % 16;

      for (chunk_vertex& v : cd.vertices)
      {
        v.position += offset;
      }

      MapChunk* chunk = _world->get_chunk_at(cd.origin + chunk_center_ofs);

      if (chunk)
      {
        chunk->override_data(cd, params);
        chunk->mt->changed.store(true);
      }
    }

    if (params.fix_gaps)
    {
      fix_gaps();
    }

    recalc_normals_around_selection();

    for (auto& it : _selected_models)
    {
      it.second.position += offset;
    }
  }

  void chunk_mover::update_selection_target(math::vector_3d const& cursor_pos)
  {
    if (_selection_size.first <= 0)
    {
      return;
    }

    MapChunk* chunk = _world->get_chunk_at(cursor_pos);

    if (chunk)
    {
      int px = chunk->px + chunk->mt->index.x * 16;
      int pz = chunk->py + chunk->mt->index.z * 16;

      std::pair<int, int> pos = { px, pz };

      if (_last_cursor_chunk != pos)
      {
        clear_selection_target_display();

        _last_cursor_chunk = pos;

        // move to the start
        px -= _selection_size.first / 2;
        pz -= _selection_size.second / 2;

        math::vector_3d orig(px * CHUNKSIZE + 5.f, 0.f, pz * CHUNKSIZE + 5.f);

        for (int x = 0; x < _selection_size.first; ++x)
        {
          for (int z = 0; z < _selection_size.second; ++z)
          {
            int id = x + z * _selection_size.first;

            if (_selection_grid_data[id])
            {
              MapChunk* chunk = _world->get_chunk_at(orig + math::vector_3d(x * CHUNKSIZE, 0.f, z * CHUNKSIZE));

              if (chunk)
              {
                chunk->set_is_in_paste_zone(true);
              }
            }
          }
        }
      }
    }
  }

  void chunk_mover::clear_selection_target_display()
  {
    if (_selection_size.first > 0 && _last_cursor_chunk.first > -1)
    {
      int px = _last_cursor_chunk.first - (_selection_size.first / 2);
      int pz = _last_cursor_chunk.second - (_selection_size.second/ 2);

      math::vector_3d orig(px * CHUNKSIZE + 5.f, 0.f, pz * CHUNKSIZE + 5.f);

      for (int x = 0; x < _selection_size.first; ++x)
      {
        for (int z = 0; z < _selection_size.second; ++z)
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
      int x = cd.world_id_x();
      int z = cd.world_id_z();

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

      _selection_start = { min_x, min_z };
      _selection_size = { max_x - min_x, max_z - min_z };
      _selection_center = { (max_x + min_x) / 2, (max_z + min_z) / 2 };
    }
    else
    {
      // important to reset the selection size
      // it's used to check if there is any chunk selected
      _selection_size = { 0,0 };
      _selection_grid_data.clear();
    }



    _selection_grid_data.clear();

    // initialize the grid
    for (int i = 0; i < _selection_size.first * _selection_size.second; ++i)
    {
      _selection_grid_data[i] = false;
    }

    for (auto const& it : _selected_chunks)
    {
      auto const& cd = it.second;
      int x = cd.world_id_x() - min_x;
      int z = cd.world_id_z() - min_z;

      int id = x + z * _selection_size.first;

      _selection_grid_data[id] = true;
    }
  }

  void chunk_mover::recalc_normals_around_selection()
  {
    if (_selection_size.first < 1 || _last_cursor_chunk.first < 0)
    {
      return;
    }

    // update normals around the paste area too
    int px = _last_cursor_chunk.first - (_selection_size.first / 2);
    int pz = _last_cursor_chunk.second - (_selection_size.second / 2);

    math::vector_3d orig(px * CHUNKSIZE + 5.f, 0.f, pz * CHUNKSIZE + 5.f);

    for (int x = -1; x < _selection_size.first + 1; ++x)
    {
      for (int z = -1; z < _selection_size.second + 1; ++z)
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
    if (_selection_size.first < 1 || _last_cursor_chunk.first < 0)
    {
      return;
    }

    int px = _last_cursor_chunk.first - (_selection_size.first / 2);
    int pz = _last_cursor_chunk.second - (_selection_size.second / 2);

    math::vector_3d orig(px * CHUNKSIZE + 5.f, 0.f, pz * CHUNKSIZE + 5.f);
    math::vector_3d ofs_left(-CHUNKSIZE, 0.f, 0.f);
    math::vector_3d ofs_up(0.f, 0.f, -CHUNKSIZE);

    for (int x = -1; x <= _selection_size.first; ++x)
    {
      for (int z = -1; z <= _selection_size.second; ++z)
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
