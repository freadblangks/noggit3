// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

in vec4 position;
in vec3 normal;
in vec4 color;
in vec2 uv1;
in vec2 uv2;
in int id;

in mat4 transform;

out vec3 f_position;
out vec3 f_normal;
out vec2 f_texcoord;
out vec2 f_texcoord_2;
out vec4 f_vertex_color;
flat out int index;

uniform mat4 model_view;
uniform mat4 projection;

struct batch_uniforms
{
  uvec2 texture_1;
  uvec2 padding_1;
  uvec2 texture_2;
  uvec2 padding_2;

  int texture_index_1;
  int texture_index_2;
  int use_vertex_color;
  int exterior_lit;

  int shader_id;
  int unfogged;
  int unlit;
  float alpha_test;
};


layout (std140) uniform render_data
{
  batch_uniforms data[96];
};

void main()
{
  index = id;
  vec4 pos = transform * position;
  vec4 view_space_pos = model_view * pos;
  gl_Position = projection * view_space_pos;

  f_position = pos.xyz;
  f_normal = mat3(transform) * normal;

  // Env and EnvMetal
  if(data[index].shader_id == 3 || data[index].shader_id == 5)
  {
    f_texcoord = uv1;
    f_texcoord_2 = reflect(normalize(view_space_pos.xyz), f_normal).xy;
  }
  else
  {
    f_texcoord = uv1;
    f_texcoord_2 = uv2;
  }
  
  f_vertex_color = color;
}
