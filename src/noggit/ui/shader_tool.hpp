// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_4d.hpp>
#include <noggit/float_property.hpp>
#include <noggit/ui/slider_spinbox.hpp>

#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

#include <qt-color-widgets/color_selector.hpp>
#include <qt-color-widgets/color_wheel.hpp>
#include <qt-color-widgets/hue_slider.hpp>
#include <qt-color-widgets/gradient_slider.hpp>
#include <qt-color-widgets/color_list_widget.hpp>


namespace noggit
{
  namespace ui
  {
    class shader_tool : public QWidget
    {
    public:
      shader_tool(math::vector_4d& color, QWidget* parent = nullptr);

      void changeShader (World*, math::vector_3d const& pos, float dt, bool add);
      void pickColor(World* world, math::vector_3d const& pos);
      void addColorToPalette();

      void change_radius(float change) { _radius_property.change(change); }
      void changeSpeed(float change) { _speed_property.change(change); }

      float brushRadius() const { return _radius_property.get(); }

      QSize sizeHint() const override;

    private:
      float_property _radius_property;
      float_property _speed_property;
      math::vector_4d& _color;

      slider_spinbox* _radius_widget;
      slider_spinbox* _speed_widget;
      QSpinBox* _spin_hue;
      QSpinBox* _spin_saturation;
      QSpinBox* _spin_value;

      color_widgets::ColorSelector* color_picker;
      color_widgets::ColorWheel* color_wheel;
      color_widgets::HueSlider* _slide_hue;
      color_widgets::GradientSlider* _slide_saturation;
      color_widgets::GradientSlider* _slide_value;
      color_widgets::ColorListWidget* _color_palette;

    public Q_SLOTS:
      void set_hsv();
      void update_color_widgets();

    };
  }
}
