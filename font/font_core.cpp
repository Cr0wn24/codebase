function void
f_init(void)
{
  Arena *arena = arena_alloc();
  f_state = push_array<F_State>(arena, 1);
  f_state->arena = arena;
  f_state->atlas = atlas_make(arena, v2u64(4096, 4096));
}

function void
F_Destroy(void)
{
  arena_free(f_state->arena);
  f_state = 0;
}

function F_Handle
f_handle_zero(void)
{
  F_Handle result = {};
  return result;
}

function B32
f_handle_match(F_Handle a, F_Handle b)
{
  B32 result = memory_match(&a, &b, sizeof(F_Handle));
  return result;
}

function B32
f_tag_match(F_Tag a, F_Tag b)
{
  B32 result = str8_match(a.name, b.name);
  return result;
}

function FP_Handle
fp_handle_from_tag(F_Tag tag)
{
  FP_Handle result = fp_handle_zero();
  U64 cold_idx = U64_MAX;

  // hampus: Cache lookup

  for(U64 i = 0; i < array_count(f_state->tag_table); ++i)
  {
    if(fp_handle_match(f_state->fp_table[i], fp_handle_zero()) &&
       cold_idx == U64_MAX)
    {
      cold_idx = i;
    }
    else if(f_tag_match(f_state->tag_table[i], tag))
    {
      result = f_state->fp_table[i];
    }
  }

  // hampus: Cache miss

  if(fp_handle_match(result, fp_handle_zero()))
  {
    if(cold_idx != U64_MAX)
    {
      FP_Handle fp_handle = {};
      if(tag.data.size == 0)
      {
        fp_handle = fp_font_open_file(f_state->arena, tag.name);
      }
      else
      {
        fp_handle = fp_font_open_memory(f_state->arena, tag.data);
      }
      ASSERT(!fp_handle_match(fp_handle, fp_handle_zero()));

      // TODO(hampus): Close the old font handle
      f_state->tag_table[cold_idx] = tag;
      f_state->fp_table[cold_idx] = fp_handle;

      result = fp_handle;
    }
    else
    {
      // TODO(hampus): What to do here? Cache is hot and full
      invalid_code_path;
    }
  }

  return result;
}

function F_Glyph *
f_glyph_from_tag_size_cp(F_Tag tag, U32 size, U32 cp)
{
  TempArena scratch = get_scratch(0, 0);
  FP_Handle fp_handle = fp_handle_from_tag(tag);
  ASSERT(!fp_handle_match(fp_handle, fp_handle_zero()));
  U64 slot_idx = cp % array_count(f_state->glyph_lookup_table);
  F_Glyph *glyph_node = f_state->glyph_lookup_table[slot_idx];
  while(glyph_node)
  {
    if(glyph_node->cp == cp && glyph_node->size == size && f_tag_match(glyph_node->tag, tag))
    {
      break;
    }
    glyph_node = glyph_node->hash_next;
  }

  // hampus: Cache miss

  if(glyph_node == 0)
  {
    glyph_node = push_array<F_Glyph>(f_state->arena, 1);
    glyph_node->cp = cp;
    glyph_node->size = size;
    glyph_node->tag = tag;

    TempArena bitmap_memory_arena = TempArena(scratch.arena);

    FP_RasterResult raster_result = fp_raster(bitmap_memory_arena.arena, fp_handle, size, cp);

    Atlas *atlas = &f_state->atlas;
    if(raster_result.dim.x != 0 && raster_result.dim.y != 0)
    {
      // hampus: Allocate atlas region

      RectU64 atlas_region = atlas_region_alloc(f_state->arena, atlas, raster_result.dim);
      U64 atlas_pitch = atlas->dim.width * 4;
      U64 atlas_row_offset = atlas_region.min.x * 4;
      void *atlas_memory = (U8 *)atlas->memory + atlas_row_offset + atlas_region.min.y * atlas_pitch;

      // hampus: Rasterize from 8-bit to 32-bit into atlas region

      U8 *dst_row = (U8 *)atlas_memory;
      U8 *src = (U8 *)raster_result.memory;
      for(U64 y = 0; y < raster_result.dim.y; ++y)
      {
        U8 *dst = dst_row;
        for(U64 x = 0; x < raster_result.dim.x; ++x)
        {
          *dst++ = 0xff;
          *dst++ = 0xff;
          *dst++ = 0xff;
          *dst++ = *src++;
        }
        dst_row += atlas_pitch;
      }
      glyph_node->region_uv.min = v2f32((F32)atlas_region.min.x / (F32)atlas->dim.x,
                                        (F32)atlas_region.min.y / (F32)atlas->dim.y);
      glyph_node->region_uv.max = v2f32(((F32)atlas_region.min.x + (F32)raster_result.dim.x) / (F32)atlas->dim.x,
                                        ((F32)atlas_region.min.y + (F32)raster_result.dim.y) / (F32)atlas->dim.y);
    }

    sll_stack_push_n(f_state->glyph_lookup_table[slot_idx], glyph_node, hash_next);

    // hampus: Fill cache glyph node

    FP_Metrics metrics = f_metrics_from_font_size_cp(fp_handle, size, cp);
    glyph_node->bearing = metrics.bearing;
    glyph_node->advance = metrics.advance;
    glyph_node->bitmap_size = v2f32((F32)raster_result.dim.x, (F32)raster_result.dim.y);

    f_state->atlas_texture_dirty = true;
  }
  return (glyph_node);
}

function F_GlyphRun
f_glyph_run_from_tag_size_str32(Arena *arena, F_Tag tag, U32 size, String32 str32)
{
  TempArena scratch = get_scratch(&arena, 1);
  F_GlyphRun result = {};
  FP_Handle fp_handle = fp_handle_from_tag(tag);
  ASSERT(!fp_handle_match(fp_handle, fp_handle_zero()));

  // TODO(hampus): Kerning
  for(U64 glyph_idx = 0; glyph_idx < str32.size; ++glyph_idx)
  {
    U32 cp = str32.data[glyph_idx];

    F_Glyph *glyph_node = f_glyph_from_tag_size_cp(tag, size, cp);

    // hampus: Fill glyph run node

    F_GlyphRunNode *run_node = push_array<F_GlyphRunNode>(arena, 1);
    run_node->bearing = glyph_node->bearing;
    run_node->bitmap_size = glyph_node->bitmap_size;
    run_node->region_uv = glyph_node->region_uv;
    run_node->advance = glyph_node->advance;
    dll_push_back(result.first, result.last, run_node);
  }

  return result;
}

function F_GlyphRun
f_glyph_run_from_tag_size_str8(Arena *arena, F_Tag tag, U32 size, String8 string)
{
  TempArena scratch = get_scratch(&arena, 1);
  String32 str32 = str32_from_str8(scratch.arena, string);
  F_GlyphRun result = f_glyph_run_from_tag_size_str32(arena, tag, size, str32);
  return result;
}

function F32
f_advance_from_tag_size_str32(F_Tag tag, U32 size, String32 string)
{
  F32 result = {};
  TempArena scratch = get_scratch(0, 0);
  F_GlyphRun glyph_run = f_glyph_run_from_tag_size_str32(scratch.arena, tag, size, string);
  for(F_GlyphRunNode *n = glyph_run.first; n != 0; n = n->next)
  {
    result += n->advance;
  }
  return result;
}

function F32
f_advance_from_tag_size_string(F_Tag tag, U32 size, String8 string)
{
  F32 result = {};
  TempArena scratch = get_scratch(0, 0);
  F_GlyphRun glyph_run = f_glyph_run_from_tag_size_str8(scratch.arena, tag, size, string);
  for(F_GlyphRunNode *n = glyph_run.first; n != 0; n = n->next)
  {
    result += n->advance;
  }
  return result;
}

function F32
f_advance_from_tag_size_cp(F_Tag tag, U32 size, U32 cp)
{
  F32 result = {};
  F_Glyph *glyph = f_glyph_from_tag_size_cp(tag, size, cp);
  result = glyph->advance;
  return result;
}

function F32
f_line_height_from_tag_size(F_Tag tag, U32 size)
{
  F32 result = 0;
  FP_Handle fp_handle = fp_handle_from_tag(tag);
  ASSERT(!fp_handle_match(fp_handle, fp_handle_zero()));
  FP_Metrics metrics = fp_metrics_from_font_size(fp_handle, size);
  result = metrics.line_height;
  return result;
}

function F32
f_descent_from_tag_size(F_Tag tag, U32 size)
{
  F32 result = 0;
  FP_Handle fp_handle = fp_handle_from_tag(tag);
  ASSERT(!fp_handle_match(fp_handle, fp_handle_zero()));
  FP_Metrics metrics = fp_metrics_from_font_size(fp_handle, size);
  result = metrics.descent;
  return result;
}

function F32
f_max_height_from_tag_size_string(F_Tag tag, U32 size, String8 string)
{
  F32 result = 0;
  TempArena scratch = get_scratch(0, 0);
  F_GlyphRun glyph_run = f_glyph_run_from_tag_size_str8(scratch.arena, tag, size, string);
  for(F_GlyphRunNode *n = glyph_run.first; n != 0; n = n->next)
  {
    result = max(result, n->bitmap_size.y);
  }
  return result;
}

function B32
f_atlas_region_is_dirty(void)
{
  B32 result = f_state->atlas_texture_dirty;
  return result;
}

function Atlas *
f_atlas(void)
{
  Atlas *result = &f_state->atlas;
  return result;
}
