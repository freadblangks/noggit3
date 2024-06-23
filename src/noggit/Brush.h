// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

class Brush
{
public:
  Brush(float radius = 15.f, float inner_radius_ratio = 0.5f);

  void set_inner_ratio(float ratio);
  void set_radius(float radius);
  float get_inner_ratio() const { return _inner_ratio; }
  float get_radius() const { return _radius; }
  float get_inner_radius() const { return _inner_size; }

  float value_at_dist(float dist) const;

private:
  void update_values();

  float _inner_ratio;
  float _inner_size;
  float _outer_size;
  float _radius;
};
