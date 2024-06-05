// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/chunk_mover.hpp>

#include <QWidget>

namespace noggit::ui
{
  class chunk_mover_ui : public QWidget
  {
  public:
    chunk_mover_ui(noggit::chunk_mover* chunk_mover, QWidget* parent = nullptr);

    void change_height_offset(float change);
  private:
    noggit::chunk_mover* _chunk_mover;
  };
}
