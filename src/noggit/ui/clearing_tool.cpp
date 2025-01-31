// This file is part of Noggit3, licensed under GNU General Public License (version 3).


#include <noggit/ui/clearing_tool.hpp>
#include <noggit/ui/checkbox.hpp>
#include <noggit/ui/font_awesome.hpp>
#include <noggit/ui/slider_spinbox.hpp>
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
      , _texture_threshold(1.0f)
      , _clear_height(false)
      , _clear_textures(false)
      , _clear_duplicate_textures(false)
      , _clear_textures_under_threshold(false)
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
      clearing_option_layout->addWidget(new checkbox("Textures below Threshold", &_clear_textures_under_threshold, clearing_option_group));
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

      auto parameters_group(new QGroupBox("Parameters", this));
      auto parameters_layout(new QFormLayout(parameters_group));

      parameters_layout->addRow(new slider_spinbox("Radius", &_radius, 0.f, 1000.f, 2, parameters_group));
      parameters_layout->addRow(new slider_spinbox("Texture Alpha Threshold", &_texture_threshold, 0.f, 255.f, 0, parameters_group));

      layout->addRow(parameters_group);

      setMinimumWidth(sizeHint().width());
    }

    void clearing_tool::clear(World* world, math::vector_3d const& pos)
    {
      // Chunk
      if (_mode == 0)
      {
        world->clear_on_chunks(pos, _radius.get()
          , _clear_height.get()
          , _clear_textures.get()
          , _clear_duplicate_textures.get()
          , _clear_textures_under_threshold.get()
          , _texture_threshold.get()
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
        world->clear_on_tiles(pos, _radius.get()
          , _clear_height.get()
          , _clear_textures.get()
          , _clear_duplicate_textures.get()
          , _clear_textures_under_threshold.get()
          , _texture_threshold.get()
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

    QSize clearing_tool::sizeHint() const
    {
      return QSize(215, height());
    }

  }
}
