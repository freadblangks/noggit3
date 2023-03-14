// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 410 core


struct m2_data
{
  vec4 mesh_color;

  int fog_mode;
  int unfogged;
  int unlit;
  int pixel_shader;

  mat4 tex_matrix_1;
  mat4 tex_matrix_2;
  
  ivec4 tex_param;

  float alpha_test;
  int tex_unit_lookup_1;
  int tex_unit_lookup_2;
  int padding;
};

layout (std140) uniform chunk_data
{
  m2_data data;
};

in vec2 uv1;
in vec2 uv2;
in float camera_dist;
in vec3 norm;

out vec4 out_color;

uniform sampler2DArray textures[32];

uniform vec4 fog_color;
uniform float fog_start;
uniform float fog_end;
uniform int draw_fog;

uniform vec3 light_dir;
uniform vec3 diffuse_color;
uniform vec3 ambient_color;

vec4 tex_color(ivec2 param, vec2 uv)
{
  if(param.x == 0)
  {
    return texture(textures[0], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 1)
  {
    return texture(textures[1], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 2)
  {
    return texture(textures[2], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 3)
  {
    return texture(textures[3], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 4)
  {
    return texture(textures[4], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 5)
  {
    return texture(textures[5], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 6)
  {
    return texture(textures[6], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 7)
  {
    return texture(textures[7], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 8)
  {
    return texture(textures[8], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 9)
  {
    return texture(textures[9], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 10)
  {
    return texture(textures[10], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 11)
  {
    return texture(textures[11], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 12)
  {
    return texture(textures[12], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 13)
  {
    return texture(textures[13], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 14)
  {
    return texture(textures[14], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 15)
  {
    return texture(textures[15], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 16)
  {
    return texture(textures[16], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 17)
  {
    return texture(textures[17], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 18)
  {
    return texture(textures[18], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 19)
  {
    return texture(textures[19], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 20)
  {
    return texture(textures[20], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 21)
  {
    return texture(textures[21], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 22)
  {
    return texture(textures[22], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 23)
  {
    return texture(textures[23], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 24)
  {
    return texture(textures[24], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 25)
  {
    return texture(textures[25], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 26)
  {
    return texture(textures[26], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 27)
  {
    return texture(textures[27], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 28)
  {
    return texture(textures[28], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 29)
  {
    return texture(textures[29], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 30)
  {
    return texture(textures[30], vec3(uv, param.y + 0.1));
  }
  else if(param.x == 31)
  {
    return texture(textures[31], vec3(uv, param.y + 0.1));
  }
}

void main()
{
  vec4 color = vec4(0.0);

  if(data.mesh_color.a < data.alpha_test)
  {
    discard;
  }

  vec4 texture1 = tex_color(data.tex_param.xy, uv1);
  vec4 texture2 = tex_color(data.tex_param.zw, uv2);
  
  // code from Deamon87 and https://wowdev.wiki/M2/Rendering#Pixel_Shaders
  if (data.pixel_shader == 0) //Combiners_Opaque
  { 
    color.rgb = texture1.rgb * data.mesh_color.rgb;
    color.a = data.mesh_color.a;
  } 
  else if (data.pixel_shader == 1) // Combiners_Decal
  { 
    color.rgb = mix(data.mesh_color.rgb, texture1.rgb, data.mesh_color.a);
    color.a = data.mesh_color.a;
  } 
  else if (data.pixel_shader == 2) // Combiners_Add
  { 
    color.rgba = texture1.rgba + data.mesh_color.rgba;
  } 
  else if (data.pixel_shader == 3) // Combiners_Mod2x
  { 
    color.rgb = texture1.rgb * data.mesh_color.rgb * vec3(2.0);
    color.a = texture1.a * data.mesh_color.a * 2.0;
  } 
  else if (data.pixel_shader == 4) // Combiners_Fade
  { 
    color.rgb = mix(texture1.rgb, data.mesh_color.rgb, data.mesh_color.a);
    color.a = data.mesh_color.a;
  } 
  else if (data.pixel_shader == 5) // Combiners_Mod
  { 
    color.rgba = texture1.rgba * data.mesh_color.rgba;
  } 
  else if (data.pixel_shader == 6) // Combiners_Opaque_Opaque
  { 
    color.rgb = texture1.rgb * texture2.rgb * data.mesh_color.rgb;
    color.a = data.mesh_color.a;
  } 
  else if (data.pixel_shader == 7) // Combiners_Opaque_Add
  { 
    color.rgb = texture2.rgb + texture1.rgb * data.mesh_color.rgb;
    color.a = data.mesh_color.a + texture1.a;
  } 
  else if (data.pixel_shader == 8) // Combiners_Opaque_Mod2x
  { 
    color.rgb = texture1.rgb * data.mesh_color.rgb * texture2.rgb * vec3(2.0);
    color.a  = texture2.a * data.mesh_color.a * 2.0;
  } 
  else if (data.pixel_shader == 9)  // Combiners_Opaque_Mod2xNA
  {
    color.rgb = texture1.rgb * data.mesh_color.rgb * texture2.rgb * vec3(2.0);
    color.a  = data.mesh_color.a;
  } 
  else if (data.pixel_shader == 10) // Combiners_Opaque_AddNA
  { 
    color.rgb = texture2.rgb + texture1.rgb * data.mesh_color.rgb;
    color.a = data.mesh_color.a;
  } 
  else if (data.pixel_shader == 11) // Combiners_Opaque_Mod
  { 
    color.rgb = texture1.rgb * texture2.rgb * data.mesh_color.rgb;
    color.a = texture2.a * data.mesh_color.a;
  } 
  else if (data.pixel_shader == 12) // Combiners_Mod_Opaque
  { 
    color.rgb = texture1.rgb * texture2.rgb * data.mesh_color.rgb;
    color.a = texture1.a;
  } 
  else if (data.pixel_shader == 13) // Combiners_Mod_Add
  { 
     color.rgba = texture2.rgba + texture1.rgba * data.mesh_color.rgba;
  } 
  else if (data.pixel_shader == 14) // Combiners_Mod_Mod2x
  { 
    color.rgba = texture1.rgba * texture2.rgba * data.mesh_color.rgba * vec4(2.0);
  } 
  else if (data.pixel_shader == 15) // Combiners_Mod_Mod2xNA
  { 
    color.rgb = texture1.rgb * texture2.rgb * data.mesh_color.rgb * vec3(2.0);
    color.a = texture1.a * data.mesh_color.a;
  } 
  else if (data.pixel_shader == 16) // Combiners_Mod_AddNA
  { 
    color.rgb = texture2.rgb + texture1.rgb * data.mesh_color.rgb;
    color.a = texture1.a * data.mesh_color.a;
  } 
  else if (data.pixel_shader == 17) // Combiners_Mod_Mod
  { 
    color.rgba = texture1.rgba * texture2.rgba * data.mesh_color.rgba;
  } 
  else if (data.pixel_shader == 18) // Combiners_Add_Mod
  { 
    color.rgb = (texture1.rgb + data.mesh_color.rgb) * texture2.a;
    color.a = (texture1.a + data.mesh_color.a) * texture2.a;
  } 
  else if (data.pixel_shader == 19) // Combiners_Mod2x_Mod2x
  {
    color.rgba = texture1.rgba * texture2.rgba * data.mesh_color.rgba * vec4(4.0);
  }
  else if (data.pixel_shader == 20)  // Combiners_Opaque_Mod2xNA_Alpha
  {
    color.rgb = (data.mesh_color.rgb * texture1.rgb) * mix(texture2.rgb * 2.0, vec3(1.0), texture1.a);
    color.a = data.mesh_color.a;
  }
  else if (data.pixel_shader == 21)   //Combiners_Opaque_AddAlpha
  {
    color.rgb = (data.mesh_color.rgb * texture1.rgb) + (texture2.rgb * texture2.a);
    color.a = data.mesh_color.a;
  }
  else if (data.pixel_shader == 22)   // Combiners_Opaque_AddAlpha_Alpha
  {
    color.rgb = (data.mesh_color.rgb * texture1.rgb) + (texture2.rgb * texture2.a * texture1.a);
    color.a = data.mesh_color.a;
  }

  if(color.a < data.alpha_test)
  {
    discard;
  }

  if(data.unlit == 0)
  {
    // diffuse + ambient lighting  
    color.rgb *= vec3(clamp (diffuse_color * max(dot(norm, light_dir), 0.0), 0.0, 1.0)) + ambient_color;
  }  

  if(draw_fog == 1 && data.unfogged == 0 && camera_dist >= fog_end * fog_start)
  {
    float start = fog_end * fog_start;
    float alpha = (camera_dist - start) / (fog_end - start);

    vec3 fog;

    // see https://wowdev.wiki/M2/Rendering#Fog_Modes
    if(data.fog_mode == 1)
    {
      fog = fog_color.rgb;
    }
    else if(data.fog_mode == 2)
    {
      fog = vec3(0.);
    }
    else if(data.fog_mode == 3)
    {
      fog = vec3(1.);
    }
    else if(data.fog_mode == 4)
    {
      fog = vec3(0.5);
    }

    color.rgb = mix(color.rgb, fog, alpha);
  }

  out_color = color;
}
