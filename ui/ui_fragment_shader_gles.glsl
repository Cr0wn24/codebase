#version 300 es

precision highp float;
in vec4 f_color;
in float f_corner_radius;
in vec2 f_corner_radius_coord;
in vec2 f_dst_half_size;
in vec2 f_dst_center;
in vec2 f_dst_pos;
in vec2 f_src_pos;
in float f_softness;
in float f_border_thickness;
in float f_omit_texture;

out vec4 FragColor;

uniform sampler2D sampler;

float
rounded_rect_sdf(vec2 sample_pos, vec2 rect_center, vec2 rect_half_size, float r)
{
 vec2 d2 = (abs(rect_center - sample_pos) - rect_half_size + vec2(r, r));
 return min(max(d2.x, d2.y), 0.0) + length(max(d2, 0.0)) - r;
}

void
main()
{
 vec2 softness_padding = vec2(max(0.0, f_softness * 2.0 - 1.0), max(0.0, f_softness * 2.0 - 1.0));

 float dist = rounded_rect_sdf(f_dst_pos, f_dst_center, f_dst_half_size - softness_padding, f_corner_radius);

 float sdf_factor = 1.f - smoothstep(0.0f, 2.0f * f_softness, dist);

 float border_factor = 1.0f;
 if(f_border_thickness != 0.0f)
 {
  float inside_d = rounded_rect_sdf(f_dst_pos, f_dst_center, f_dst_half_size - softness_padding - vec2(f_border_thickness, f_border_thickness), max(f_corner_radius - f_border_thickness, 0.0f));
  border_factor = smoothstep(0.0f, 2.0f * f_softness, inside_d);
 }

 vec4 sample_color = vec4(1, 1, 1, 1);
 if(f_omit_texture < 1.0f)
 {
  sample_color = texture(sampler, f_src_pos);
 }
 vec4 color = sample_color;
 color *= f_color;
 color.a *= sdf_factor * border_factor;
 FragColor = color;
}