// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 410 core


uniform sampler2DArray textures[16];

uniform vec4 ocean_color_light;
uniform vec4 ocean_color_dark;
uniform vec4 river_color_light;
uniform vec4 river_color_dark;
uniform float animtime;

struct liquid_chunk_data
{
  int liquid_vertex_format;
  int array_id;
  int id_start_in_array;
  int texture_count;
  vec2 param;
};

layout (std140) uniform chunk_data
{
  liquid_chunk_data ubo_data[256];
};

in vec3 f_position;
in float depth_;
in vec2 tex_coord_;
flat in int draw_id;

out vec4 out_color;

uniform bool draw_cursor_circle;
uniform vec3 cursor_position;
uniform float cursor_radius;
uniform vec4 cursor_color;


// we already send animtime / 60
const float lava_anim_factor = 1. / (2880. / 60.);

int texture_index_in_array()
{
  return ubo_data[draw_id].id_start_in_array + int(floor(mod(animtime, ubo_data[draw_id].texture_count)));
}

vec2 rot2(vec2 p, float degree)
{
  float a = radians(degree);
  return mat2(cos(a), -sin(a), sin(a), cos(a))*p;
}

vec3 texture_color(int array, int layer, vec2 uv)
{
  if(array == 0)
  {
    return texture(textures[0], vec3(uv, layer)).rgb;
  }
  else if(array == 1)
  {
    return texture(textures[1], vec3(uv, layer)).rgb;
  }
  else if(array == 2)
  {
    return texture(textures[2], vec3(uv, layer)).rgb;
  }
  else if(array == 3)
  {
    return texture(textures[3], vec3(uv, layer)).rgb;
  }
  else if(array == 4)
  {
    return texture(textures[4], vec3(uv, layer)).rgb;
  }
  else if(array == 5)
  {
    return texture(textures[5], vec3(uv, layer)).rgb;
  }
  else if(array == 6)
  {
    return texture(textures[6], vec3(uv, layer)).rgb;
  }
  else if(array == 7)
  {
    return texture(textures[7], vec3(uv, layer)).rgb;
  }
  else if(array == 8)
  {
    return texture(textures[8], vec3(uv, layer)).rgb;
  }
  else if(array == 9)
  {
    return texture(textures[9], vec3(uv, layer)).rgb;
  }
  else if(array == 10)
  {
    return texture(textures[10], vec3(uv, layer)).rgb;
  }
  else if(array == 11)
  {
    return texture(textures[11], vec3(uv, layer)).rgb;
  }
  else if(array == 12)
  {
    return texture(textures[12], vec3(uv, layer)).rgb;
  }
  else if(array == 13)
  {
    return texture(textures[13], vec3(uv, layer)).rgb;
  }
  else if(array == 14)
  {
    return texture(textures[14], vec3(uv, layer)).rgb;
  }
  else if(array == 15)
  {
    return texture(textures[15], vec3(uv, layer)).rgb;
  }

  return vec3(0., 1., 0.);
}

void main()
{
  int type = ubo_data[draw_id].liquid_vertex_format;
  int array = ubo_data[draw_id].array_id;
  int index = texture_index_in_array();
  vec2 param = ubo_data[draw_id].param;

  if(type == 2 || type == 3)
  {
    out_color = vec4(texture_color(array, index, tex_coord_ + param * animtime * lava_anim_factor), 1.);
  }
  else
  {
    vec2 uv = rot2(tex_coord_ * param.x, param.y);
    vec4 texel = vec4(texture_color(array, index, uv), 1.);

    vec4 lerp = (type == 1)
              ? mix (ocean_color_light, ocean_color_dark, depth_) 
              : mix (river_color_light, river_color_dark, depth_)
              ;
              
    //clamp shouldn't be needed
    out_color = vec4 (clamp(texel + lerp, 0.0, 1.0).rgb, lerp.a);
  }  

  if (draw_cursor_circle)
  {
    vec3 fw = fwidth(f_position.xyz);
    float diff = length(f_position.xz - cursor_position.xz);
    float alpha = smoothstep(0.0, length(fw.xz), abs(diff - cursor_radius));

    out_color.rgb = mix(cursor_color.rgb, out_color.rgb, alpha);
  }
}
