// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/widget.hpp>
#include <noggit/float_property.hpp>

#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>

namespace noggit::ui
{
  class slider_spinbox : public widget
  {
  public:
    slider_spinbox(QString label, float_property* prop, float min, float max, int decimal_count, QWidget* parent = nullptr);

  private:
    float _min;
    float _max;
    int _slider_shift;

    float_property* _prop;
    QDoubleSpinBox* _spin;
    QSlider* _slider;
  };
}
