// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Log.h>
#include <noggit/MPQ.h>
#include <opengl/texture.hpp>

#include <array>

class Alphamap
{
public:
  Alphamap();
  Alphamap(MPQFile* f, unsigned int flags, bool use_big_alphamaps, bool do_not_fix_alpha_map);

  void setAlpha(size_t offset, std::uint8_t value);
  void setAlpha(const std::uint8_t *pAmap);

  std::uint8_t getAlpha(size_t offset) const;
  const std::uint8_t *getAlpha();

  std::vector<uint8_t> compress() const;

private:
  void readCompressed(MPQFile *f);
  void readBigAlpha(MPQFile *f);
  void readNotCompressed(MPQFile *f, bool do_not_fix_alpha_map);

  void reset();

  std::array<std::uint8_t, 64 * 64> amap;
};
