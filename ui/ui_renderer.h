#ifndef UI_RENDERER_H
#define UI_RENDERER_H

struct UI_ClipRectNode
{
  UI_ClipRectNode *next;
  RectF32 v;
};

struct UI_DrawRectParams
{
  Vec4F32 color;
  R_Tex2DSlice slice;
  F32 corner_radius;
  F32 softness;
  F32 border_thickness;
};

struct UI_RectInstance
{
  Vec2F32 dst_min;
  Vec2F32 dst_max;
  Vec2F32 src_min;
  Vec2F32 src_max;
  Vec4F32 colors[Corner_COUNT];
  Vec4F32 corner_radius;
  Vec4F32 extra;
};

struct UI_RectBatchParams
{
  R_Handle texture;
  UI_ClipRectNode *clip_rect_node;
};

struct UI_RectBatch
{
  UI_RectInstance *base;
  U64 count;
  U64 max_count;
  UI_RectBatchParams params;
};

struct UI_RectBatchNode
{
  UI_RectBatchNode *next;
  UI_RectBatchNode *prev;
  UI_RectBatch *v;
};

struct UI_RendererState
{
  UI_RectBatchNode *first_batch_node;
  UI_RectBatchNode *last_batch_node;

  R_Handle pipeline;
  R_Handle vertex_buffer;
  R_Handle uniform_buffer;
  R_Handle white_texture;

  UI_ClipRectNode *clip_rect_stack;
};

static void ui_renderer_init();
static void ui_renderer_destroy();
static UI_RectInstance *ui_draw_rect(Vec2F32 min, Vec2F32 max, UI_DrawRectParams params);
static F32 ui_draw_text(Vec2F32 pos, F_Tag tag, U32 size, String8 string, Vec4F32 color);
static F32 ui_draw_glyph_run(Vec2F32 pos, Vec4F32 color, F_GlyphRun glyph_run);

static void ui_draw();

[[nodiscard]] static RectF32 ui_clip_rect_top();
static void ui_clip_rect_push(RectF32 rect);
static void ui_clip_rect_pop();

#endif // UI_RENDERER_H
