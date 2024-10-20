struct VS_INPUT_UI
{
  float2 min : MIN;
  float2 max : MAX;
  float2 min_uv : MIN_UV;
  float2 max_uv : MAX_UV;
  float4 colors[4] : COLORS;
  float4 corner_radius : CORNER_RADIUS;
  float4 extra : EXTRA; // 0: softness, 1: border thickness, 2: omit_texture
  uint vertex_id : SV_VertexID;
};

struct PS_INPUT_UI
{
  float4 pos : SV_POSITION;
  float2 rect_pos : DST_POS;
  float2 rect_half_size : DST_HALF_SIZE;
  float2 uv : UV;
  float2 corner_radius_coord : CORNER_RADIUS_COORD;
  float corner_radius : CORNER_RADIUS;
  float edge_softness : EDGE_SOFTNESS;
  float border_thickness : BORDER_THICKNESS;
  float omit_texture : OMIT_TEXTURE;
  float4 color : COLOR;
};

cbuffer cbuffer_ui : register(b0)
{
  float4x4 projection;
  row_major float4x4 transform;
}

Texture2D<float4> texture0 : register(t0);

sampler sampler0 : register(s0);

PS_INPUT_UI
vs(VS_INPUT_UI input)
{
  float2 vertices[4];
  vertices[0] = float2(-1, -1);
  vertices[1] = float2(+1, -1);
  vertices[2] = float2(-1, +1);
  vertices[3] = float2(+1, +1);

  float2 min = input.min;
  float2 max = input.max;

  float2 rect_size = max - min;

  float2 points[] =
  {
    float2(min.x, min.y),
    float2(max.x, min.y),
    float2(min.x, max.y),
    float2(max.x, max.y),
  };

  points[0] = mul(transform, float4(points[0], 1, 1)).xy;
  points[1] = mul(transform, float4(points[1], 1, 1)).xy;
  points[2] = mul(transform, float4(points[2], 1, 1)).xy;
  points[3] = mul(transform, float4(points[3], 1, 1)).xy;

  float2 rect_half_size = rect_size / 2;
  float2 rect_pos = points[input.vertex_id];

  float2 src_half_size = (input.max_uv - input.min_uv) / 2;
  float2 src_center = (input.max_uv + input.min_uv) / 2;
  float2 src_pos = (vertices[input.vertex_id] * src_half_size + src_center);

  float2 rect_c_verts_pct[] =
  {
    float2(0, 0),
    float2(1, 0),
    float2(0, 1),
    float2(1, 1),
  };

  PS_INPUT_UI output;
  output.pos = mul(projection, float4(rect_pos, 0, 1));
  output.rect_pos = rect_pos;
  output.rect_half_size = rect_half_size;
  output.uv = src_pos;
  output.color = input.colors[input.vertex_id];
  output.corner_radius_coord = rect_c_verts_pct[input.vertex_id];
  output.corner_radius = input.corner_radius[input.vertex_id];
  output.edge_softness = input.extra.x;
  output.border_thickness = input.extra.y;
  output.omit_texture = input.extra.z;
  return output;
}

float
rounded_rect_sdf(float2 sample_pos, float2 rect_half_size, float r)
{
  return length(max(abs(sample_pos) - rect_half_size + r, 0.0)) - r;
}

float4
ps(PS_INPUT_UI input) : SV_TARGET
{
  float softness = input.edge_softness;

  float2 sdf_sample_pos =
  float2((2 * input.corner_radius_coord.x - 1) * input.rect_half_size.x,
         (2 * input.corner_radius_coord.y - 1) * input.rect_half_size.y);

  float dist = rounded_rect_sdf(sdf_sample_pos,
                                input.rect_half_size -
                                float2(softness * 2.f, softness * 2.f),
                                input.corner_radius);

  float sdf_factor = 1.f - smoothstep(0, 2 * softness, dist);

  float inside_d = rounded_rect_sdf(sdf_sample_pos, 
  input.rect_half_size - float2(softness * 2.f, softness * 2.f) -
                                    input.border_thickness,
                                    max(input.corner_radius - input.border_thickness, 0));

  float border_factor = smoothstep(0, 2 * softness, inside_d);
  if(input.border_thickness == 0)
  {
    border_factor = 1;
  }

  float4 sample_color = float4(1, 1, 1, 1);
  if(input.omit_texture < 1)
  {
    sample_color = texture0.Sample(sampler0, input.uv);
  }

  float4 color = sample_color;
  color *= input.color;
  color.a *= sdf_factor * border_factor;
  return color;
}