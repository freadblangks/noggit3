// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>

#include <boost/variant.hpp>

#include <string>
#include <vector>

class ModelInstance;
class WMOInstance;
class MapChunk;
class liquid_layer;

struct selected_chunk_type
{
  selected_chunk_type(MapChunk* _chunk, std::tuple<int, int, int> _triangle, math::vector_3d _position)
    : chunk(_chunk)
    , triangle(_triangle)
    , position(_position)
  {}

  MapChunk* chunk;
  std::tuple<int,int,int> triangle; // mVertices[i] points of the hit triangle
  math::vector_3d position;

  bool operator== (selected_chunk_type const& other) const
  {
    return chunk == other.chunk;
  }
};

struct selected_liquid_layer_type
{
  selected_liquid_layer_type(liquid_layer* layer, std::tuple<int, int, int> triangle, math::vector_3d position, int liquid_id)
    : layer(layer)
    , triangle(triangle)
    , position(position)
    , liquid_id(liquid_id)
  {

  }

  bool operator== (selected_liquid_layer_type const& other) const
  {
    return layer == other.layer;
  }

  liquid_layer* layer;
  std::tuple<int, int, int> triangle;
  math::vector_3d position;
  int liquid_id;
};

using selected_model_type = ModelInstance*;
using selected_wmo_type = WMOInstance*;
using selection_type = boost::variant < selected_model_type
                                      , selected_wmo_type
                                      , selected_chunk_type
                                      , selected_liquid_layer_type
                                      >;
//! \note Keep in same order as variant!
enum eSelectionEntryTypes
{
  eEntry_Model,
  eEntry_WMO,
  eEntry_MapChunk,
  eEntry_LiquidLayer
};

using selection_entry = std::pair<float, selection_type>;
using selection_result = std::vector<selection_entry>;
