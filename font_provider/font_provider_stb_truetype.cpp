#pragma warning(push, 0)
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "third_party/stb/stb_truetype.h"
#pragma warning(pop)

// TODO(hampus): Caching of indices for codepoints
// TODO(hampus): Caching of scaling

static FP_STBTT_State *fp_stbtt_state;

static void
fp_init()
{
  Arena *arena = arena_alloc();
  fp_stbtt_state = push_array<FP_STBTT_State>(arena, 1);
  fp_stbtt_state->arena = arena;
  fp_stbtt_state->read_file = os_file_read;
}

static void
fp_set_file_read_proc(FP_FileReadProc *proc)
{
  fp_stbtt_state->read_file = proc;
}

static FP_Handle
fp_font_open_file(Arena *arena, String8 path)
{
  FP_Handle result = fp_handle_zero();
  String8 file_read_result = fp_stbtt_state->read_file(arena, path);
  if(file_read_result.size != 0)
  {
    result = fp_font_open_static_data_string(arena, &file_read_result);
  }
  return result;
}

static FP_Handle
fp_font_open_static_data_string(Arena *arena, String8 *data_ptr)
{
  FP_Handle result = fp_handle_zero();
  stbtt_fontinfo *stb_font = push_array<stbtt_fontinfo>(arena, 1);
  stbtt_InitFont(stb_font, data_ptr->data, 0);
  result.u64[0] = int_from_ptr(stb_font);
  return result;
}

static void
fp_font_close(FP_Handle font)
{
}

static FP_RasterResult
fp_raster(Arena *arena, FP_Handle font, U32 size, U32 cp)
{
  profile_function();

  FP_RasterResult result = {};
  stbtt_fontinfo *stb_font = (stbtt_fontinfo *)ptr_from_int(font.u64[0]);

  F32 scale = stbtt_ScaleForMappingEmToPixels(stb_font, (F32)size * 1.33f);

  U64 stb_glyph_idx = (U64)stbtt_FindGlyphIndex(stb_font, safe_s32_from_u64(cp));

  // hampus: Get dimensions of the bitmap

  RectS32 bitmap_rect = {};
  stbtt_GetGlyphBitmapBox(stb_font,
                          safe_s32_from_u64(stb_glyph_idx),
                          scale, scale,
                          &bitmap_rect.x0, &bitmap_rect.y0,
                          &bitmap_rect.x1, &bitmap_rect.y1);
  Vec2U64 bitmap_dim = v2u64((U64)(bitmap_rect.x1 - bitmap_rect.x0), (U64)(bitmap_rect.y1 - bitmap_rect.y0));

  U64 bitmap_size = bitmap_dim.x * bitmap_dim.y;

  // hampus: Rasterize

  if(cp == 101 && size == 14)
  {
    os_print_debug_string("%" PRIU32 ", Bitmap size: %" PRIU64 ", %" PRIU64 "\n", size, bitmap_dim.x, bitmap_dim.y);
  }

  TempArena scratch = get_scratch(0, 0);
  U8 *bitmap_memory = push_array_no_zero<U8>(arena, bitmap_size);
  stbtt_MakeGlyphBitmap(stb_font,
                        bitmap_memory,
                        safe_s32_from_u64(bitmap_dim.x),
                        safe_s32_from_u64(bitmap_dim.y),
                        safe_s32_from_u64(bitmap_dim.x),
                        scale, scale,
                        safe_s32_from_u64(stb_glyph_idx));

  FP_FontMetrics font_metrics = fp_get_font_metrics(font, size);

  S32 advance = 0;
  S32 left_side_bearing = 0;
  stbtt_GetCodepointHMetrics(stb_font, safe_s32_from_u64(cp), &advance, &left_side_bearing);
  RectS32 glyph_bounding_box = {};
  stbtt_GetCodepointBox(stb_font, safe_s32_from_u64(cp), &glyph_bounding_box.x0, &glyph_bounding_box.y0, &glyph_bounding_box.x1, &glyph_bounding_box.y1);

  result.dim = v2u64(bitmap_dim.x, (U64)(font_metrics.ascent + font_metrics.descent));
  result.memory = (void *)push_array<U8>(arena, result.dim.x * result.dim.y * 4);
  result.metrics.left_bearing = floor_f32((F32)left_side_bearing * scale);
  result.metrics.advance = round_f32((F32)advance * scale);

  U64 dst_pitch = result.dim.x * 4;
  U8 *dst = (U8 *)result.memory + (U64)((S32)font_metrics.ascent + bitmap_rect.y0) * dst_pitch;
  U8 *src = (U8 *)bitmap_memory;
  for(U64 y = 0; y < bitmap_dim.y; ++y)
  {
    for(U64 x = 0; x < bitmap_dim.x; ++x)
    {
      *dst++ = 0xff;
      *dst++ = 0xff;
      *dst++ = 0xff;
      *dst++ = *src++;
    }
  }
  return result;
}

static FP_FontMetrics
fp_get_font_metrics(FP_Handle font, U32 size)
{
  FP_FontMetrics result = {};
  stbtt_fontinfo *stb_font = (stbtt_fontinfo *)ptr_from_int(font.u64[0]);
  F32 scale = stbtt_ScaleForMappingEmToPixels(stb_font, (F32)size * 1.33f);
  S32 ascent = 0;
  S32 descent = 0;
  S32 line_gap = 0;
  stbtt_GetFontVMetrics(stb_font, &ascent, &descent, &line_gap);
  result.ascent = floor_f32((F32)ascent * scale);
  // NOTE(hampus): stbtt uses negative values for the descent, hence the negation to get positive.
  result.descent = floor_f32((F32)-descent * scale);
  result.line_gap = floor_f32((F32)line_gap * scale);
  return result;
}

static FP_GlyphMetrics
fp_get_glyph_metrics(FP_Handle font, U32 size, U32 cp)
{
  FP_GlyphMetrics result = {};
  stbtt_fontinfo *stb_font = (stbtt_fontinfo *)ptr_from_int(font.u64[0]);
  F32 scale = stbtt_ScaleForMappingEmToPixels(stb_font, (F32)size * 1.33f);

  return result;
}