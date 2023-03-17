// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 410 core

in vec3 position;
in vec3 normal;
in vec3 mccv;
in vec2 texcoord;

uniform mat4 mvp;

out vec3 vary_position;
out vec2 vary_texcoord;
out vec3 vary_normal;
out vec3 vary_mccv;


flat out int chunk_id;

void main()
{
  gl_Position = mvp * vec4(position, 1.0);
  vary_normal = normal;
  vary_position = position;
  vary_texcoord = texcoord;
  vary_mccv = mccv;

  chunk_id = gl_VertexID / 145;
}
