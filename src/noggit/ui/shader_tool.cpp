// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/World.h>
#include <noggit/ui/shader_tool.hpp>
#include <util/qt/overload.hpp>
#include <noggit/ui/font_awesome.hpp>

#include <qt-color-widgets/color_selector.hpp>
#include <qt-color-widgets/color_wheel.hpp>
#include <qt-color-widgets/hue_slider.hpp>
#include <qt-color-widgets/gradient_slider.hpp>
#include <qt-color-widgets/color_list_widget.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>


#include <functional>

namespace noggit
{
  namespace ui
  {
    shader_tool::shader_tool(math::vector_4d& color, QWidget* parent)
      : QWidget(parent)
      , _radius_property(15.f)
      , _speed_property(1.0f)
      , _color(color)
    {

      auto layout (new QFormLayout(this));

      _radius_widget = new slider_spinbox("Radius:", &_radius_property, 0.f, 100.f, 2, this);
      layout->addRow (_radius_widget);

      _radius_widget = new slider_spinbox("Speed:", &_speed_property, 0.f, 10.f, 2, this);
      layout->addRow(_radius_widget);

      color_picker = new color_widgets::ColorSelector (this);
      color_picker->setDisplayMode (color_widgets::ColorSelector::NoAlpha);
      color_picker->setColor (QColor::fromRgbF (color.x, color.y, color.z, color.w));
      color_picker->setMinimumHeight(25);

      layout->addRow("Color:", color_picker);

      color_wheel = new color_widgets::ColorWheel(this);
      color_wheel->setColor (QColor::fromRgbF (color.x, color.y, color.z, color.w));
      color_wheel->setMinimumSize(QSize(200, 200));
      layout->addRow(color_wheel);

      _spin_hue = new QSpinBox(this);
      _spin_hue->setRange(0, 359);
      layout->addRow("Hue:", _spin_hue);

      _slide_hue = new color_widgets::HueSlider(this);
      layout->addRow(_slide_hue);

      _spin_saturation = new QSpinBox(this);
      _spin_saturation->setRange(0, 255);
      layout->addRow("Saturation:", _spin_saturation);

      _slide_saturation = new color_widgets::GradientSlider(this);
      _slide_saturation->setRange(0, 255);
      layout->addRow(_slide_saturation);


      _spin_value = new QSpinBox(this);
      _spin_value->setRange(0, 255);
      layout->addRow("Value:", _spin_value);

      _slide_value = new color_widgets::GradientSlider(this);
      _slide_value->setRange(0, 255);
      layout->addRow(_slide_value);

      _color_palette = new color_widgets::ColorListWidget(this);
      _color_palette->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
      layout->addRow(_color_palette);

      auto info_label (new QLabel("drag&drop colors to select them", this));
      info_label->setAlignment(Qt::AlignCenter);

      layout->addRow(info_label);

      QObject::connect(_slide_saturation, &color_widgets::GradientSlider::valueChanged, this, &shader_tool::set_hsv);
      QObject::connect(_slide_value, &color_widgets::GradientSlider::valueChanged, this, &shader_tool::set_hsv);
      QObject::connect(_slide_hue, &color_widgets::HueSlider::valueChanged, this, &shader_tool::set_hsv);

      QObject::connect(color_wheel, &color_widgets::ColorWheel::colorSelected, this, &shader_tool::update_color_widgets);
      QObject::connect(color_picker, &color_widgets::ColorSelector::colorChanged, this, &shader_tool::update_color_widgets);

      QObject::connect(_slide_saturation, SIGNAL(valueChanged(int)), _spin_saturation, SLOT(setValue(int)));
      QObject::connect(_slide_value, SIGNAL(valueChanged(int)), _spin_value, SLOT(setValue(int)));
      QObject::connect(_slide_hue, SIGNAL(valueChanged(int)), _spin_hue, SLOT(setValue(int)));

      QObject::connect(_spin_saturation, SIGNAL(valueChanged(int)), _slide_saturation, SLOT(setValue(int)));
      QObject::connect(_spin_hue, SIGNAL(valueChanged(int)), _slide_hue, SLOT(setValue(int)));
      QObject::connect(_spin_value, SIGNAL(valueChanged(int)), _slide_value, SLOT(setValue(int)));


      connect ( _color_palette, &color_widgets::ColorListWidget::color_added
              , [&] ()
                {
                  _color_palette->setColorAt(_color_palette->colors().length() - 1, color_wheel->color());
                }
              );


      connect ( color_picker, &color_widgets::ColorSelector::colorChanged
              , [this] (QColor new_color)
                {
                  QSignalBlocker const blocker (color_wheel);
                  color_wheel->setColor(new_color);
                  _color.x = new_color.redF();
                  _color.y = new_color.greenF();
                  _color.z = new_color.blueF();
                  _color.w = 1.0f;
                }
              );

      connect (color_wheel, &color_widgets::ColorWheel::colorChanged, color_picker, &color_widgets::ColorSelector::setColor);

      setMinimumWidth(sizeHint().width());

    }

    void shader_tool::changeShader
      (World* world, math::vector_3d const& pos, float dt, bool add)
    {
      world->changeShader (pos, _color, 2.0f*dt*_speed_property.get(), _radius_property.get(), add);
    }

    void shader_tool::pickColor(World* world, math::vector_3d const& pos)
    {
      math::vector_3d color = world->pickShaderColor(pos);

      QColor new_color;
      new_color.setRgbF(color.x * 0.5, color.y * 0.5, color.z * 0.5);
      color_wheel->setColor(new_color);

    }

    void shader_tool::addColorToPalette()
    {
      _color_palette->append();
    }

    void shader_tool::set_hsv()
    {
      if (!signalsBlocked())
      {
        color_wheel->setColor(QColor::fromHsv(
          _slide_hue->value(),
          _slide_saturation->value(),
          _slide_value->value()
        ));
        update_color_widgets();
      }
    }

    void shader_tool::update_color_widgets()
    {
      bool blocked = signalsBlocked();
      blockSignals(true);
      Q_FOREACH(QWidget * w, findChildren<QWidget*>())
        w->blockSignals(true);

      _slide_hue->setValue(qRound(color_wheel->hue() * 360.0));
      _slide_hue->setColorSaturation(color_wheel->saturation());
      _slide_hue->setColorValue(color_wheel->value());
      _spin_hue->setValue(_slide_hue->value());

      _slide_saturation->setValue(qRound(color_wheel->saturation() * 255.0));
      _spin_saturation->setValue(_slide_saturation->value());
      _slide_saturation->setFirstColor(QColor::fromHsvF(color_wheel->hue(), 0, color_wheel->value()));
      _slide_saturation->setLastColor(QColor::fromHsvF(color_wheel->hue(), 1, color_wheel->value()));

      _slide_value->setValue(qRound(color_wheel->value() * 255.0));
      _spin_value->setValue(_slide_value->value());
      _slide_value->setFirstColor(QColor::fromHsvF(color_wheel->hue(), color_wheel->saturation(), 0));
      _slide_value->setLastColor(QColor::fromHsvF(color_wheel->hue(), color_wheel->saturation(), 1));


      blockSignals(blocked);
      for (QWidget* w : findChildren<QWidget*>())
        w->blockSignals(false);


    }

    QSize shader_tool::sizeHint() const
    {
      return QSize(215, height());
    }

  }
}
