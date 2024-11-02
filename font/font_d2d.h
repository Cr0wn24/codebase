#ifndef FONT_D2D_H
#define FONT_D2D_H

#include <d2d1_3.h>

struct F_Handle
{
  U64 u64[1];
};

struct F_Tag
{
  String8 path;
};

struct F_Glyph
{
  F_Glyph *hash_next;
  U32 idx;
  U32 size;
  IDWriteFontFace5 *font_face;

  RectF32 region_uv;
  FP_GlyphMetrics metrics;
  Vec2U64 bitmap_size;
};

struct F_GlyphRunNode
{
  F_GlyphRunNode *next;
  F_GlyphRunNode *prev;

  RectF32 region_uv;
  FP_GlyphMetrics metrics;
  Vec2U64 bitmap_size;
};

struct F_GlyphRun
{
  F_GlyphRunNode *first;
  F_GlyphRunNode *last;
};

struct F_D2D_State
{
  Arena *arena;

  StaticArray<F_Glyph *, 256> glyph_from_idx_lookup_table;

  wchar_t locale[LOCALE_NAME_MAX_LENGTH];

  IDWriteFactory4 *dwrite_factory;
  IDWriteFontFallback *font_fallback;
  IDWriteFontFallback1 *font_fallback1;
  IDWriteFontCollection *font_collection;
  IDWriteTextAnalyzer *text_analyzer;
  IDWriteTextAnalyzer1 *text_analyzer1;

  ID2D1Factory5 *d2d_factory;
  ID2D1Device4 *d2d_device;
  ID2D1DeviceContext4 *d2d_device_context;
  ID2D1BitmapRenderTarget *d2d_bitmap_render_target;
};

#endif // FONT_D2D_H