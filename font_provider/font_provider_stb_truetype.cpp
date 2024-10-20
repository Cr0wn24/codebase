#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "third_party/stb/stb_truetype.h"

// TODO(hampus): Caching of indices for codepoints
// TODO(hampus): Caching of scaling

function FP_Handle
fp_font_open_file(Arena *arena, String8 path)
{
  FP_Handle result = fp_handle_zero();
  String8 file_read_result = fp_state->read_file(arena, path);
  if(file_read_result.size != 0)
  {
    result = fp_font_open_memory(arena, file_read_result);
  }
  return result;
}

function FP_Handle
fp_font_open_memory(Arena *arena, String8 memory)
{
  FP_Handle result = fp_handle_zero();
  stbtt_fontinfo *stb_font = push_array<stbtt_fontinfo>(arena, 1);
  stbtt_InitFont(stb_font, memory.data, 0);
  result.u64[0] = int_from_ptr(stb_font);
  return result;
}

function void
fp_font_close(FP_Handle font)
{
}

function FP_RasterResult
fp_raster(Arena *arena, FP_Handle font, U32 size, U32 cp)
{
  FP_RasterResult result = {};
  stbtt_fontinfo *stb_font = (stbtt_fontinfo *)ptr_from_int(font.u64[0]);

  F32 scale = stbtt_ScaleForMappingEmToPixels(stb_font, size * 1.33f);

  U64 stb_glyph_idx = stbtt_FindGlyphIndex(stb_font, safe_s32_from_u64(cp));

  // hampus: Get dimensions of the bitmap

  RectS32 bitmap_rect = {};
  stbtt_GetGlyphBitmapBox(stb_font,
                          safe_s32_from_u64(stb_glyph_idx),
                          scale, scale,
                          &bitmap_rect.x0, &bitmap_rect.y0,
                          &bitmap_rect.x1, &bitmap_rect.y1);
  Vec2U64 bitmap_dim = v2u64(bitmap_rect.x1 - bitmap_rect.x0, bitmap_rect.y1 - bitmap_rect.y0);

  U64 bitmap_size = bitmap_dim.x * bitmap_dim.y;

  // hampus: Rasterize

  U8 *bitmap_memory = push_array_no_zero<U8>(arena, bitmap_size);
  stbtt_MakeGlyphBitmap(stb_font,
                        bitmap_memory,
                        safe_s32_from_u64(bitmap_dim.x),
                        safe_s32_from_u64(bitmap_dim.y),
                        safe_s32_from_u64(bitmap_dim.x),
                        scale, scale,
                        safe_s32_from_u64(stb_glyph_idx));
  result.memory = bitmap_memory;
  result.dim = bitmap_dim;
  return result;
}

function FP_Metrics
fp_metrics_from_font_size(FP_Handle font, U32 size)
{
  FP_Metrics result = {};
  stbtt_fontinfo *stb_font = (stbtt_fontinfo *)ptr_from_int(font.u64[0]);
  F32 scale = stbtt_ScaleForMappingEmToPixels(stb_font, size * 1.33f);
  int ascent = 0;
  int descent = 0;
  int line_gap = 0;
  stbtt_GetFontVMetrics(stb_font, &ascent, &descent, &line_gap);
  result.ascent = floor_f32(ascent * scale);
  result.descent = floor_f32(descent * scale);
  result.line_height = floor_f32((ascent - descent + line_gap) * scale);
  return result;
}

function FP_Metrics
f_metrics_from_font_size_cp(FP_Handle font, U32 size, U32 cp)
{
  FP_Metrics result = {};
  stbtt_fontinfo *stb_font = (stbtt_fontinfo *)ptr_from_int(font.u64[0]);
  F32 scale = stbtt_ScaleForMappingEmToPixels(stb_font, size * 1.33f);
  int advance = 0;
  int left_side_bearing = 0;
  stbtt_GetCodepointHMetrics(stb_font, safe_s32_from_u64(cp), &advance, &left_side_bearing);
  RectS32 glyph_bounding_box = {};
  stbtt_GetCodepointBox(stb_font, safe_s32_from_u64(cp), &glyph_bounding_box.x0, &glyph_bounding_box.y0, &glyph_bounding_box.x1, &glyph_bounding_box.y1);
  result.advance = round_f32(advance * scale);
  result.bearing.x = floor_f32(left_side_bearing * scale);
  result.bearing.y = floor_f32(glyph_bounding_box.y1 * scale);
  return result;
}