// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/MPQ.h>
#include <noggit/alphamap.hpp>
#include <noggit/map_chunk_headers.hpp>
#include <noggit/MapHeaders.h>

#include <cstdint>
#include <array>

class Brush;
class MapTile;

struct tmp_edit_alpha_values
{
  using alpha_layer = std::array<float, 64 * 64>;
  // use 4 "alphamaps" for an easier editing
  std::array<alpha_layer, 4> map;

  alpha_layer& operator[](std::size_t i)
  {
    return map.at(i);
  }
};

class TextureSet
{
public:
  TextureSet() = delete;
  TextureSet( MapChunkHeader const& header
            , MPQFile* f
            , size_t base
            , MapTile* tile
            , bool use_big_alphamaps
            , bool do_not_fix_alpha_map
            , bool do_not_convert_alphamaps
            , chunk_shadow* shadow
            );

  void copy_data(noggit::chunk_data& data);
  void override_data(noggit::chunk_data& data, noggit::chunk_override_params const& params);

  math::vector_3d anim_param(int layer) const;

  int addTexture(scoped_blp_texture_reference texture);
  void eraseTexture(size_t id);
  void eraseTextures();
  // return true if at least 1 texture has been erased
  bool eraseUnusedTextures(float threshold = 0.01f);
  void swap_layers(int layer_1, int layer_2);
  void replace_texture(scoped_blp_texture_reference const& texture_to_replace, scoped_blp_texture_reference replacement_texture);
  bool paintTexture(float xbase, float zbase, float x, float z, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture);
  bool replace_texture( float xbase
                      , float zbase
                      , float x
                      , float z
                      , float radius
                      , scoped_blp_texture_reference const& texture_to_replace
                      , scoped_blp_texture_reference replacement_texture
                      );
  bool canPaintTexture(std::string const& texture);

  size_t const& num() const { return nTextures; }
  unsigned int flag(size_t id);
  unsigned int effect(size_t id);
  void setEffect(size_t id, int value);
  bool is_animated(std::size_t id) const;
  void change_texture_flag(scoped_blp_texture_reference const& tex, std::size_t flag, bool add);
  void clear_texture_flags();

  std::vector<std::vector<uint8_t>> save_alpha(bool big_alphamap);

  void convertToBigAlpha();
  void convertToOldAlpha();

  void merge_layers(size_t id1, size_t id2);
  bool removeDuplicate();

  std::vector<uint8_t> lod_texture_map();

  bool apply_alpha_changes();

  void create_temporary_alphamaps_if_needed();
  size_t nTextures;
  std::unique_ptr<tmp_edit_alpha_values> tmp_edit_values;

  void update_alpha_shadow_map_if_needed(int chunk_x, int chunk_y, chunk_shadow* shadow);

  std::string const& texture(int id) const { return _textures[id]; }

private:
  int get_texture_index_or_add (scoped_blp_texture_reference texture, float target);

  uint8_t sum_alpha(size_t offset) const;

  void alphas_to_big_alpha(uint8_t* dest);
  void alphas_to_old_alpha(uint8_t* dest);

  void update_lod_texture_map();

  std::vector<std::string> _textures;
  std::array<std::unique_ptr<Alphamap>, 3> alphamaps;

  // used on loading to prepare the alphamap data to upload in the async loader
  std::unique_ptr<std::array<std::uint8_t, 4 * 64 * 64>> _setup_amap_data;
  bool _first_amap_setup = false;

  bool _need_amap_update = true;

  std::vector<uint8_t> _lod_texture_map;
  bool _need_lod_texture_map_update = false;

  void require_update();

  ENTRY_MCLY _layers_info[4];

  bool _do_not_convert_alphamaps;

  static constexpr std::array<std::uint8_t, 256*256> make_alpha_lookup_array()
  {
    std::array<std::uint8_t, 256 * 256> array{};

    for (int i = 0; i < 256 * 256; ++i)
    {
      array[i] = i / 255 + (i % 255 <= 127 ? 0 : 1);
    }

    return array;
  }

  static std::array<std::uint8_t, 256 * 256> alpha_convertion_lookup;
};
