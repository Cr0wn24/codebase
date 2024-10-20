#version 300 es

layout(location = 0) in vec2 v_dst_min;
layout(location = 1) in vec2 v_dst_max;
layout(location = 2) in vec2 v_src_min;
layout(location = 3) in vec2 v_src_max;
layout(location = 4) in mat4 v_colors;
layout(location = 8) in vec4 v_corner_radius;
layout(location = 9) in vec4 v_extra; // 0: softness, 1: border thickness, 2: omit_texture

out vec4 f_color;
out float f_corner_radius;
out vec2 f_corner_radius_coord;
out vec2 f_dst_half_size;
out vec2 f_dst_center;
out vec2 f_dst_pos;
out vec2 f_src_pos;
out float f_softness;
out float f_border_thickness;
out float f_omit_texture;

uniform mat4 projection;
uniform mat4 transform;

void
main()
{
  vec2 vertices[4];

  vertices[0] = vec2(-1.0, -1.0);
  vertices[1] = vec2(+1.0, -1.0);
  vertices[2] = vec2(-1.0, +1.0);
  vertices[3] = vec2(+1.0, +1.0);

  vec2 dst_center = 0.5 * (v_dst_max + v_dst_min);
  vec2 dst_half_size = 0.5f * (v_dst_max - v_dst_min);
  vec2 dst_pos = dst_center + dst_half_size * vertices[gl_VertexID];

  vec2 src_center = 0.5 * (v_src_max + v_src_min);
  vec2 src_half_size = 0.5 * (v_src_max - v_src_min);
  vec2 src_pos = src_center + src_half_size * vertices[gl_VertexID];

  vec2 rect_c_verts_pct[4];
  rect_c_verts_pct[0] = vec2(0, 0);
  rect_c_verts_pct[1] = vec2(1, 0);
  rect_c_verts_pct[2] = vec2(0, 1);
  rect_c_verts_pct[3] = vec2(1, 1);

  gl_Position = projection * vec4(dst_pos, 0, 1.0);
  f_dst_pos = dst_pos;
  f_dst_half_size = dst_half_size;
  f_dst_center = dst_center;
  f_src_pos = src_pos;
  f_color = v_colors[gl_VertexID];
  f_corner_radius_coord = rect_c_verts_pct[gl_VertexID];
  f_corner_radius = v_corner_radius[gl_VertexID];
  f_softness = v_extra.x;
  f_border_thickness = v_extra.y;
  f_omit_texture = v_extra.z;
}