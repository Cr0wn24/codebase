#ifndef RENDER_CORE_H
#define RENDER_CORE_H

struct R_Handle
{
  U64 u64[1];
};

struct R_Tex2DSlice
{
  R_Handle tex;
  RectF32 uv;
};

struct R_ShaderDesc
{
  String8 vs_source;
  String8 ps_source;
  String8 vs_entry_point_name;
  String8 ps_entry_point_name;
};

enum R_AttributeKind
{
  R_AttributeKind_Float,
  R_AttributeKind_Float2,
  R_AttributeKind_Float3,
  R_AttributeKind_Float4,
};

enum R_InputSlotClass
{
  R_InputSlotClass_PerVertex,
  R_InputSlotClass_PerInstance,
};

enum R_Topology
{
  R_Topology_TriangleList,
  R_Topology_TriangleStrip,
};

enum R_SampleFilter
{
  R_SampleFilter_Bilinear,
  R_SampleFilter_Nearest,
};

struct R_InputLayoutAttribute
{
  String8 name;
  R_AttributeKind kind;
  U64 offset;
  R_InputSlotClass slot_class;
  U64 semantic_index;
  U64 step_rate;
};

struct R_InputLayoutDesc
{
  Array<R_InputLayoutAttribute, 32> attribs;
  U64 attribs_count;
};

struct R_PipelineDesc
{
  R_ShaderDesc shader;
  R_InputLayoutDesc input_layout;
  R_Topology topology;
  R_SampleFilter sample_filter;
  B32 enable_depth_buffering;
};

enum R_BufferKind
{
  R_BufferKind_Uniform,
  R_BufferKind_Vertex,
};

struct R_BufferDesc
{
  U64 size;
  R_BufferKind kind;
  U64 stride;
};

struct R_FillBufferDesc
{
  String8 data;
};

struct R_AttributeDesc
{
  String8 semantic_name;
  R_AttributeKind kind;
  U64 offset;
  R_InputSlotClass slot_class;
  U64 semantic_index;
  U64 step_rate;
};

function R_Handle r_handle_zero(void);
function B32 r_handle_match(R_Handle a, R_Handle b);

function void r_init(void);
function R_Handle r_make_render_window_context(OS_Handle window);
function void r_destroy_render_window_context(R_Handle handle);
function R_Handle r_make_pipeline(R_PipelineDesc desc);
function void r_destroy_pipeline(R_Handle handle);
function R_Handle r_make_buffer(R_BufferDesc desc);
function void r_destroy_buffer(R_Handle handle);
function R_Handle r_make_tex2d_from_bitmap(void *data, U32 width, U32 height);
function R_Handle r_make_tex2d_from_memory(String8 data);
function void r_destroy_tex2d(R_Handle handle);
function void r_update_tex2d_contents(R_Handle texture, void *memory);

function void r_begin_pass(Vec4F32 clear_color);
function void r_end_pass(void);
function void r_draw(U64 vertex_count);
function void r_instanced_draw(U64 vertex_count_per_instance, U64 instance_count);
function void r_commit(void);

function void r_apply_pipeline(R_Handle pipeline);
function void r_apply_vertex_buffer(R_Handle buffer, U64 size);
function void r_apply_uniform_buffer(R_Handle buffer);
function void r_apply_tex2d(R_Handle tex2d);
function void r_apply_window_context(R_Handle window_context);
function void r_apply_scissor_rect(RectF32 rect);

function void r_fill_buffer(R_Handle buffer, R_FillBufferDesc desc);

function void r_add_input_layout_attribute(R_InputLayoutDesc *input_layout_desc, R_AttributeDesc desc);

#endif // RENDER_CORE_H
