// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 410 core
//todo: add fallback in case the extension isn't supported for the user
#extension GL_ARB_shader_draw_parameters : require

in vec4 position;
in vec2 tex_coord;
in float depth;

uniform mat4 model_view;
uniform mat4 projection;
uniform mat4 transform;

uniform int use_transform = int(0);

out vec3 f_position;
out float depth_;
out vec2 tex_coord_;
flat out int draw_id;

void main()
{
  f_position = position.xyz;
  depth_ = depth;
  tex_coord_ = tex_coord;

  if(use_transform == 1)
  {
    gl_Position = projection * model_view * transform * position;
    draw_id = 0;
  }
  else
  {
    gl_Position = projection * model_view * position;
    draw_id = gl_DrawIDARB;
  }
}
