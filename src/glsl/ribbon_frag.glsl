// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 410 core
#ifdef use_bindless
#extension GL_ARB_bindless_texture : require

uniform uvec2 tex_array;
#else
uniform sampler2DArray tex_array;
#endif

in vec2 f_uv;

out vec4 out_color;

uniform int index_in_array;
uniform vec4 color;

void main()
{
  #ifdef use_bindless
  vec4 t = texture(sampler2DArray(tex_array), vec3(f_uv, index_in_array));
#else
  vec4 t = texture(tex_array, vec3(f_uv, index_in_array));
#endif

  out_color = vec4(color.rgb * t.rgb, color.a);
}
