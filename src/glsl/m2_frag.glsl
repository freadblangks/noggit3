// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 410 core
#ifdef use_bindless
#extension GL_ARB_bindless_texture : require
#else
uniform sampler2DArray array_0;
uniform sampler2DArray array_1;
#endif

struct m2_data
{
  vec4 mesh_color;

  int fog_mode;
  int unfogged;
  int unlit;
  int pixel_shader;

  mat4 tex_matrix_1;
  mat4 tex_matrix_2;
  
  uvec2 texture_handle_1;
  uvec2 pad1;
  uvec2 texture_handle_2;
  uvec2 pad2;

  int index_1;
  int index_2;
  ivec2 padding;

  float alpha_test;
  int tex_unit_lookup_1;
  int tex_unit_lookup_2;
  int tex_count;
};

uniform int index;

layout (std140) uniform render_data
{
  m2_data data[192];
};

in vec2 uv1;
in vec2 uv2;
in float camera_dist;
in vec3 norm;

out vec4 out_color;

uniform vec4 fog_color;
uniform float fog_start;
uniform float fog_end;
uniform int draw_fog;

uniform vec3 light_dir;
uniform vec3 diffuse_color;
uniform vec3 ambient_color;



void main()
{
  vec4 color = vec4(0.0);

  if(data[index].mesh_color.a < data[index].alpha_test)
  {
    discard;
  }

#ifdef use_bindless
  vec4 texture1 = texture(sampler2DArray(data[index].texture_handle_1), vec3(uv1, data[index].index_1));
#else
  vec4 texture1 = texture(array_0, vec3(uv1, data[index].index_1));
#endif

  vec4 texture2 = vec4(0.);
  
  if(data[index].tex_count > 1)
  {
#ifdef use_bindless
  texture2 = texture(sampler2DArray(data[index].texture_handle_2), vec3(uv2, data[index].index_2));
#else
  texture2 = texture(array_1, vec3(uv2, data[index].index_2));
#endif
  }

  // code from Deamon87 and https://wowdev.wiki/M2/Rendering#Pixel_Shaders
  if (data[index].pixel_shader == 0) //Combiners_Opaque
  { 
    color.rgb = texture1.rgb * data[index].mesh_color.rgb;
    color.a = data[index].mesh_color.a;
  } 
  else if (data[index].pixel_shader == 1) // Combiners_Decal
  { 
    color.rgb = mix(data[index].mesh_color.rgb, texture1.rgb, data[index].mesh_color.a);
    color.a = data[index].mesh_color.a;
  } 
  else if (data[index].pixel_shader == 2) // Combiners_Add
  { 
    color.rgba = texture1.rgba + data[index].mesh_color.rgba;
  } 
  else if (data[index].pixel_shader == 3) // Combiners_Mod2x
  { 
    color.rgb = texture1.rgb * data[index].mesh_color.rgb * vec3(2.0);
    color.a = texture1.a * data[index].mesh_color.a * 2.0;
  } 
  else if (data[index].pixel_shader == 4) // Combiners_Fade
  { 
    color.rgb = mix(texture1.rgb, data[index].mesh_color.rgb, data[index].mesh_color.a);
    color.a = data[index].mesh_color.a;
  } 
  else if (data[index].pixel_shader == 5) // Combiners_Mod
  { 
    color.rgba = texture1.rgba * data[index].mesh_color.rgba;
  } 
  else if (data[index].pixel_shader == 6) // Combiners_Opaque_Opaque
  { 
    color.rgb = texture1.rgb * texture2.rgb * data[index].mesh_color.rgb;
    color.a = data[index].mesh_color.a;
  } 
  else if (data[index].pixel_shader == 7) // Combiners_Opaque_Add
  { 
    color.rgb = texture2.rgb + texture1.rgb * data[index].mesh_color.rgb;
    color.a = data[index].mesh_color.a + texture1.a;
  } 
  else if (data[index].pixel_shader == 8) // Combiners_Opaque_Mod2x
  { 
    color.rgb = texture1.rgb * data[index].mesh_color.rgb * texture2.rgb * vec3(2.0);
    color.a  = texture2.a * data[index].mesh_color.a * 2.0;
  } 
  else if (data[index].pixel_shader == 9)  // Combiners_Opaque_Mod2xNA
  {
    color.rgb = texture1.rgb * data[index].mesh_color.rgb * texture2.rgb * vec3(2.0);
    color.a  = data[index].mesh_color.a;
  } 
  else if (data[index].pixel_shader == 10) // Combiners_Opaque_AddNA
  { 
    color.rgb = texture2.rgb + texture1.rgb * data[index].mesh_color.rgb;
    color.a = data[index].mesh_color.a;
  } 
  else if (data[index].pixel_shader == 11) // Combiners_Opaque_Mod
  { 
    color.rgb = texture1.rgb * texture2.rgb * data[index].mesh_color.rgb;
    color.a = texture2.a * data[index].mesh_color.a;
  } 
  else if (data[index].pixel_shader == 12) // Combiners_Mod_Opaque
  { 
    color.rgb = texture1.rgb * texture2.rgb * data[index].mesh_color.rgb;
    color.a = texture1.a;
  } 
  else if (data[index].pixel_shader == 13) // Combiners_Mod_Add
  { 
     color.rgba = texture2.rgba + texture1.rgba * data[index].mesh_color.rgba;
  } 
  else if (data[index].pixel_shader == 14) // Combiners_Mod_Mod2x
  { 
    color.rgba = texture1.rgba * texture2.rgba * data[index].mesh_color.rgba * vec4(2.0);
  } 
  else if (data[index].pixel_shader == 15) // Combiners_Mod_Mod2xNA
  { 
    color.rgb = texture1.rgb * texture2.rgb * data[index].mesh_color.rgb * vec3(2.0);
    color.a = texture1.a * data[index].mesh_color.a;
  } 
  else if (data[index].pixel_shader == 16) // Combiners_Mod_AddNA
  { 
    color.rgb = texture2.rgb + texture1.rgb * data[index].mesh_color.rgb;
    color.a = texture1.a * data[index].mesh_color.a;
  } 
  else if (data[index].pixel_shader == 17) // Combiners_Mod_Mod
  { 
    color.rgba = texture1.rgba * texture2.rgba * data[index].mesh_color.rgba;
  } 
  else if (data[index].pixel_shader == 18) // Combiners_Add_Mod
  { 
    color.rgb = (texture1.rgb + data[index].mesh_color.rgb) * texture2.a;
    color.a = (texture1.a + data[index].mesh_color.a) * texture2.a;
  } 
  else if (data[index].pixel_shader == 19) // Combiners_Mod2x_Mod2x
  {
    color.rgba = texture1.rgba * texture2.rgba * data[index].mesh_color.rgba * vec4(4.0);
  }
  else if (data[index].pixel_shader == 20)  // Combiners_Opaque_Mod2xNA_Alpha
  {
    color.rgb = (data[index].mesh_color.rgb * texture1.rgb) * mix(texture2.rgb * 2.0, vec3(1.0), texture1.a);
    color.a = data[index].mesh_color.a;
  }
  else if (data[index].pixel_shader == 21)   //Combiners_Opaque_AddAlpha
  {
    color.rgb = (data[index].mesh_color.rgb * texture1.rgb) + (texture2.rgb * texture2.a);
    color.a = data[index].mesh_color.a;
  }
  else if (data[index].pixel_shader == 22)   // Combiners_Opaque_AddAlpha_Alpha
  {
    color.rgb = (data[index].mesh_color.rgb * texture1.rgb) + (texture2.rgb * texture2.a * texture1.a);
    color.a = data[index].mesh_color.a;
  }

  if(color.a < data[index].alpha_test)
  {
    discard;
  }

  if(data[index].unlit == 0)
  {
    // diffuse + ambient lighting  
    color.rgb *= vec3(clamp (diffuse_color * max(dot(norm, light_dir), 0.0), 0.0, 1.0)) + ambient_color;
  }  

  if(draw_fog == 1 && data[index].unfogged == 0 && camera_dist >= fog_end * fog_start)
  {
    float start = fog_end * fog_start;
    float alpha = (camera_dist - start) / (fog_end - start);

    vec3 fog;

    // see https://wowdev.wiki/M2/Rendering#Fog_Modes
    if(data[index].fog_mode == 1)
    {
      fog = fog_color.rgb;
    }
    else if(data[index].fog_mode == 2)
    {
      fog = vec3(0.);
    }
    else if(data[index].fog_mode == 3)
    {
      fog = vec3(1.);
    }
    else if(data[index].fog_mode == 4)
    {
      fog = vec3(0.5);
    }

    color.rgb = mix(color.rgb, fog, alpha);
  }

  out_color = color;
}
