#ifndef FONT_DWRITE_MAP_TEXT_TO_GLYPHS_H
#define FONT_DWRITE_MAP_TEXT_TO_GLYPHS_H

#pragma warning(push, 0)
#include <dwrite_3.h>
#pragma warning(pop)

struct F_DWrite_GlyphArrayChunk;

struct F_DWrite_GlyphArray
{
  U64 count;
  U16 *indices;
  F32 *advances;
  DWRITE_GLYPH_OFFSET *offsets;
};

struct F_DWrite_GlyphArrayChunk
{
  F_DWrite_GlyphArrayChunk *next;
  F_DWrite_GlyphArrayChunk *prev;
  U64 total_glyph_count;
  U64 count;
  F_DWrite_GlyphArray v[512];
};

struct F_DWrite_TextToGlyphsSegment
{
  // per segment data
  IDWriteFontFace5 *font_face;
  uint32_t bidi_level;
  F32 font_size_em;
  U64 glyph_count;

  // per glyph data
  U16 *glyph_indices;
  F32 *glyph_advances;
  DWRITE_GLYPH_OFFSET *glyph_offsets;
};

struct F_DWrite_TextToGlyphsSegmentNode
{
  F_DWrite_TextToGlyphsSegmentNode *next;
  F_DWrite_TextToGlyphsSegmentNode *prev;
  F_DWrite_TextToGlyphsSegment v;
};

struct F_DWrite_MapTextToGlyphsResult
{
  // NOTE(hampus): One of these segments for every time the fallback font doesn't match the
  // previous one. For example, if a font contained all the characters, there would just be
  // one segment in the list.

  F_DWrite_TextToGlyphsSegmentNode *first_segment;
  F_DWrite_TextToGlyphsSegmentNode *last_segment;
};

struct TextAnalysisSinkResult
{
  U32 text_position;
  U32 text_length;
  DWRITE_SCRIPT_ANALYSIS analysis;
  U32 resolved_bidi_level;
  U32 explicit_bidi_level;
};

struct TextAnalysisSinkResultChunk
{
  TextAnalysisSinkResultChunk *next;
  TextAnalysisSinkResultChunk *prev;
  U64 count;
  TextAnalysisSinkResult v[512];
};

struct F_DWrite_MapTextToGlyphsState
{
  TextAnalysisSinkResultChunk *first_text_analsys_sink_chunk;
  Arena *arena;
};

static F_DWrite_MapTextToGlyphsResult f_dwrite_map_text_to_glyphs(Arena *arena, IDWriteFontFallback1 *font_fallback, IDWriteFontCollection *font_collection, IDWriteTextAnalyzer1 *text_analyzer, const wchar_t *locale, const wchar_t *base_family, const F32 font_size, const wchar_t *text, const U32 text_length);

#endif