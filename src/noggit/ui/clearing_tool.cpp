// This file is part of Noggit3, licensed under GNU General Public License (version 3).


#include <noggit/ui/clearing_tool.hpp>
#include <noggit/ui/checkbox.hpp>
#include <noggit/ui/font_awesome.hpp>
#include <noggit/World.h>

#include <util/qt/overload.hpp>

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>


#include <functional>

namespace noggit
{
  namespace ui
  {
    clearing_tool::clearing_tool(QWidget* parent)
      : QWidget(parent)
      , _radius(15.0f)
      , _clear_height(false)
      , _clear_textures(false)
      , _clear_duplicate_textures(false)
      , _clear_texture_flags(false)
      , _clear_liquids(false)
      , _clear_m2s(false)
      , _clear_wmos(false)
      , _clear_shadows(false)
      , _clear_mccv(false)
      , _clear_impassible_flag(false)
      , _clear_holes(false)
    {

      auto layout (new QFormLayout(this));

      auto clearing_option_group(new QGroupBox("Clear", this));
      auto clearing_option_layout(new QFormLayout(clearing_option_group));

      clearing_option_layout->addWidget(new checkbox("Height", &_clear_height, clearing_option_group));
      clearing_option_layout->addWidget(new checkbox("Textures", &_clear_textures, clearing_option_group));
      clearing_option_layout->addWidget(new checkbox("Texture Duplicates", &_clear_duplicate_textures, clearing_option_group));
      clearing_option_layout->addWidget(new checkbox("Liquids", &_clear_liquids, clearing_option_group));
      clearing_option_layout->addWidget(new checkbox("M2s", &_clear_m2s, clearing_option_group));
      clearing_option_layout->addWidget(new checkbox("WMOs", &_clear_wmos, clearing_option_group));
      clearing_option_layout->addWidget(new checkbox("Shadows", &_clear_shadows, clearing_option_group));
      clearing_option_layout->addWidget(new checkbox("Vertex Colors", &_clear_mccv, clearing_option_group));
      clearing_option_layout->addWidget(new checkbox("Impassible Flag", &_clear_impassible_flag, clearing_option_group));
      clearing_option_layout->addWidget(new checkbox("Holes", &_clear_holes, clearing_option_group));

      layout->addRow(clearing_option_group);

      auto mode_option_group(new QGroupBox("Mode", this));
      auto mode_option_layout(new QFormLayout(mode_option_group));


      auto chunk_button(new QRadioButton("Chunk", this));
      auto adt_button(new QRadioButton("Adt", this));


      QButtonGroup* mode_button_group = new QButtonGroup(this);
      mode_button_group->addButton(chunk_button, 0);
      mode_button_group->addButton(adt_button, 1);

      connect ( mode_button_group, qOverload<int>(&QButtonGroup::buttonClicked)
              , [&](int id) { _mode = id; }
              );

      mode_option_layout->addRow(chunk_button);
      mode_option_layout->addRow(adt_button);

      mode_button_group->button(_mode)->setChecked(true);

      layout->addRow(mode_option_group);

      _radius_spin = new QDoubleSpinBox (this);
      _radius_spin->setRange (0.0f, 1000.0f);
      _radius_spin->setDecimals (2);
      _radius_spin->setValue (_radius);

      layout->addRow ("Radius:", _radius_spin);

      _radius_slider = new QSlider (Qt::Orientation::Horizontal, this);
      _radius_slider->setRange (0, 1000);
      _radius_slider->setSliderPosition (_radius);

      layout->addRow (_radius_slider);



      connect ( _radius_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  _radius = v;
                  QSignalBlocker const blocker(_radius_slider);
                  _radius_slider->setSliderPosition ((int)std::round (v));
                }
              );

      connect ( _radius_slider, &QSlider::valueChanged
              , [&] (int v)
                {
                  _radius = v;
                  QSignalBlocker const blocker(_radius_spin);
                  _radius_spin->setValue(v);
                }
              );

      setMinimumWidth(sizeHint().width());

    }

    void clearing_tool::clear(World* world, math::vector_3d const& pos)
    {
      // Chunk
      if (_mode == 0)
      {
        world->clear_on_chunks(pos, _radius
          , _clear_height.get()
          , _clear_textures.get()
          , _clear_duplicate_textures.get()
          , _clear_texture_flags.get()
          , _clear_liquids.get()
          , _clear_m2s.get()
          , _clear_wmos.get()
          , _clear_shadows.get()
          , _clear_mccv.get()
          , _clear_impassible_flag.get()
          , _clear_holes.get()
          );
      }
      // Adt
      else if (_mode == 1)
      {
        world->clear_on_tiles(pos, _radius
          , _clear_height.get()
          , _clear_textures.get()
          , _clear_duplicate_textures.get()
          , _clear_texture_flags.get()
          , _clear_liquids.get()
          , _clear_m2s.get()
          , _clear_wmos.get()
          , _clear_shadows.get()
          , _clear_mccv.get()
          , _clear_impassible_flag.get()
          , _clear_holes.get()
          );
      }
    }

    void clearing_tool::change_radius(float change)
    {
      _radius_spin->setValue(_radius + change);
    }

    QSize clearing_tool::sizeHint() const
    {
      return QSize(215, height());
    }

  }
}
