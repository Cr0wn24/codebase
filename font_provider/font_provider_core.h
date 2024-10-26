#ifndef FONT_PROVIDER_CORE_H
#define FONT_PROVIDER_CORE_H

struct FP_Handle
{
  U64 u64[1];
};

struct FP_RasterResult
{
  void *memory;
  Vec2U64 dim;
  F32 left_bearing;
};

#define FP_FILE_READ_PROC(name) String8 name(Arena *arena, String8 string)
typedef FP_FILE_READ_PROC(FP_FileReadProc);

struct FP_FontMetrics
{
  F32 descent;
  F32 ascent;
  F32 line_gap;
};

struct FP_GlyphMetrics
{
  F32 advance;
  F32 left_bearing;
};

struct FP_State
{
  Arena *arena;
};

[[nodiscard]] static FP_Handle fp_font_open_file(Arena *arena, String8 path);
[[nodiscard]] static FP_Handle fp_font_open_static_data_string(Arena *arena, String8 *data_ptr);
static void fp_font_close(FP_Handle font);

[[nodiscard]] static B32 fp_handle_match(FP_Handle a, FP_Handle b);
[[nodiscard]] static FP_Handle fp_handle_zero();

static void fp_set_file_read_proc(FP_FileReadProc *proc);

[[nodiscard]] static FP_RasterResult fp_raster(Arena *arena, FP_Handle font, U32 size, U32 cp);

[[nodiscard]] static FP_FontMetrics fp_get_font_metrics(FP_Handle font, U32 size);
[[nodiscard]] static FP_GlyphMetrics fp_get_glyph_metrics(FP_Handle font, U32 size, U32 cp);

static FP_State *fp_state;

#endif