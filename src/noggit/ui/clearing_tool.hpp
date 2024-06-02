// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/bool_toggle_property.hpp>
#include <math/vector_3d.hpp>

#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>


class World;

namespace noggit
{
  namespace ui
  {
    class clearing_tool : public QWidget
    {
    public:
      clearing_tool(QWidget* parent = nullptr);

      void clear(World* world, math::vector_3d const& pos);

      void change_radius(float change);
      float radius() const { return _radius; }

      QSize sizeHint() const override;

    private:
      float _radius;
      float _texture_threshold;
      int _mode = 0;

      bool_toggle_property _clear_height;
      bool_toggle_property _clear_textures;
      bool_toggle_property _clear_duplicate_textures;
      bool_toggle_property _clear_textures_under_threshold;
      bool_toggle_property _clear_texture_flags;
      bool_toggle_property _clear_liquids;
      bool_toggle_property _clear_m2s;
      bool_toggle_property _clear_wmos;
      bool_toggle_property _clear_shadows;
      bool_toggle_property _clear_mccv;
      bool_toggle_property _clear_impassible_flag;
      bool_toggle_property _clear_holes;

      QSlider* _radius_slider;
      QDoubleSpinBox* _radius_spin;
      QSlider* _texture_threshold_slider;
      QDoubleSpinBox* _texture_threshold_spin;

    };
  }
}
