// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/bounding_box.hpp>
#include <noggit/Log.h>
#include <noggit/MapHeaders.h>
#include <noggit/Misc.h> // checkinside
#include <noggit/ModelInstance.h>
#include <noggit/WMO.h> // WMO
#include <noggit/WMOInstance.h>
#include <opengl/primitives.hpp>
#include <opengl/scoped.hpp>

WMOInstance::WMOInstance(std::string const& filename, ENTRY_MODF const* d)
  : wmo(filename)
  , pos(math::vector_3d(d->pos[0], d->pos[1], d->pos[2]))
  , dir(math::degrees(d->rot[0]), math::degrees(d->rot[1]), math::degrees(d->rot[2]))
  , mUniqueID(d->uniqueID), mFlags(d->flags)
  , mUnknown(d->unknown), mNameset(d->nameSet)
  , _doodadset(d->doodadSet)
{
  extents[0] = math::vector_3d(d->extents[0][0], d->extents[0][1], d->extents[0][2]);
  extents[1] = math::vector_3d(d->extents[1][0], d->extents[1][1], d->extents[1][2]);

  update_transform_matrix();
  change_doodadset(_doodadset);
}

WMOInstance::WMOInstance(std::string const& filename)
  : wmo(filename)
  , pos(math::vector_3d(0.0f, 0.0f, 0.0f))
  , dir(math::degrees::vec3(0_deg, 0_deg, 0_deg))
  , mUniqueID(0)
  , mFlags(0)
  , mUnknown(0)
  , mNameset(0)
  , _doodadset(0)
{
  change_doodadset(_doodadset);
}

bool WMOInstance::is_a_duplicate_of(WMOInstance const& other)
{
  return wmo->filename == other.wmo->filename
      && misc::vec3d_equals(pos, other.pos)
      && misc::deg_vec3d_equals(dir, other.dir);
}


void WMOInstance::update_transform_matrix()
{
  math::matrix_4x4 mat( math::matrix_4x4(math::matrix_4x4::translation, pos)
                      * math::matrix_4x4
                        ( math::matrix_4x4::rotation_yzx
                        , { -dir.z
                          , dir.y - 90_deg
                          , dir.x
                          }
                        )
                      );

  _transform_mat = mat;
  _transform_mat_inverted = mat.inverted();
  _transform_mat_transposed = mat.transposed();

  _need_doodadset_update = true;
}

void WMOInstance::intersect (math::ray const& ray, selection_result* results)
{
  if (!ray.intersect_bounds (extents[0], extents[1]))
  {
    return;
  }

  math::ray subray(_transform_mat_inverted, ray);

  for (auto&& result : wmo->intersect(subray))
  {
    results->emplace_back (result, selected_wmo_type (this));
  }
}

void WMOInstance::recalcExtents()
{
  // todo: keep track of whether the extents need to be recalculated or not
  // keep the old extents since they are saved in the adt
  if (wmo->loading_failed() || !wmo->finishedLoading())
  {
    _need_recalc_extents = true;
    return;
  }

  update_transform_matrix();
  update_doodads();

  std::vector<math::vector_3d> points;

  math::vector_3d wmo_min(misc::transform_model_box_coords(wmo->extents[0]));
  math::vector_3d wmo_max(misc::transform_model_box_coords(wmo->extents[1]));

  auto&& root_points = _transform_mat * math::aabb(wmo_min, wmo_max).all_corners();

  points.insert(points.end(), root_points.begin(), root_points.end());

  for (int i = 0; i < (int)wmo->groups.size(); ++i)
  {
    auto const& group = wmo->groups[i];
    auto&& group_points = _transform_mat
      * math::aabb( group.BoundingBoxMin // no need to use misc::transform_model_box_coords
                  , group.BoundingBoxMax // they are already in world coord (see group ctor)
                  ).all_corners();

    points.insert(points.end(), group_points.begin(), group_points.end());

    if (group.has_skybox())
    {
      math::aabb const group_aabb(group_points);

      group_extents[i] = {group_aabb.min, group_aabb.max};
    }
  }

  math::aabb const wmo_aabb(points);

  extents[0] = wmo_aabb.min;
  extents[1] = wmo_aabb.max;

  _aabb_center = (wmo_aabb.min + wmo_aabb.max) * 0.5f;
  _aabb_radius = (wmo_aabb.max - _aabb_center).length();

  wmo->require_transform_buffer_update();

  _need_recalc_extents = false;
}

bool WMOInstance::isInsideRect(math::vector_3d rect[2]) const
{
  return misc::rectOverlap(extents, rect);
}

void WMOInstance::change_doodadset(uint16_t doodad_set)
{
  if (!wmo->finishedLoading())
  {
    _need_doodadset_update = true;
    return;
  }

  // don't set an invalid doodad set
  if (doodad_set >= wmo->doodadsets.size())
  {
    return;
  }

  _doodadset = doodad_set;
  _doodads_per_group = wmo->doodads_per_group(_doodadset);
  _need_doodadset_update = false;
  _doodadset_loaded = true;

  update_doodads();
}

void WMOInstance::update_doodads()
{
  update_transform_matrix();

  if (!_doodadset_loaded)
  {
    // it will call update_doodads again so return after that
    change_doodadset(_doodadset);
    return;
  }

  bool still_need_update = false;

  for (auto& group_doodads : _doodads_per_group)
  {
    for (auto& doodad : group_doodads.second)
    {
      if (!doodad.update_transform_matrix_wmo(this))
      {
        still_need_update = true;
      }
    }
  }

  _need_doodadset_update = still_need_update;
}

void WMOInstance::resetDirection()
{
  dir = math::degrees::vec3(0.0_deg, dir.y, 0.0_deg);
  recalcExtents();
}

std::vector<wmo_doodad_instance*> WMOInstance::get_current_doodads()
{
  std::vector<wmo_doodad_instance*> doodads;

  if (!wmo->finishedLoading() || wmo->loading_failed())
  {
    return doodads;
  }

  if (_need_doodadset_update)
  {
    change_doodadset(_doodadset);
  }

  for (int i = 0; i < wmo->groups.size(); ++i)
  {
    doodads.reserve(doodads.size() + _doodads_per_group[i].size());

    for (auto& doodad : _doodads_per_group[i])
    {
      if (doodad.need_matrix_update())
      {
        doodad.update_transform_matrix_wmo(this);
      }

      doodads.push_back(&doodad);
    }
  }

  return doodads;
}

bool WMOInstance::is_visible(math::frustum const& frustum, float const& cull_distance, math::vector_3d const& camera, display_mode display)
{
  if (!frustum.intersectsSphere(_aabb_center, _aabb_radius))
  {
    return false;
  }

  float dist = display == display_mode::in_3D
    ? (_aabb_center - camera).length() - _aabb_radius
    : std::abs(_aabb_center.y - camera.y) - _aabb_radius;

  return (dist < cull_distance);
}

std::vector<wmo_doodad_instance*> WMOInstance::get_visible_doodads
  ( math::frustum const& frustum
  , float const& cull_distance
  , math::vector_3d const& camera
  , bool draw_hidden_models
  , display_mode display
  )
{
  std::vector<wmo_doodad_instance*> doodads;

  if (!wmo->finishedLoading() || wmo->loading_failed())
  {
    return doodads;
  }

  if (_need_doodadset_update)
  {
    change_doodadset(_doodadset);
  }

  if (!wmo->is_hidden() || draw_hidden_models)
  {
    for (int i = 0; i < wmo->groups.size(); ++i)
    {
      if (wmo->groups[i].visible)
      {
        doodads.reserve(doodads.size() + _doodads_per_group[i].size());

        for (auto& doodad : _doodads_per_group[i])
        {
          if (doodad.need_matrix_update())
          {
            doodad.update_transform_matrix_wmo(this);
          }

          doodads.push_back(&doodad);
        }
      }
    }
  }

  return doodads;
}

// todo: improve opengl::primitives to not recreate everything for each primitive
//       or store the selected wmos' bbox data in a buffer
void WMOInstance::draw_box_selected ( math::matrix_4x4 const& model_view
                                    , math::matrix_4x4 const& projection
                                    )
{
  opengl::primitives::wire_box(extents[0], extents[1])
    .draw ( model_view
          , projection
          , math::matrix_4x4(math::matrix_4x4::unit)
          , math::vector_4d(0.0f, 1.0f, 0.0f, 1.0f)
          );

  for (auto& group : wmo->groups)
  {
    opengl::primitives::wire_box(group.BoundingBoxMin, group.BoundingBoxMax)
      .draw ( model_view
            , projection
            , _transform_mat_transposed
            , {1.0f, 1.0f, 1.0f, 1.0f}
            );
  }

  opengl::primitives::wire_box ( math::vector_3d(wmo->extents[0].x, wmo->extents[0].z, -wmo->extents[0].y)
                               , math::vector_3d(wmo->extents[1].x, wmo->extents[1].z, -wmo->extents[1].y)
                               ).draw ( model_view
                                      , projection
                                      , _transform_mat_transposed
                                      , {1.0f, 0.0f, 0.0f, 1.0f}
                                      );
}
