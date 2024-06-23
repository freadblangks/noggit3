// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/trig.hpp>
#include <math/vector_3d.hpp>
#include <noggit/float_property.hpp>
#include <noggit/tool_enums.hpp>

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
    class terrain_tool : public QWidget
    {
      Q_OBJECT

    public:
      terrain_tool(QWidget* parent = nullptr);

      void changeTerrain (World*, math::vector_3d const& pos, float dt);

      void nextType();
      void change_radius(float change) { _radius.change(change); }
      void change_inner_radius(float change) { _inner_radius.change(change); }
      void change_speed(float change) { _speed.change(change); }

      void set_radius(float set) { _radius.set(set); }
      void set_speed(float set) { _speed.set(set); }

      void setOrientation (float orientation);
      void setAngle (float angle);


      // vertex edit only functions
      void moveVertices (World*, float dt);
      void flattenVertices (World*);

      void changeOrientation (float change);
      void changeAngle (float change);
      void setOrientRelativeTo (World*, math::vector_3d const& pos);

      float radius() const { return _radius.get(); }
      float inner_radius_ratio() const { return _inner_radius.get();  }

      void storeCursorPos (math::vector_3d* cursor_pos) { _cursor_pos = cursor_pos; }

      QSize sizeHint() const override;

      eTerrainType _edit_type;

    signals:
      void updateVertices(int vertex_mode, math::degrees const& angle, math::degrees const& orientation);

    private:
      void updateVertexGroup();

      float_property _radius;
      float_property _speed;
      float_property _inner_radius;
      math::degrees _vertex_angle;
      math::degrees _vertex_orientation;

      math::vector_3d* _cursor_pos;

      int _vertex_mode;

      // UI stuff:

      QButtonGroup* _type_button_group;
      QButtonGroup* _vertex_button_group;
      QGroupBox* _speed_box;
      QGroupBox* _vertex_type_group;

      QCheckBox* _only_affect_ground_below_cursor;
      QCheckBox* _only_affect_ground_above_cursor;

      QSlider* _angle_slider;
      QDial* _orientation_dial;

      QCheckBox* _models_follow_ground;
      QCheckBox* _models_follow_ground_normals;
    };
  }
}
