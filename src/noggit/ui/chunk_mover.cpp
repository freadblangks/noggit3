// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/chunk_mover.hpp>
#include <noggit/ui/checkbox.hpp>
#include <noggit/ui/double_spinbox.hpp>
#include <noggit/ui/slider_spinbox.hpp>

#include <noggit/map_chunk_headers.hpp>

#include <QFormLayout>
#include <QGroupBox>

namespace noggit::ui
{
  chunk_mover_ui::chunk_mover_ui(noggit::chunk_mover* chunk_mover, QWidget* parent)
    : QWidget(parent)
    , _chunk_mover(chunk_mover)
    , _override_height(true)
    , _override_textures(true)
    , _override_alphamaps(true)
    , _override_liquids(true)
    , _override_shadows(false)
    , _override_area_id(true)
    , _override_holes(true)
    , _override_models(true)
    , _fix_gaps(true)
    , _clear_shadows(true)
    , _clear_models(true)
    , _radius(15.f)
    , _square_brush(true)
  {
    auto layout(new QFormLayout(this));

    QGroupBox* overrides_group = new QGroupBox("Override", this);
    auto overrides_layout(new QFormLayout(overrides_group));

    overrides_layout->addRow(new checkbox("Height", &_override_height, overrides_group));
    overrides_layout->addRow(new checkbox("Textures", &_override_textures, overrides_group));
    overrides_layout->addRow(new checkbox("Alphamaps", &_override_alphamaps, overrides_group));
    overrides_layout->addRow(new checkbox("Liquids", &_override_liquids, overrides_group));
    overrides_layout->addRow(new checkbox("Shadows", &_override_shadows, overrides_group));
    overrides_layout->addRow(new checkbox("Area ID", &_override_area_id, overrides_group));
    overrides_layout->addRow(new checkbox("Holes", &_override_holes, overrides_group));
    overrides_layout->addRow(new checkbox("Models", &_override_models, overrides_group));

    layout->addRow(overrides_group);

    QGroupBox* param_group = new QGroupBox("Parameters", this);
    auto param_layout(new QFormLayout(param_group));

    param_layout->addRow(new slider_spinbox("Radius", &_radius, 1.f, 500.f, 0, this));
    param_layout->addRow(new checkbox("Square Brush", &_square_brush, param_group));

    auto spinbox(new double_spinbox(&chunk_mover->height_offset_property(), this));
    spinbox->setDecimals(3);
    spinbox->setRange(-20000.f, 20000.f);
    param_layout->addRow("Height Offset:", spinbox);

    param_layout->addRow(new checkbox("Fix Gaps", &_fix_gaps, param_group));
    param_layout->addRow(new checkbox("Clear Shadows", &_clear_shadows, param_group));
    param_layout->addRow(new checkbox("Clear Models", &_clear_models, param_group));

    layout->addRow(param_group);
  }

  void chunk_mover_ui::change_height_offset(float change)
  {
    _chunk_mover->height_offset_property().change(change);
  }

  void chunk_mover_ui::paste_selection()
  {
    _chunk_mover->apply(false);
  }

  chunk_override_params chunk_mover_ui::override_params() const
  {
    chunk_override_params params;
    params.height = _override_height.get();
    params.textures = _override_textures.get();
    params.alphamaps = _override_alphamaps.get();
    params.liquids = _override_liquids.get();
    params.shadows = _override_shadows.get();
    params.area_id = _override_area_id.get();
    params.holes = _override_holes.get();
    params.models = _override_models.get();

    params.fix_gaps = _fix_gaps.get();
    params.clear_shadows = _clear_shadows.get();
    params.clear_models = _clear_models.get();

    return params;
  }
}
