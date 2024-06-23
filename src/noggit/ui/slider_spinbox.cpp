// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/slider_spinbox.hpp>

#include <QtWidgets/QFormLayout>

namespace noggit::ui
{
  slider_spinbox::slider_spinbox(QString label, float_property* prop, float min, float max, int decimal_count, QWidget* parent)
    : widget(parent)
    , _min(min)
    , _max(max)
    , _prop(prop)
  {
    auto layout(new QFormLayout(this));

    // sliders only do integers, so shifting the value allows
    // having the same precision as the double spinbox
    _slider_shift = 1;
    for (int i = 0; i < decimal_count; ++i)
    {
      _slider_shift *= 10;
    }

    _spin = new QDoubleSpinBox(this);
    _spin->setRange(min, max);
    _spin->setDecimals(decimal_count);
    _spin->setValue(_prop->get());
    layout->addRow(label, _spin);

    _slider = new QSlider(Qt::Orientation::Horizontal, this);
    _slider->setRange(static_cast<int>(std::round(min * _slider_shift)), static_cast<int>(std::round(max * _slider_shift)));
    _slider->setValue(static_cast<int>(std::round(_prop->get() * _slider_shift)));
    _slider->setValue(static_cast<int>(std::round(_prop->get() * _slider_shift)));
    _slider->setSingleStep(std::max(1, _slider_shift / 2));
    layout->addRow(_slider);

    layout->setContentsMargins(0, 0, 0, 0);

    connect ( _spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
            , [&] (double v)
              {
                QSignalBlocker const blocker(_slider);
                QSignalBlocker const blocker2(_prop);

                _prop->set(v);
                _slider->setSliderPosition (static_cast<int>(std::roundf(v * _slider_shift)));
              }
            );

    connect ( _slider, &QSlider::valueChanged
            , [&] (int v)
              {
                QSignalBlocker const blocker(_spin);
                QSignalBlocker const blocker2(_prop);

                float f = static_cast<float>(v) / _slider_shift;

                _prop->set(f);
                _spin->setValue(f);
              }
            );

    connect ( _prop, qOverload<float>(&float_property::changed)
            , [&] (float v)
              {
                QSignalBlocker const blocker(_spin);
                QSignalBlocker const blocker2(_slider);
                QSignalBlocker const blocker3(_prop);

                // required otherwise you can go out of range
                // when changing values with keybinds
                v = std::clamp(v, _min, _max);

                _prop->set(v);
                _spin->setValue(v);
                _slider->setSliderPosition(static_cast<int>(std::round(v * _slider_shift)));
              }
            );
  }
}
