// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

uniform sampler2DArray textures[32];

uniform ivec4 tex_param;

uniform bool use_vertex_color;

uniform bool draw_fog;
uniform bool unfogged;
uniform float fog_start;
uniform float fog_end;
uniform vec3 fog_color;
uniform vec3 camera;

uniform bool unlit;
uniform bool exterior_lit;
uniform vec3 exterior_light_dir;
uniform vec3 exterior_diffuse_color;
uniform vec3 exterior_ambient_color;
uniform vec3 ambient_color;

uniform float alpha_test;

uniform int shader_id;

in vec3 f_position;
in vec3 f_normal;
in vec2 f_texcoord;
in vec2 f_texcoord_2;
in vec4 f_vertex_color;

out vec4 out_color;

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

vec3 lighting(vec3 material)
{
  vec3 light_color = vec3(1.);
  vec3 vertex_color = use_vertex_color ? f_vertex_color.rgb : vec3(0.);

  if(unlit)
  {
    light_color = vertex_color + (exterior_lit ? exterior_ambient_color : ambient_color);
  }
  else if(exterior_lit)
  {
    vec3 ambient = exterior_ambient_color + vertex_color.rgb;

    light_color = vec3(clamp (exterior_diffuse_color * max(dot(f_normal, exterior_light_dir), 0.0), 0.0, 1.0)) + ambient;
  }
  else
  {
    light_color = ambient_color + vertex_color.rgb;
  }  

  return material * light_color;
}

void main()
{
  float dist_from_camera = distance(camera, f_position);
  bool fog = draw_fog && !unfogged;

  if(fog && dist_from_camera >= fog_end)
  {
    out_color = vec4(fog_color, 1.);
    return;
  }

  vec4 tex   = tex_color(tex_param.xy, f_texcoord);
  vec4 tex_2 = tex_color(tex_param.zw, f_texcoord_2);

  if(tex.a < alpha_test)
  {
    discard;
  }

  vec4 vertex_color = vec4(0., 0., 0., 1.f);
  vec3 light_color = vec3(1.);

  if(use_vertex_color) 
  {
    vertex_color = f_vertex_color;
  }


  // see: https://github.com/Deamon87/WebWowViewerCpp/blob/master/wowViewerLib/src/glsl/wmoShader.glsl
  if(shader_id == 3) // Env
  {
    vec3 env = tex_2.rgb * tex.rgb;
    out_color = vec4(lighting(tex.rgb) + env, 1.);
  }
  else if(shader_id == 5) // EnvMetal
  {
    vec3 env = tex_2.rgb * tex.rgb * tex.a;
    out_color = vec4(lighting(tex.rgb) + env, 1.);
  }
  else if(shader_id == 6) // TwoLayerDiffuse
  {
    vec3 layer2 = mix(tex.rgb, tex_2.rgb, tex_2.a);
    out_color = vec4(lighting(mix(layer2, tex.rgb, vertex_color.a)), 1.);
  }
  else // default shader, used for shader_id 0,1,2,4 (Diffuse, Specular, Metal, Opaque)
  {
    out_color = vec4(lighting(tex.rgb), 1.);
  }

  if(fog && (dist_from_camera >= fog_end * fog_start))
  {
    float start = fog_end * fog_start;
    float alpha = (dist_from_camera - start) / (fog_end - start);

    out_color.rgb = mix(out_color.rgb, fog_color, alpha);
  }

  if(out_color.a < alpha_test)
  {
    discard;
  }
}
