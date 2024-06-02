// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/bookmarks.hpp>

#include <noggit/DBC.h>
#include <noggit/DBCFile.h>
#include <noggit/Log.h>

#include <fstream>
#include <sstream>

namespace noggit
{
  void bookmark_manager::reload()
  {
    _bookmarks.clear();

    std::ifstream f("bookmarks.txt");
    if (!f.is_open())
    {
      LogDebug << "No bookmarks file." << std::endl;
      return;
    }

    bookmark b;

    while (f >> b.map_id >> b.pos.x >> b.pos.y >> b.pos.z >> b.camera_yaw >> b.camera_pitch >> b.area_id)
    {
      if (b.map_id == -1)
      {
        continue;
      }

      std::stringstream name;
      name << MapDB::getMapName(b.map_id) << ": " << AreaDB::getAreaName(b.area_id);
      b.name = name.str();

      _bookmarks.push_back(b);
    }

    f.close();
  }

  bookmark const& bookmark_manager::add(math::vector_3d const& pos, math::degrees yaw, math::degrees pitch, std::uint32_t map, std::uint32_t area)
  {
    bookmark b;

    b.map_id = map;
    b.area_id = area;
    b.pos = pos;
    b.camera_yaw = yaw._;
    b.camera_pitch = pitch._;

    std::stringstream name;
    name << MapDB::getMapName(b.map_id) << ": " << AreaDB::getAreaName(b.area_id);
    b.name = name.str();

    _bookmarks.push_back(b);

    std::ofstream f ("bookmarks.txt", std::ios_base::app);
    f << map << " "
      << pos.x << " "
      << pos.y << " "
      << pos.z << " "
      << yaw._ << " "
      << pitch._ << " "
      << area << std::endl;

    return *_bookmarks.rbegin();
  }
}
