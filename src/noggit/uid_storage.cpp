// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/uid_storage.hpp>
#include <noggit/settings.hpp>

#include <boost/filesystem.hpp>

bool uid_storage::hasMaxUIDStored(uint32_t mapID)
{
  return NoggitSettings.uids->value(QString::number(mapID), -1).toUInt() != -1;
}

uint32_t uid_storage::getMaxUID(uint32_t mapID)
{
  return NoggitSettings.uids->value(QString::number(mapID), 0).toUInt();
}

void uid_storage::saveMaxUID(uint32_t mapID, uint32_t uid)
{
  NoggitSettings.uids->setValue(QString::number(mapID), uid);
}

void uid_storage::remove_uid_for_map(uint32_t map_id)
{
  NoggitSettings.uids->remove(QString::number(map_id));
}
