// This file is part of Noggit3, licensed under GNU General Public License (version 3). 
#version 410 core

struct chunk_shader_data
{
  bool has_shadow;
  bool is_textured;
  bool cant_paint;
  bool impassible_flag;

  vec3 tex_anim[4]; // direction + speed
  vec4 areaid_color;
  ivec4 tex_param_0;
  ivec4 tex_param_1;
};

layout (std140) uniform chunk_data
{
  chunk_shader_data ubo_data[256];
};


// todo: move to opengl 4.1+ to be able to use the layout qualifier to be able to validate the program on creation
uniform sampler2DArray alphamap;
// todo: use a dynamically set define for the array size
uniform sampler2DArray texture_arrays[31];


uniform bool show_unpaintable_chunks;
uniform bool draw_impassible_flag;
uniform bool draw_areaid_overlay;
uniform bool draw_terrain_height_contour;
uniform bool draw_lines;
uniform bool draw_hole_lines;

uniform bool draw_wireframe;
uniform int wireframe_type;
uniform float wireframe_radius;
uniform float wireframe_width;
uniform vec4 wireframe_color;
uniform bool rainbow_wireframe;

uniform vec3 camera;
uniform bool draw_fog;
uniform vec4 fog_color;
uniform float fog_start;
uniform float fog_end;

uniform bool draw_cursor_circle;
uniform vec3 cursor_position;
uniform float outer_cursor_radius;
uniform float inner_cursor_ratio;
uniform vec4 cursor_color;

uniform vec3 light_dir;
uniform vec3 diffuse_color;
uniform vec3 ambient_color;

uniform float anim_time;


in vec3 vary_position;
in vec2 vary_texcoord;
in vec3 vary_normal;
in vec3 vary_mccv;

flat in int chunk_id;

out vec4 out_color;

const float TILESIZE  = 533.33333;
const float CHUNKSIZE = TILESIZE / 16.0;
const float HOLESIZE  = CHUNKSIZE * 0.25;
const float UNITSIZE = HOLESIZE * 0.5;

// required because opengl can't handle non uniform index for uniform arrays
vec3 texture_color(int array_index, int index_in_array, vec2 anim_uv)
{
  if(array_index == 0)
  {
    return texture(texture_arrays[0], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 1)
  {
    return texture(texture_arrays[1], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 2)
  {
    return texture(texture_arrays[2], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 3)
  {
    return texture(texture_arrays[3], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 4)
  {
    return texture(texture_arrays[4], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 5)
  {
    return texture(texture_arrays[5], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 6)
  {
    return texture(texture_arrays[6], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 7)
  {
    return texture(texture_arrays[7], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 8)
  {
    return texture(texture_arrays[8], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 9)
  {
    return texture(texture_arrays[9], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 10)
  {
    return texture(texture_arrays[10], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 11)
  {
    return texture(texture_arrays[11], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 12)
  {
    return texture(texture_arrays[12], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 13)
  {
    return texture(texture_arrays[13], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 14)
  {
    return texture(texture_arrays[1], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 15)
  {
    return texture(texture_arrays[1], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 16)
  {
    return texture(texture_arrays[1], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 17)
  {
    return texture(texture_arrays[1], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 18)
  {
    return texture(texture_arrays[1], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 19)
  {
    return texture(texture_arrays[1], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 20)
  {
    return texture(texture_arrays[2], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 21)
  {
    return texture(texture_arrays[2], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 22)
  {
    return texture(texture_arrays[2], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 23)
  {
    return texture(texture_arrays[2], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 24)
  {
    return texture(texture_arrays[2], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 25)
  {
    return texture(texture_arrays[2], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 26)
  {
    return texture(texture_arrays[2], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 27)
  {
    return texture(texture_arrays[2], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 28)
  {
    return texture(texture_arrays[2], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 29)
  {
    return texture(texture_arrays[29], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
  else if(array_index == 30)
  {
    return texture(texture_arrays[30], vec3(vary_texcoord + anim_uv, index_in_array)).rgb;
  }
}

vec2 anim_uv(int index)
{
  vec3 v = ubo_data[chunk_id].tex_anim[index];
  return v.xy * mod(anim_time * v.z, 1.);
}

vec4 texture_blend() 
{
  if(!ubo_data[chunk_id].is_textured)
    return vec4 (1.0, 1.0, 1.0, 1.0);

  vec3 alpha = texture(alphamap, vec3(vary_texcoord / 8., chunk_id + 0.1)).rgb;

  float a0 = alpha.r;
  float a1 = alpha.g;
  float a2 = alpha.b;

  ivec4 p0 = ubo_data[chunk_id].tex_param_0;
  ivec4 p1 = ubo_data[chunk_id].tex_param_1;

  vec3 t0 = texture_color(p0.x, p1.x, anim_uv(0));
  vec3 t1 = texture_color(p0.y, p1.y, anim_uv(1));
  vec3 t2 = texture_color(p0.z, p1.z, anim_uv(2));
  vec3 t3 = texture_color(p0.w, p1.w, anim_uv(3));

  return vec4 (t0 * (1.0 - (a0 + a1 + a2)) + t1 * a0 + t2 * a1 + t3 * a2, 1.0);
}

float contour_alpha(float unit_size, float pos, float line_width)
{
  float f = abs(fract((pos + unit_size*0.5) / unit_size) - 0.5);
  float df = abs(line_width / unit_size);
  return smoothstep(0.0, df, f);
}

float contour_alpha(float unit_size, vec2 pos, vec2 line_width)
{
  return 1.0 - min( contour_alpha(unit_size, pos.x, line_width.x)
                  , contour_alpha(unit_size, pos.y, line_width.y)
                  );
}

void main()
{
  float dist_from_camera = distance(camera, vary_position);

  if(draw_fog && dist_from_camera >= fog_end)
  {
    out_color = fog_color;
    return;
  } 
  vec3 fw = fwidth(vary_position.xyz);

  out_color = texture_blend();
  out_color.rgb *= vary_mccv;

  // diffuse + ambient lighting
  out_color.rgb *= vec3(clamp (diffuse_color * max(dot(vary_normal, light_dir), 0.0), 0.0, 1.0)) + ambient_color;

  if(show_unpaintable_chunks && ubo_data[chunk_id].cant_paint)
  {
    out_color *= vec4(1.0, 0.0, 0.0, 1.0);
  }
  
  if(draw_areaid_overlay)
  {
    out_color = out_color * 0.3 + ubo_data[chunk_id].areaid_color;
  }

  if(draw_impassible_flag && ubo_data[chunk_id].impassible_flag)
  {
    out_color.rgb = mix(vec3(1.0), out_color.rgb, 0.5);
  }

  if(ubo_data[chunk_id].has_shadow)
  {
    out_color = vec4 (out_color.rgb * (1.0 - texture(alphamap, vec3(vary_texcoord / 8.0, chunk_id + 0.1)).a * 0.333) , 1.0);
  }  

  if (draw_terrain_height_contour)
  {
    out_color = vec4(out_color.rgb * contour_alpha(4.0, vary_position.y+0.1, fw.y), 1.0);
  }

  bool lines_drawn = false;
  if(draw_lines)
  {
    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

    color.a = contour_alpha(TILESIZE, vary_position.xz, fw.xz * 1.5);
    color.g = color.a > 0.0 ? 0.8 : 0.0;

    if(color.a == 0.0)
    {
      color.a = contour_alpha(CHUNKSIZE, vary_position.xz, fw.xz);
      color.r = color.a > 0.0 ? 0.8 : 0.0;
    }
    if(draw_hole_lines && color.a == 0.0)
    {
      color.a = contour_alpha(HOLESIZE, vary_position.xz, fw.xz * 0.75);
      color.b = 0.8;
    }
    
    lines_drawn = color.a > 0.0;
    out_color.rgb = mix(out_color.rgb, color.rgb, color.a);
  }

  if(draw_fog && dist_from_camera >= fog_end * fog_start)
  {
    float start = fog_end * fog_start;
    float alpha = (dist_from_camera - start) / (fog_end - start);
    out_color.rgb = mix(out_color.rgb, fog_color.rgb, alpha);
  }

  if(draw_wireframe && !lines_drawn)
  {
    // true by default => type 0
	  bool draw_wire = true;
    float real_wireframe_radius = max(outer_cursor_radius * wireframe_radius, 2.0 * UNITSIZE); 
	
	  if(wireframe_type == 1)
	  {
		  draw_wire = (length(vary_position.xz - cursor_position.xz) < real_wireframe_radius);
	  }
	
	  if(draw_wire)
	  {
		  float alpha = contour_alpha(UNITSIZE, vary_position.xz, fw.xz * wireframe_width);
		  float xmod = mod(vary_position.x, UNITSIZE);
		  float zmod = mod(vary_position.z, UNITSIZE);
		  float d = length(fw.xz) * wireframe_width;
		  float diff = min( min(abs(xmod - zmod), abs(xmod - UNITSIZE + zmod))
                      , min(abs(zmod - xmod), abs(zmod + UNITSIZE - zmod))
                      );        

		  alpha = max(alpha, 1.0 - smoothstep(0.0, d, diff));
      out_color.rgb = mix(out_color.rgb, wireframe_color.rgb, wireframe_color.a*alpha);
	  }
  }

  if (draw_cursor_circle)
  {
    float diff = length(vary_position.xz - cursor_position.xz);
    diff = min(abs(diff - outer_cursor_radius), abs(diff - outer_cursor_radius * inner_cursor_ratio));
    float alpha = smoothstep(0.0, length(fw.xz), diff);

    out_color.rgb = mix(cursor_color.rgb, out_color.rgb, alpha);
  }
}
