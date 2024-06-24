#include <noggit/camera.hpp>
#include <noggit/settings.hpp>

#include <math/projection.hpp>

namespace noggit
{
  camera::camera ( math::vector_3d const& position_
                 , math::degrees yaw_
                 , math::degrees pitch_
                 )
    : position (position_)
    , move_speed (200.6f)
    , _roll (0.0f)
    , _yaw (0.f)
    , _pitch (0.f)
    , _fov (math::degrees (NoggitSettings.value("fov", 54.f).toFloat()))
  {
    //! \note ensure ranges
    yaw (yaw_);
    pitch (pitch_);
  }

  math::degrees camera::yaw() const
  {
    return _yaw;
  }

  math::degrees camera::yaw (math::degrees value)
  {
    return _yaw = math::degrees(value._);
  }

  void camera::add_to_yaw (math::degrees value)
  {
    yaw (math::degrees (_yaw._ - value._));
  }

  math::degrees camera::pitch() const
  {
    return _pitch;
  }

  math::degrees camera::pitch (math::degrees value)
  {
    return _pitch = math::degrees(value._);;
  }

  void camera::add_to_pitch (math::degrees value)
  {
    pitch (math::degrees (_pitch._ - value._));
  }

  math::degrees camera::roll() const
  {
    return _roll;
  }

  math::degrees camera::roll(math::degrees value)
  {
    return _roll = math::degrees(value._);;
  }

  void camera::add_to_roll(math::degrees value)
  {
    roll(math::degrees(_roll._ - value._));
  }

  math::radians camera::fov() const
  {
    return _fov;
  }

  math::vector_3d camera::look_at() const
  {
    return position + direction();
  }

  math::vector_3d camera::direction() const
  {
    static math::vector_3d const forward (1.0f, 0.0f, 0.0f);

    return ( math::matrix_4x4 ( math::matrix_4x4::rotation_yzx
                              , math::degrees::vec3 (_roll, _yaw, _pitch)
                              )
           * forward
           ).normalize();
  }
  math::vector_3d camera::up() const
  {
    static math::vector_3d const up (0.0f, 1.0f, 0.0f);

    return ( math::matrix_4x4 ( math::matrix_4x4::rotation_yzx
                              , math::degrees::vec3 (_roll, _yaw, _pitch)
                              )
           * up
           ).normalize();
  }

  math::vector_3d camera::right() const
  {
    return -(up() % direction()).normalized();
  }

  math::matrix_4x4 camera::look_at_matrix() const
  {
    return math::look_at(position, look_at(), up());
  }

  void camera::move_forward (float sign, float dt)
  {
    position += direction() * sign * move_speed * dt;
  }

  void camera::move_horizontal (float sign, float dt)
  {
    position += right() * sign * move_speed * dt;
  }

  void camera::move_vertical (float sign, float dt)
  {
    position += up() * sign * move_speed * dt;
  }
}
