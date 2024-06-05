// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/chunk_mover.hpp>
#include <noggit/ui/double_spinbox.hpp>

#include <QFormLayout>

namespace noggit::ui
{
  chunk_mover_ui::chunk_mover_ui(noggit::chunk_mover* chunk_mover, QWidget* parent)
    : QWidget(parent)
    , _chunk_mover(chunk_mover)
  {
    auto layout(new QFormLayout(this));

    auto spinbox(new double_spinbox(&chunk_mover->height_offset_property(), this));
    spinbox->setDecimals(3);
    spinbox->setRange(-20000.f, 20000.f);

    layout->addRow("Height Offset:", spinbox);
  }

  void chunk_mover_ui::change_height_offset(float change)
  {
    _chunk_mover->height_offset_property().change(change);
  }
}
