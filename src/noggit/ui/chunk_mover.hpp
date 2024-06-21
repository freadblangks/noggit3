// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/chunk_mover.hpp>
#include <noggit/bool_toggle_property.hpp>
#include <noggit/float_property.hpp>
#include <noggit/map_chunk_headers.hpp>

#include <QWidget>

namespace noggit::ui
{
  class chunk_mover_ui : public QWidget
  {
  public:
    chunk_mover_ui(noggit::chunk_mover* chunk_mover, QWidget* parent = nullptr);

    void change_height_offset(float change);
    void paste_selection();

    chunk_override_params override_params() const;

    float radius() const { return _radius.get(); }
    bool use_square_brush() const { return _square_brush.get(); }

    void change_radius(float v) { _radius.change(v); }
  private:
    noggit::chunk_mover* _chunk_mover;

    bool_toggle_property _override_height;
    bool_toggle_property _override_textures;
    bool_toggle_property _override_alphamaps;
    bool_toggle_property _override_liquids;
    bool_toggle_property _override_shadows;
    bool_toggle_property _override_area_id;
    bool_toggle_property _override_holes;
    bool_toggle_property _override_models;


    float_property _radius;
    bool_toggle_property _square_brush;
    bool_toggle_property _fix_gaps;
    bool_toggle_property _clear_shadows;
    bool_toggle_property _clear_models;
  };
}
