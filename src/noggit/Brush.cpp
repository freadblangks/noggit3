// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Brush.h>

Brush::Brush(float radius, float inner_ratio)
  : _radius(radius)
  , _inner_ratio(inner_ratio)
{
  update_values();
}

void Brush::set_inner_ratio(float ratio)
{
  _inner_ratio = ratio;
  update_values();
}
void Brush::set_radius(float radius)
{
  _radius = radius;
  update_values();
}

float Brush::value_at_dist(float dist) const
{
  if (dist > _radius)
  {
    return 0.0f;
  }
  if (dist < _inner_size)
  {
    return 1.0f;
  }

  // lerp from the start of the inner part to the radius
  return 1.0f - ((dist - _inner_size) / _outer_size);
}

void Brush::update_values()
{
  _inner_size = _inner_ratio * _radius;
  _outer_size = _radius - _inner_size;
}
