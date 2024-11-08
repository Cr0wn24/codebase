#ifndef FONT_D2D_H
#define FONT_D2D_H

#pragma warning(push, 0)
#include <d2d1_3.h>
#pragma warning(pop)

struct F_Handle
{
 U64 u64[1];
};

struct F_Tag
{
 String8 string;
};

struct F_FontMetrics
{
 F32 descent;
 F32 ascent;
 F32 line_gap;
};

struct F_GlyphMetrics
{
 F32 advance;
 F32 left_bearing;
};

struct F_GlyphNode
{
 F_GlyphNode *next;
 F_GlyphNode *prev;

 // hampus: rasterization parameters
 U16 idx;
 U32 size;
 IDWriteFontFace *font_face;

 // hampus: layouting
 RectF32 region_uv;
 F_GlyphMetrics metrics;
 Vec2U64 bitmap_size;
};

struct F_GlyphRunNode
{
 F_GlyphRunNode *next;
 F_GlyphRunNode *prev;

 RectF32 region_uv;
 F_GlyphMetrics metrics;
 Vec2U64 bitmap_size;
};

struct F_GlyphRun
{
 F_GlyphRunNode *first;
 F_GlyphRunNode *last;
};

struct F_Atlas
{
 Atlas atlas;
 R_Handle handle;
};

struct F_DWrite_Font
{
 IDWriteFont *font;
 IDWriteFontFace *font_face;
 IDWriteFontFile *font_file;
};

struct F_GlyphSlot
{
 F_GlyphNode *first;
 F_GlyphNode *last;
};

struct F_D2D_State
{
 Arena *arena;

 StaticArray<F_Handle, 6> dwrite_font_table;
 StaticArray<F_Tag, 6> font_tag_table;

 StaticArray<F_GlyphSlot, 256> glyph_slots;
 F_Atlas atlas;

 wchar_t locale[LOCALE_NAME_MAX_LENGTH];

 IDWriteRenderingParams *rendering_params;

 IDWriteFactory7 *dwrite_factory;
 IDWriteFontFallback *font_fallback;
 IDWriteFontFallback1 *font_fallback1;
 IDWriteFontCollection *font_collection;
 IDWriteTextAnalyzer *text_analyzer;
 IDWriteTextAnalyzer1 *text_analyzer1;

 ID2D1Factory5 *d2d_factory;
 ID2D1Device4 *d2d_device;
 ID2D1DeviceContext4 *d2d_device_context;
 ID2D1RenderTarget *d2d_render_target;
 ID2D1SolidColorBrush *foreground_brush;
};

[[nodiscard]] static F_Handle f_handle_zero();
[[nodiscard]] static B32 f_handle_match(F_Handle a, F_Handle b);

[[nodiscard]] static B32 f_tag_match(F_Tag a, F_Tag b);
[[nodiscard]] static F_Tag f_make_tag(String8 string);

[[nodiscard]] static F_Handle f_handle_from_tag(F_Tag tag);

[[nodiscard]] static F_Handle f_open_font(Arena *arena, String8 name);
[[nodiscard]] static F_Handle f_open_font_file(Arena *arena, String8 path);

[[nodiscard]] static F_GlyphRun f_make_simple_glyph_run(Arena *arena, F_Tag tag, U32 size, String32 str32);
[[nodiscard]] static F_GlyphRun f_make_simple_glyph_run(Arena *arena, F_Tag tag, U32 size, String8 string);
[[nodiscard]] static F_GlyphRun f_make_complex_glyph_run(Arena *arena, F_Tag tag, U32 size, String32 str32);
[[nodiscard]] static F_GlyphRun f_make_complex_glyph_run(Arena *arena, F_Tag tag, U32 size, String8 string);

[[nodiscard]] static F32 f_get_advance(F_Tag tag, U32 size, String32 string);
[[nodiscard]] static F32 f_get_advance(F_Tag tag, U32 size, String8 string);
[[nodiscard]] static F32 f_get_advance(F_Tag tag, U32 size, U32 cp);
[[nodiscard]] static F32 f_line_height_from_tag_size(F_Tag tag, U32 size);

static void f_destroy();

[[nodiscard]] static F_Atlas *f_atlas();

[[nodiscard]] static F_FontMetrics f_get_font_metrics(F_Tag tag, U32 size);

#endif // FONT_D2D_H