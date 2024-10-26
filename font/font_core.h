#ifndef FONT_CORE_H
#define FONT_CORE_H

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
  U32 cp;
  U32 size;
  F_Tag tag;

  RectF32 region_uv;
  FP_GlyphMetrics metrics;
  Vec2U64 bitmap_size;
};

struct F_BakedFont
{
  F_Glyph *glyph_lookup_table[128];
  FP_FontMetrics metrics;
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

struct F_Atlas
{
  Atlas atlas;
  R_Handle handle;
};

struct F_State
{
  Arena *arena;

  // TODO(hampus): Make this more dynamic.
  StaticArray<F_Tag, 6> tag_table;
  StaticArray<FP_Handle, 6> fp_table;

  // TODO(hampus): Be able to free from this table.
  // Maybe have this table per font tag.
  // TODO(hampus): Think about pulling the keys (cp, size, tag) to
  // their own table and then have the data in its own table which
  // the key references. For better cache locality when doing
  // the lookup
  StaticArray<F_Glyph *, 128> glyph_lookup_table;
  F_Atlas atlas;
};

static void f_init();

[[nodiscard]] static F_Handle f_handle_zero();
[[nodiscard]] static B32 f_handle_match(F_Handle a, F_Handle b);

[[nodiscard]] static B32 f_tag_match(F_Tag a, F_Tag b);

[[nodiscard]] static FP_Handle fp_handle_from_tag(F_Tag tag);
[[nodiscard]] static F_Glyph *f_glyph_from_tag_size_cp(F_Tag tag, U32 size, U32 cp);
[[nodiscard]] static F_GlyphRun f_make_glyph_run(Arena *arena, F_Tag tag, U32 size, String32 str32);
[[nodiscard]] static F_GlyphRun f_make_glyph_run(Arena *arena, F_Tag tag, U32 size, String8 string);

[[nodiscard]] static F32 f_get_advance(F_Tag tag, U32 size, String32 string);
[[nodiscard]] static F32 f_get_advance(F_Tag tag, U32 size, String8 string);
[[nodiscard]] static F32 f_get_advance(F_Tag tag, U32 size, U32 cp);
[[nodiscard]] static F32 f_line_height_from_tag_size(F_Tag tag, U32 size);
[[nodiscard]] static F32 f_descent_from_tag_size(F_Tag tag, U32 size);
[[nodiscard]] static F32 f_max_height_from_tag_size_string(F_Tag tag, U32 size, String8 string);

[[nodiscard]] static F_Atlas *f_atlas();

static F_State *f_state;

#endif // FONT_CORE_H
