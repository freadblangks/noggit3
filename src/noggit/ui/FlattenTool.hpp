// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/float_property.hpp>
#include <noggit/tool_enums.hpp>
#include <math/vector_3d.hpp>

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDial>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

class World;
namespace noggit
{
  namespace ui
  {
    class flatten_blur_tool : public QWidget
    {
    public:
      flatten_blur_tool(QWidget* parent = nullptr);

      void flatten (World* world, math::vector_3d const& cursor_pos, float dt);
      void blur (World* world, math::vector_3d const& cursor_pos, float dt);

      void nextFlattenType();
      void nextFlattenMode();
      void toggleFlattenAngle();
      void toggleFlattenLock();
      void lockPos (math::vector_3d const& cursor_pos);

      void change_radius(float change) { _radius.change(change); }
      void change_speed(float change) { _speed.change(change); }
      void changeOrientation(float change);
      void changeAngle(float change);
      void changeHeight(float change);

      void set_radius(float radius) { _radius.set(radius); }
      void set_speed(float speed) { _speed.set(speed); }
      void setOrientation(float orientation);

      float radius() const { return _radius.get(); }
      float angle() const { return _angle; }
      float orientation() const { return _orientation; }
      bool angled_mode() const { return _angle_group->isChecked(); }
      bool use_ref_pos() const  { return _lock_group->isChecked(); }
      math::vector_3d ref_pos() const { return _lock_pos; }

      QSize sizeHint() const override;

    private:

      float_property _radius;
      float_property _speed;
      float _angle;
      float _orientation;

      math::vector_3d _lock_pos;

      int _flatten_type;
      flatten_mode _flatten_mode;

    private:
      QButtonGroup* _type_button_box;

      QGroupBox* _angle_group;
      QSlider* _angle_slider;
      QDial* _orientation_dial;

      QGroupBox* _lock_group;
      QDoubleSpinBox* _lock_x;
      QDoubleSpinBox* _lock_z;
      QDoubleSpinBox* _lock_h;

      QCheckBox* _lock_up_checkbox;
      QCheckBox* _lock_down_checkbox;
    };
  }
}
