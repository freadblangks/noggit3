// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 410 core
#ifdef use_bindless
#extension GL_ARB_bindless_texture : require
#else
uniform sampler2DArray array_0;
uniform sampler2DArray array_1;
#endif

uniform bool draw_fog;
uniform float fog_start;
uniform float fog_end;
uniform vec3 fog_color;
uniform vec3 camera;


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


flat in int index;

layout (std140) uniform render_data
{
  batch_uniforms data[96];
};

uniform vec3 exterior_light_dir;
uniform vec3 exterior_diffuse_color;
uniform vec3 exterior_ambient_color;
uniform vec3 ambient_color;


in vec3 f_position;
in vec3 f_normal;
in vec2 f_texcoord;
in vec2 f_texcoord_2;
in vec4 f_vertex_color;

out vec4 out_color;

vec3 lighting(vec3 material)
{
  vec3 light_color = vec3(1.);
  vec3 vertex_color = data[index].use_vertex_color != 0 ? f_vertex_color.rgb : vec3(0.);

  if(data[index].unlit != 0)
  {
    light_color = vertex_color + (data[index].exterior_lit != 0 ? exterior_ambient_color : ambient_color);
  }
  else if(data[index].exterior_lit != 0)
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
  bool fog = draw_fog && data[index].unfogged == 0;

  if(fog && dist_from_camera >= fog_end)
  {
    out_color = vec4(fog_color, 1.);
    return;
  }
#ifdef use_bindless
  vec4 tex = texture(sampler2DArray(data[index].texture_1), vec3(f_texcoord, data[index].texture_index_1));
#else
  vec4 tex = texture(array_0, vec3(f_texcoord, data[index].texture_index_1));
#endif
  
  vec4 tex_2 = vec4(0.);

  if(tex.a < data[index].alpha_test)
  {
    discard;
  }

  int shader = data[index].shader_id;

  if(shader == 3 || shader == 6 || shader == 5)
  {

#ifdef use_bindless
    tex_2 = texture(sampler2DArray(data[index].texture_2), vec3(f_texcoord_2, data[index].texture_index_2));
#else
    tex_2 = texture(array_1, vec3(f_texcoord_2, data[index].texture_index_2));
#endif
  }

  vec4 vertex_color = vec4(0., 0., 0., 1.f);
  vec3 light_color = vec3(1.);

  if(data[index].use_vertex_color != 0) 
  {
    vertex_color = f_vertex_color;
  }

  // see: https://github.com/Deamon87/WebWowViewerCpp/blob/master/wowViewerLib/src/glsl/wmoShader.glsl
  if(shader == 3) // Env
  {
    vec3 env = tex_2.rgb * tex.rgb;
    out_color = vec4(lighting(tex.rgb) + env, 1.);
  }
  else if(shader == 5) // EnvMetal
  {
    vec3 env = tex_2.rgb * tex.rgb * tex.a;
    out_color = vec4(lighting(tex.rgb) + env, 1.);
  }
  else if(shader == 6) // TwoLayerDiffuse
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

  if(out_color.a < data[index].alpha_test)
  {
    discard;
  }
}
