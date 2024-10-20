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
};

#define FP_FILE_READ_PROC(name) String8 name(Arena *arena, String8 string)
typedef FP_FILE_READ_PROC(FP_FileReadProc);

struct FP_Metrics
{
  F32 ascent;
  F32 descent;
  F32 line_height;
  F32 advance;
  Vec2F32 bearing;
};

struct FP_State
{
  Arena *arena;
  FP_FileReadProc *read_file;
};

function void fp_init(void);

function FP_Handle fp_font_open_file(Arena *arena, String8 path);
function FP_Handle fp_font_open_memory(Arena *arena, String8 data);
function void fp_font_close(FP_Handle font);

function B32 fp_handle_match(FP_Handle a, FP_Handle b);
function FP_Handle fp_handle_zero(void);

function void fp_set_file_read_proc(FP_FileReadProc *proc);

function FP_RasterResult fp_raster(Arena *arena, FP_Handle font, U32 size, U32 cp);

function FP_Metrics fp_metrics_from_font_size(FP_Handle font, U32 size);
function FP_Metrics f_metrics_from_font_size_cp(FP_Handle font, U32 size, U32 cp);

global FP_State *fp_state;

#endif