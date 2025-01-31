// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/alphamap.hpp>
#include <opengl/context.hpp>

#include <boost/optional/optional.hpp>

Alphamap::Alphamap()
{
  reset();
}

Alphamap::Alphamap(MPQFile *f, unsigned int flags, bool use_big_alphamaps, bool do_not_fix_alpha_map)
{
  reset();

  if (use_big_alphamaps)
  {
    // can only compress big alpha
    if (flags & 0x200)
    {
      readCompressed(f);
    }
    else
    {
      readBigAlpha(f);
    }
  }
  else
  {
    readNotCompressed(f, do_not_fix_alpha_map);
  }
}

namespace
{
  struct compressed_mcal_entry
  {
    enum mode_t
    {
      copy = 0,              // append value[0..count - 1]
      fill = 1,              // append value[0] count times
    };
    uint8_t count : 7;
    uint8_t mode : 1;

    uint8_t value[];
  };
}

void Alphamap::readCompressed(MPQFile *f)
{
  // compressed
  char const* input = f->getPointer();

  for (std::size_t offset_output(0); offset_output < 4096;)
  {
    compressed_mcal_entry const* e = reinterpret_cast<compressed_mcal_entry const*>(input);

    int count = e->count;

    if (offset_output + count > 4096)
    {
      LogError << "Invalid MCAL, uncompressed size is greater than 4096" << std::endl;
      count = 4096 - offset_output;
    }

    ++input;

    if (count == 0)
    {
      continue;
    }

    if (e->mode == compressed_mcal_entry::fill)
    {
      memset(&amap[offset_output], e->value[0], count);
      ++input;
    }
    else
    {
      memcpy(&amap[offset_output], e->value, count);
      input += count;
    }

    offset_output += count;
  }
}

void Alphamap::readBigAlpha(MPQFile *f)
{
  memcpy(amap.data(), f->getPointer(), 64 * 64);
  f->seekRelative(0x1000);
}

namespace
{
  struct alpha_4_4
  {
    std::uint8_t lower : 4;
    std::uint8_t upper : 4;
  };
}

void Alphamap::readNotCompressed(MPQFile *f, bool do_not_fix_alpha_map)
{
  alpha_4_4 const* abuf = reinterpret_cast<alpha_4_4 const*>(f->getPointer());

  for (std::size_t x(0); x < 64; ++x)
  {
    for (std::size_t y(0); y < 64; y += 2)
    {
      amap[x * 64 + y + 0] = abuf->lower | abuf->lower << 4;
      amap[x * 64 + y + 1] = abuf->upper | abuf->upper << 4;
      ++abuf;
    }
  }

  if (!do_not_fix_alpha_map)
  {
    for (std::size_t i(0); i < 64; ++i)
    {
      amap[i * 64 + 63] = amap[i * 64 + 62];
      amap[63 * 64 + i] = amap[62 * 64 + i];
    }
    amap[63 * 64 + 63] = amap[62 * 64 + 62];
  }
  f->seekRelative(0x800);
}

void Alphamap::reset()
{
  amap.fill(0);
}

void Alphamap::setAlpha(size_t offset, std::uint8_t value)
{
  amap[offset] = value;
}

void Alphamap::setAlpha(const std::uint8_t *pAmap)
{
  memcpy(amap.data(), pAmap, 64 * 64);
}

std::uint8_t Alphamap::getAlpha(size_t offset) const
{
  return amap[offset];
}

const std::uint8_t *Alphamap::getAlpha()
{
  return amap.data();
}

std::vector<uint8_t> Alphamap::compress() const
{
  std::vector<uint8_t> data(amap.data(), amap.data() + 4096);
  auto current (data.begin());
  auto const end (data.end());
  int column_pos = 0;

  auto const consume_fill
  (
    [&]
    {
      int8_t count (0);
      column_pos %= 64;

      while ((current + 1 < end) && *current == *(current + 1) && column_pos < 63)
      {
        ++current;
        ++count;
        ++column_pos;
      }

      // include current (current is incremented in the for loop)
      if (count)
      {
        ++count;
        ++column_pos;
      }

      return count;
    }
  );

  std::vector<uint8_t> result;
  boost::optional<std::size_t> current_copy_entry_offset (boost::none);
  auto const current_copy_entry
  (
    [&]
    {
      return reinterpret_cast<compressed_mcal_entry*> (&*(result.begin() + *current_copy_entry_offset));
    }
  );

  for (; current != end; ++current)
  {
    auto const fill (consume_fill());
    if (fill)
    {
      current_copy_entry_offset = boost::none;

      result.emplace_back();
      result.emplace_back(*current);

      compressed_mcal_entry* e (reinterpret_cast<compressed_mcal_entry*> (&*(result.rbegin() + 1)));
      e->mode = compressed_mcal_entry::fill;
      e->count = fill;

      column_pos %= 64;
    }
    else
    {
      if ( current_copy_entry_offset == boost::none
          || column_pos == 64
          )
      {
        current_copy_entry_offset = result.size();
        result.emplace_back();
        result.emplace_back(*current);
        current_copy_entry()->mode = compressed_mcal_entry::copy;
        current_copy_entry()->count = 1;

        column_pos %= 64;
      }
      else
      {
        result.emplace_back(*current);
        current_copy_entry()->count++;
      }

      column_pos++;
    }
  }

  return result;
}
