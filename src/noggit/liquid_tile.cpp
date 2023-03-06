// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/liquid_chunk.hpp>
#include <noggit/Log.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/liquid_tile.hpp>

liquid_tile::liquid_tile(MapTile *pTile, float pXbase, float pZbase, bool use_mclq_green_lava)
  : tile(pTile)
  , xbase(pXbase)
  , zbase(pZbase)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x] = std::make_unique<liquid_chunk> (xbase + CHUNKSIZE * x, zbase + CHUNKSIZE * z, use_mclq_green_lava);
    }
  }
}

void liquid_tile::readFromFile(MPQFile &theFile, size_t basePos)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      theFile.seek(basePos + (z * 16 + x) * sizeof(MH2O_Header));
      chunks[z][x]->fromFile(theFile, basePos);
    }
  }
}

void liquid_tile::draw ( math::frustum const& frustum
                     , const float& cull_distance
                     , const math::vector_3d& camera
                     , bool camera_moved
                     , liquid_render& render
                     , opengl::scoped::use_program& water_shader
                     , int animtime
                     , int layer
                     , display_mode display
                     )
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->draw ( frustum
                         , cull_distance
                         , camera
                         , camera_moved
                         , render
                         , water_shader
                         , animtime
                         , layer
                         , display
                         );
    }
  }
}

liquid_chunk* liquid_tile::getChunk(int x, int z)
{
  return chunks[z][x].get();
}

void liquid_tile::autoGen(float factor)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->autoGen(tile->getChunk(x, z), factor);
    }
  }
}

void liquid_tile::saveToFile(util::sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition)
{
  if (!hasData(0))
  {
    return;
  }

  int ofsW = lCurrentPosition + 0x8; //water Header pos

  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mh2o = lCurrentPosition - 0x14; //setting offset to MH2O data in Header

  int headers_size = 256 * sizeof(MH2O_Header);
  // 8 empty bytes for the chunk header
  lADTFile.Extend(8 + headers_size);
  // set current pos after the mh2o headers
  lCurrentPosition = ofsW + headers_size;
  int header_pos = ofsW;


  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->save(lADTFile, ofsW, header_pos, lCurrentPosition);
    }
  }

  SetChunkHeader(lADTFile, ofsW - 8, 'MH2O', lCurrentPosition - ofsW);
}

bool liquid_tile::hasData(size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      if (chunks[z][x]->hasData(layer))
      {
        return true;
      }
    }
  }

  return false;
}

void liquid_tile::CropMiniChunk(int x, int z, MapChunk* chunkTerrain)
{
  chunks[z][x]->CropWater(chunkTerrain);
}

void liquid_tile::setType(int type, size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->setType(type, layer);
    }
  }
}

int liquid_tile::getType(size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      if (chunks[z][x]->hasData(layer))
      {
        return chunks[z][x]->getType(layer);
      }
    }
  }
  return 0;
}
