// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/ray.hpp>
#include <math/vector_3d.hpp> // math::vector_3d
#include <noggit/WMO.h>

#include <cstdint>
#include <set>

class MPQFile;
struct ENTRY_MODF;

class WMOInstance
{
public:
  scoped_wmo_reference wmo;
  math::vector_3d pos;
  math::vector_3d  extents[2];
  std::map<int, std::pair<math::vector_3d, math::vector_3d>> group_extents;
  math::degrees::vec3  dir;
  unsigned int mUniqueID;
  uint16_t mFlags;
  uint16_t mUnknown;
  uint16_t mNameset;

  bool need_doodads_update() const { return _need_doodadset_update; }
  bool need_recalc_extents() const { return _need_recalc_extents; }
  void require_extents_recalc() { _need_recalc_extents = true; }

  uint16_t doodadset() const { return _doodadset; }
  void change_doodadset(uint16_t doodad_set);
  void update_doodads();

private:
  uint16_t _doodadset;

  std::map<uint32_t, std::vector<wmo_doodad_instance>> _doodads_per_group;
  bool _doodadset_loaded = false;
  bool _need_doodadset_update = true;
  bool _need_recalc_extents = true;

  math::matrix_4x4 _transform_mat = math::matrix_4x4::uninitialized;
  math::matrix_4x4 _transform_mat_inverted = math::matrix_4x4::uninitialized;
  math::matrix_4x4 _transform_mat_transposed = math::matrix_4x4::uninitialized;

  float _aabb_radius;
  math::vector_3d _aabb_center;

public:
  WMOInstance(std::string const& filename, ENTRY_MODF const* d);
  explicit WMOInstance(std::string const& filename);

  WMOInstance(WMOInstance const& other) = default;
  WMOInstance& operator=(WMOInstance const& other) = default;

  WMOInstance (WMOInstance&& other)
    : wmo (std::move (other.wmo))
    , pos (other.pos)
    , group_extents(other.group_extents)
    , dir (other.dir)
    , mUniqueID (other.mUniqueID)
    , mFlags (other.mFlags)
    , mUnknown (other.mUnknown)
    , mNameset (other.mNameset)
    , _doodadset (other._doodadset)
    , _doodads_per_group(other._doodads_per_group)
    , _need_doodadset_update(other._need_doodadset_update)
    , _transform_mat(other._transform_mat)
    , _transform_mat_inverted(other._transform_mat_inverted)
    , _transform_mat_transposed(other._transform_mat_transposed)
    , _aabb_radius(other._aabb_radius)
    , _aabb_center(other._aabb_center)
  {
    std::swap (extents, other.extents);
  }

  WMOInstance& operator= (WMOInstance&& other)
  {
    std::swap(wmo, other.wmo);
    std::swap(pos, other.pos);
    std::swap(extents, other.extents);
    std::swap(group_extents, other.group_extents);
    std::swap(dir, other.dir);
    std::swap(mUniqueID, other.mUniqueID);
    std::swap(mFlags, other.mFlags);
    std::swap(mUnknown, other.mUnknown);
    std::swap(mNameset, other.mNameset);
    std::swap(_doodadset, other._doodadset);
    std::swap(_doodads_per_group, other._doodads_per_group);
    std::swap(_need_doodadset_update, other._need_doodadset_update);
    std::swap(_transform_mat, other._transform_mat);
    std::swap(_transform_mat_inverted, other._transform_mat_inverted);
    std::swap(_transform_mat_transposed, other._transform_mat_transposed);
    std::swap(_aabb_radius, other._aabb_radius);
    std::swap(_aabb_center, other._aabb_center);
    return *this;
  }
  /*
  bool operator==(WMOInstance&& other) const
  {
      return this->mUniqueID == other.mUniqueID;
  }*/

  bool is_a_duplicate_of(WMOInstance const& other);

  void update_transform_matrix();

  math::matrix_4x4 transform_matrix() const { return _transform_mat; }
  math::matrix_4x4 transform_matrix_inverted() const { return _transform_mat_inverted; }
  math::matrix_4x4 transform_matrix_transposed() const { return _transform_mat_transposed; }

  void intersect (math::ray const&, selection_result*);

  void recalcExtents();
  void resetDirection();

  bool isInsideRect(math::vector_3d rect[2]) const;

  std::vector<wmo_doodad_instance*> get_current_doodads();

  bool is_visible(math::frustum const& frustum, float const& cull_distance, math::vector_3d const& camera, display_mode display);

  std::vector<wmo_doodad_instance*> get_visible_doodads( math::frustum const& frustum
                                                       , float const& cull_distance
                                                       , math::vector_3d const& camera
                                                       , bool draw_hidden_models
                                                       , display_mode display
                                                       );

    void draw_box_selected ( math::matrix_4x4 const& model_view
                           , math::matrix_4x4 const& projection
                           );
};
