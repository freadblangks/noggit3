// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/float_property.hpp>
#include <util/qt/overload.hpp>

#include <QtWidgets/QDoubleSpinBox>

namespace noggit
{
  namespace ui
  {
    class double_spinbox : public QDoubleSpinBox
    {
    public:
      double_spinbox( float_property* prop
               , QWidget* parent = nullptr
               )
        : QDoubleSpinBox(parent)
      {
        connect ( this, qOverload<double>(&QDoubleSpinBox::valueChanged)
                , prop, &float_property::set
                );
        connect ( prop, &float_property::changed
                , this, &QDoubleSpinBox::setValue
                );

        setValue(prop->get());
      }
    };
  }
}
