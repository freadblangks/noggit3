// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <math/vector_3d.hpp>
#include <math/vector_4d.hpp>

#include <utility>

struct CImVector
{
  std::uint8_t b;
  std::uint8_t g;
  std::uint8_t r;
  std::uint8_t a;
};

struct CArgb
{
  std::uint8_t r;
  std::uint8_t g;
  std::uint8_t b;
  std::uint8_t a;
};

struct WMOMaterial
{
  union
  {
    uint32_t value;
    struct
    {
      uint32_t unlit : 1;
      uint32_t unfogged : 1;
      uint32_t unculled : 1;
      uint32_t ext_light : 1; // darkened used for the intern face of windows
      uint32_t sidn : 1;
      uint32_t window : 1; // lighting related(flag checked in CMapObj::UpdateSceneMaterials)
      uint32_t clamp_s : 1;
      uint32_t clamp_t : 1;
      uint32_t unused : 24;
    };
  } flags;
  uint32_t shader;
  uint32_t blend_mode; // Blending: 0 for opaque, 1 for transparent
  uint32_t texture_offset_1; // Start position for the first texture filename in the MOTX data block
  CImVector sidn_color; // emissive color
  CImVector frame_sidn_color; // runtime value
  uint32_t texture_offset_2; // Start position for the second texture filename in the MOTX data block
  CArgb diffuse_color;
  uint32_t ground_type;
  uint32_t texture_offset_3;
  uint32_t color_2;
  uint32_t flag_2;
  uint32_t runtime_data[2];
  // also runtime data
  uint32_t texture1; // this is the first texture object.
  uint32_t texture2; // this is the second texture object.
};

struct WMOLiquidHeader
{
  int32_t X, Y, A, B;
  math::vector_3d pos;
  int16_t material_id;
};

struct SMOWVert
{
  std::uint8_t flow1;
  std::uint8_t flow2;
  std::uint8_t flow1Pct;
  std::uint8_t filler;
};
struct SMOMVert
{
  std::int16_t s;
  std::int16_t t;
};

struct LiquidVertex
{
  union
  {
    SMOWVert water_vertex;
    SMOMVert magma_vertex;
  };
  float height;
};

struct SMOLTile
{
  uint8_t liquid : 6;
  uint8_t fishable : 1;
  uint8_t shared : 1;
};

struct wmo_batch
{
  int8_t unused[12];

  uint32_t index_start;
  uint16_t index_count;
  uint16_t vertex_start;
  uint16_t vertex_end;

  uint8_t flags;
  uint8_t texture;
};

union wmo_group_flags
{
  uint32_t value;
  struct
  {
    uint32_t has_bsp_tree : 1; // 0x1
    uint32_t has_light_map : 1; // 0x2
    uint32_t has_vertex_color : 1; // 0x4
    uint32_t exterior : 1; // 0x8
    uint32_t flag_0x10 : 1;
    uint32_t flag_0x20 : 1;
    uint32_t exterior_lit : 1; // 0x40
    uint32_t unreacheable : 1; // 0x80
    uint32_t flag_0x100: 1;
    uint32_t has_light : 1; // 0x200
    uint32_t flag_0x400 : 1;
    uint32_t has_doodads : 1; // 0x800
    uint32_t has_water : 1; // 0x1000
    uint32_t indoor : 1; // 0x2000
    uint32_t flag_0x4000 : 1;
    uint32_t flag_0x8000 : 1;
    uint32_t always_draw : 1; // 0x10000
    uint32_t has_mori_morb : 1; // 0x20000, cata+ only (?)
    uint32_t skybox : 1; // 0x40000
    uint32_t ocean : 1; // 0x80000
    uint32_t flag_0x100000 : 1;
    uint32_t mount_allowed : 1; // 0x200000
    uint32_t flag_0x400000 : 1;
    uint32_t flag_0x800000 : 1;
    uint32_t use_mocv2_for_texture_blending : 1; // 0x1000000
    uint32_t has_two_motv : 1; // 0x2000000
    uint32_t antiportal : 1; // 0x4000000
    uint32_t unk : 1; // 0x8000000 requires intBatchCount == 0, extBatchCount == 0, UNREACHABLE.
    uint32_t unused : 4;
  };
};
static_assert ( sizeof (wmo_group_flags) == sizeof (std::uint32_t)
              , "bitfields shall be implemented packed"
              );

struct wmo_group_header
{
  uint32_t group_name; // offset into MOGN
  uint32_t descriptive_group_name; // offset into MOGN
  wmo_group_flags flags;
  float box1[3];
  float box2[3];
  uint16_t portal_start;
  uint16_t portal_count;
  uint16_t transparency_batches_count;
  uint16_t interior_batch_count;
  uint16_t exterior_batch_count;
  uint16_t padding_or_batch_type_d; // probably padding, but might be data?
  uint8_t fogs[4];
  uint32_t group_liquid; // used for MLIQ
  uint32_t id;
  int32_t unk2, unk3;
};


struct WMOPV
{
  math::vector_3d a, b, c, d;
};

struct WMOPR
{
  int16_t portal, group, dir, reserved;
};

struct WMODoodadSet
{
  char name[0x14];
  int32_t start;
  int32_t size;
  int32_t unused;
};



union mohd_flags
{
  std::uint16_t flags;
  struct
  {
    std::uint16_t do_not_attenuate_vertices_based_on_distance_to_portal : 1;
    std::uint16_t use_unified_render_path : 1;
    std::uint16_t use_liquid_type_dbc_id : 1;
    std::uint16_t do_not_fix_vertex_color_alpha : 1;
    std::uint16_t unused : 12;
  };
};
static_assert ( sizeof (mohd_flags) == sizeof (std::uint16_t)
              , "bitfields shall be implemented packed"
              );


struct wmo_vertex
{
  math::vector_3d position;
  math::vector_3d normal;
  math::vector_4d color;
  math::vector_2d uv1;
  math::vector_2d uv2;
};
