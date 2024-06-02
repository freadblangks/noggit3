// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <math/trig.hpp>

#include <string>
#include <vector>

namespace noggit
{
  struct bookmark
  {
    std::uint32_t map_id;
    std::uint32_t area_id;
    std::string name;
    math::vector_3d pos;
    float camera_yaw;
    float camera_pitch;
  };

  class bookmark_manager
  {
  public:
    void reload();
    bookmark const& add(math::vector_3d const& pos, math::degrees yaw, math::degrees pitch, std::uint32_t map, std::uint32_t area);
    std::vector<bookmark> const& bookmarks() const { return _bookmarks; }

    static bookmark_manager& instance()
    {
      static bookmark_manager inst;
      return inst;
    }
  private:
    bookmark_manager() {}

    std::vector<bookmark> _bookmarks;
  };
}
