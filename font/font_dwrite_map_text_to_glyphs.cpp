#pragma comment(lib, "dwrite.lib")

struct TextAnalysisSource final : IDWriteTextAnalysisSource
{
  TextAnalysisSource(const wchar_t *locale, const wchar_t *text, const U32 text_length) noexcept
      : _locale{locale},
        _text{text},
        _text_length{text_length}
  {
  }

  ULONG STDMETHODCALLTYPE
  AddRef() noexcept override
  {
    return 1;
  }

  ULONG STDMETHODCALLTYPE
  Release() noexcept override
  {
    return 1;
  }

  HRESULT STDMETHODCALLTYPE
  QueryInterface(const IID &riid, void **ppvObject) noexcept override
  {
    if(IsEqualGUID(riid, __uuidof(IDWriteTextAnalysisSource)))
    {
      *ppvObject = this;
      return S_OK;
    }

    *ppvObject = 0;
    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE
  GetTextAtPosition(U32 text_pos, const WCHAR **text_string, U32 *text_length) noexcept override
  {
    text_pos = min(text_pos, _text_length);
    *text_string = _text + text_pos;
    *text_length = _text_length - text_pos;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE
  GetTextBeforePosition(U32 text_pos, const WCHAR **text_string, U32 *text_length) noexcept override
  {
    text_pos = min(text_pos, _text_length);
    *text_string = _text;
    *text_length = text_pos;
    return S_OK;
  }

  DWRITE_READING_DIRECTION STDMETHODCALLTYPE
  GetParagraphReadingDirection() noexcept override
  {
    return DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
  }

  HRESULT STDMETHODCALLTYPE
  GetLocaleName(U32 text_pos, U32 *text_length, const WCHAR **locale_name) noexcept override
  {
    *text_length = _text_length - text_pos;
    *locale_name = _locale;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE
  GetNumberSubstitution(U32 text_pos, U32 *text_length, IDWriteNumberSubstitution **nunber_substitution) noexcept override
  {
    return E_NOTIMPL;
  }

private:
  const wchar_t *_locale;
  const wchar_t *_text;
  const U32 _text_length;
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

struct TextAnalysisSink final : IDWriteTextAnalysisSink
{
  TextAnalysisSinkResultChunk *first_result_chunk;
  TextAnalysisSinkResultChunk *last_result_chunk;

  ULONG STDMETHODCALLTYPE
  AddRef() noexcept override
  {
    return 1;
  }

  ULONG STDMETHODCALLTYPE
  Release() noexcept override
  {
    return 1;
  }

  HRESULT STDMETHODCALLTYPE
  QueryInterface(const IID &riid, void **object) noexcept override
  {
    if(IsEqualGUID(riid, __uuidof(IDWriteTextAnalysisSink)))
    {
      *object = this;
      return S_OK;
    }

    object = 0;
    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE
  SetScriptAnalysis(U32 text_pos, U32 text_length, const DWRITE_SCRIPT_ANALYSIS *script_analysis) noexcept override
  {
    profile_function();
    TextAnalysisSinkResultChunk *chunk = last_result_chunk;
    if(chunk == 0 || chunk->count == ARRAYSIZE(chunk->v))
    {
      chunk = (TextAnalysisSinkResultChunk *)calloc(1, sizeof(TextAnalysisSinkResultChunk));
      if(first_result_chunk == 0)
      {
        first_result_chunk = last_result_chunk = chunk;
      }
      else
      {
        last_result_chunk->next = chunk;
        last_result_chunk = first_result_chunk;
      }
    }
    TextAnalysisSinkResult &result = chunk->v[chunk->count];
    result.text_position = text_pos;
    result.text_length = text_length;
    result.analysis = *script_analysis;
    chunk->count += 1;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE
  SetLineBreakpoints(U32 text_pos, U32 text_length, const DWRITE_LINE_BREAKPOINT *line_breakpoints) noexcept override
  {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE
  SetBidiLevel(U32 text_pos, U32 text_length, U8 explicit_level, U8 resolved_level) noexcept override
  {
    // TODO(hampus): Is this correct?
    last_result_chunk->v[last_result_chunk->count - 1].explicit_bidi_level = explicit_level;
    last_result_chunk->v[last_result_chunk->count - 1].resolved_bidi_level = resolved_level;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE
  SetNumberSubstitution(U32 text_pos, U32 text_length, IDWriteNumberSubstitution *nunber_substitution) noexcept override
  {
    return E_NOTIMPL;
  }

  ~TextAnalysisSink()
  {
    TextAnalysisSinkResultChunk *next_chunk = 0;
    for(TextAnalysisSinkResultChunk *chunk = first_result_chunk; chunk != 0; chunk = next_chunk)
    {
      next_chunk = chunk->next;
      free(chunk);
      chunk = 0;
    }
  }
};

static F_DWrite_GlyphArray *
allocate_and_push_back_glyph_array(Arena *arena, F_DWrite_GlyphArrayChunk **first_chunk, F_DWrite_GlyphArrayChunk **last_chunk)
{
  profile_function();
  F_DWrite_GlyphArray *glyph_array = 0;
  F_DWrite_GlyphArrayChunk *glyph_array_chunk = *last_chunk;
  {
    if(glyph_array_chunk == 0 || glyph_array_chunk->count == ARRAYSIZE(glyph_array_chunk->v))
    {
      glyph_array_chunk = push_array<F_DWrite_GlyphArrayChunk>(arena, 1);
      if(*first_chunk == 0)
      {
        *first_chunk = *last_chunk = glyph_array_chunk;
      }
      else
      {
        (*last_chunk)->next = glyph_array_chunk;
        glyph_array_chunk->prev = *last_chunk;
        *last_chunk = glyph_array_chunk;
      }
    }
    glyph_array = &glyph_array_chunk->v[glyph_array_chunk->count];
    glyph_array_chunk->count += 1;
  }
  return glyph_array;
}

static F_DWrite_TextToGlyphsSegmentNode *
allocate_and_push_back_segment_node(Arena *arena, F_DWrite_TextToGlyphsSegmentNode **first_segment, F_DWrite_TextToGlyphsSegmentNode **last_segment)
{
  profile_function();
  F_DWrite_TextToGlyphsSegmentNode *segment_node = push_array<F_DWrite_TextToGlyphsSegmentNode>(arena, 1);
  if(*first_segment == 0)
  {
    *first_segment = *last_segment = segment_node;
  }
  else
  {
    (*last_segment)->next = segment_node;
    segment_node->prev = *last_segment;
    *last_segment = segment_node;
  }
  return segment_node;
}

static void
fill_segment_with_glyph_array_chunks(F_DWrite_TextToGlyphsSegment *segment, F_DWrite_GlyphArrayChunk *first_chunk, F_DWrite_GlyphArrayChunk *last_chunk)
{
  profile_function();
  U64 total_glyph_count = 0;
  for(F_DWrite_GlyphArrayChunk *chunk = first_chunk; chunk != 0; chunk = chunk->next)
  {
    total_glyph_count += chunk->total_glyph_count;
  }

  segment->glyph_count = total_glyph_count;
  segment->glyph_indices = (U16 *)calloc(segment->glyph_count, sizeof(U16));
  segment->glyph_advances = (F32 *)calloc(segment->glyph_count, sizeof(F32));
  segment->glyph_offsets = (DWRITE_GLYPH_OFFSET *)calloc(segment->glyph_count, sizeof(DWRITE_GLYPH_OFFSET));
  U64 glyph_idx_offset = 0;
  for(F_DWrite_GlyphArrayChunk *chunk = first_chunk; chunk != 0; chunk = chunk->next)
  {
    for(U64 glyph_array_idx = 0; glyph_array_idx < chunk->count; ++glyph_array_idx)
    {
      F_DWrite_GlyphArray &glyph_array = chunk->v[glyph_array_idx];
      memory_copy_typed(segment->glyph_indices + glyph_idx_offset, glyph_array.indices, glyph_array.count);
      memory_copy_typed(segment->glyph_advances + glyph_idx_offset, glyph_array.advances, glyph_array.count);
      memory_copy_typed(segment->glyph_offsets + glyph_idx_offset, glyph_array.offsets, glyph_array.count);

      glyph_idx_offset += glyph_array.count;
    }
  }
}

static F_DWrite_MapTextToGlyphsResult
f_dwrite_map_text_to_glyphs(Arena *arena, IDWriteFontFallback1 *font_fallback, IDWriteFontCollection *font_collection, IDWriteTextAnalyzer1 *text_analyzer, const wchar_t *locale, const wchar_t *base_family, const F32 font_size, const wchar_t *text, const U32 text_length)
{
  profile_function();

  F_DWrite_MapTextToGlyphsResult result = {};

  HRESULT hr = 0;

  for(U32 fallback_offset = 0; fallback_offset < text_length;)
  {
    TempArena scratch = get_scratch(&arena, 1);

    //----------------------------------------------------------
    // hampus: get mapped font and length

    IDWriteFontFace5 *mapped_font_face = 0;
    U32 mapped_text_length = 0;
    {
      // NOTE(hampus): We need an analysis source that holds the text and the locale
      TextAnalysisSource analysis_source{locale, text, text_length};

      // NOTE(hampus): This get the appropiate font required for rendering the text
      F32 scale = 0;
      {
        profile_scope("MapCharacters()");
        hr = font_fallback->MapCharacters(&analysis_source,
                                          fallback_offset,
                                          text_length,
                                          font_collection,
                                          base_family,
                                          0,
                                          0,
                                          &mapped_text_length,
                                          &scale,
                                          &mapped_font_face);
      }
      ASSERT(SUCCEEDED(hr));
      if(mapped_font_face == 0)
      {
        // NOTE(hampus): This means that no font was available for this character.
        // mapped_text_length is the number of characters to skip.
        // TODO(hampus): Should be replaced by a missing glyph, typically glyph index 0 in fonts
        fallback_offset += mapped_text_length;
        continue;
      }
    }

    // NOTE(hampus): This is a way to get the font face name of the
    // fallback font if you want that.
    {
      IDWriteLocalizedStrings *localized_strings = 0;
      mapped_font_face->GetFamilyNames(&localized_strings);
      WCHAR buffer[512] = {};
      localized_strings->GetString(0, buffer, 512);
      localized_strings->Release();
    }

    //----------------------------------------------------------
    // hampus: get glyph array list with both simple and complex glyphs

    // NOTE(hampus): Each simple and complex text will get their own F_DWrite_GlyphArray.
    // In the end a long contigous array will be allocated and the glyph runs
    // will be memcpy'd into that. That is because DWRITE_GLYPH_RUN expcets
    // one large array of glyph indices. So these chunks are only temporary.

    // TODO(hampus): Is chunking really necessary? Since this memory is just temporary and
    // if we know the upper limits, we could just preallocate F_DWrite_GlyphArray and not having to deal with
    // chunks.
    F_DWrite_TextToGlyphsSegment *segment = 0;
    F_DWrite_GlyphArrayChunk *first_glyph_array_chunk = 0;
    F_DWrite_GlyphArrayChunk *last_glyph_array_chunk = 0;

    const wchar_t *fallback_ptr = text + fallback_offset;
    const wchar_t *fallback_opl = fallback_ptr + mapped_text_length;
    while(fallback_ptr < fallback_opl)
    {
      U32 fallback_remaining = (U32)(fallback_opl - fallback_ptr);

      U32 max_glyph_indices_count = mapped_text_length * 128;
      U16 *glyph_indices = push_array_no_zero<U16>(scratch.arena, max_glyph_indices_count);
      BOOL is_simple = FALSE;
      U32 complex_mapped_length = 0;

      hr = text_analyzer->GetTextComplexity(fallback_ptr,
                                            fallback_remaining,
                                            mapped_font_face,
                                            &is_simple,
                                            &complex_mapped_length,
                                            glyph_indices);
      ASSERT(SUCCEEDED(hr));

      if(is_simple)
      {
        // NOTE(hampus): This text was simple. This means we can just use
        // the indices directly without having to do any more shaping work.

        if(segment != 0)
        {
          if(segment->bidi_level != 0)
          {
            fill_segment_with_glyph_array_chunks(segment, first_glyph_array_chunk, last_glyph_array_chunk);
            first_glyph_array_chunk = 0;
            last_glyph_array_chunk = 0;
            segment = 0;
          }
        }

        if(segment == 0)
        {
          F_DWrite_TextToGlyphsSegmentNode *segment_node = allocate_and_push_back_segment_node(arena, &result.first_segment, &result.last_segment);
          segment = &segment_node->v;
          segment->font_face = mapped_font_face;
          segment->font_size_em = font_size;
        }

        // hampus: get a new glyph array

        F_DWrite_GlyphArray *glyph_array = allocate_and_push_back_glyph_array(scratch.arena, &first_glyph_array_chunk, &last_glyph_array_chunk);

        // hampus: allocate the arrays in the glyph array

        DWRITE_FONT_METRICS1 font_metrics = {};
        mapped_font_face->GetMetrics(&font_metrics);
        glyph_array->count = complex_mapped_length;
        glyph_array->advances = push_array_no_zero<F32>(scratch.arena, glyph_array->count);
        glyph_array->offsets = push_array_no_zero<DWRITE_GLYPH_OFFSET>(scratch.arena, glyph_array->count);
        glyph_array->indices = push_array_no_zero<U16>(scratch.arena, glyph_array->count);

        // hampus: fill in indices

        memory_copy_typed(glyph_array->indices, glyph_indices, glyph_array->count);

        // hampus: fill in advances

        {
          S32 *design_advances = push_array_no_zero<S32>(scratch.arena, glyph_array->count);
          hr = mapped_font_face->GetDesignGlyphAdvances((U32)glyph_array->count, glyph_array->indices, design_advances);
          F32 scale = font_size / (F32)font_metrics.designUnitsPerEm;
          for(U64 idx = 0; idx < glyph_array->count; idx++)
          {
            glyph_array->advances[idx] = (F32)design_advances[idx] * scale;
          }
        }

        last_glyph_array_chunk->total_glyph_count += glyph_array->count;
      }
      else
      {
        // NOTE(hampus): This text was not simple. We have to do extra work. :(

        TextAnalysisSource analysis_source{locale, fallback_ptr, complex_mapped_length};
        TextAnalysisSink analysis_sink = {};

        hr = text_analyzer->AnalyzeScript(&analysis_source, 0, complex_mapped_length, &analysis_sink);
        ASSERT(SUCCEEDED(hr));

        // NOTE(hampus): The text range given to AnalyzeBidi should not split a paragraph.
        // It is meant for one paragraph as a whole or multiple paragraphs.
        // TODO(hampus): What to do about it? What is a paragraph?
        hr = text_analyzer->AnalyzeBidi(&analysis_source, 0, complex_mapped_length, &analysis_sink);
        ASSERT(SUCCEEDED(hr));

        DWRITE_SHAPING_GLYPH_PROPERTIES *glyph_props = push_array_no_zero<DWRITE_SHAPING_GLYPH_PROPERTIES>(scratch.arena, max_glyph_indices_count);

        for(TextAnalysisSinkResultChunk *chunk = analysis_sink.first_result_chunk; chunk != 0; chunk = chunk->next)
        {
          for(U64 text_analysis_sink_result_idx = 0; text_analysis_sink_result_idx < chunk->count; ++text_analysis_sink_result_idx)
          {
            TextAnalysisSinkResult &analysis_result = chunk->v[text_analysis_sink_result_idx];

            if(segment != 0)
            {
              if(segment->bidi_level != analysis_result.resolved_bidi_level)
              {
                fill_segment_with_glyph_array_chunks(segment, first_glyph_array_chunk, last_glyph_array_chunk);
                first_glyph_array_chunk = 0;
                last_glyph_array_chunk = 0;
                segment = 0;
              }
            }

            if(segment == 0)
            {
              F_DWrite_TextToGlyphsSegmentNode *segment_node = allocate_and_push_back_segment_node(arena, &result.first_segment, &result.last_segment);
              segment = &segment_node->v;
              segment->font_face = mapped_font_face;
              segment->font_size_em = font_size;
              segment->bidi_level = analysis_result.resolved_bidi_level;
            }

            U16 *cluster_map = push_array_no_zero<U16>(scratch.arena, analysis_result.text_length);
            DWRITE_SHAPING_TEXT_PROPERTIES *text_props = push_array_no_zero<DWRITE_SHAPING_TEXT_PROPERTIES>(scratch.arena, analysis_result.text_length);

            U32 actual_glyph_count = 0;

            F_DWrite_GlyphArray *glyph_array = allocate_and_push_back_glyph_array(scratch.arena, &first_glyph_array_chunk, &last_glyph_array_chunk);

            BOOL is_right_to_left = (BOOL)(analysis_result.resolved_bidi_level & 1);
            for(int retry = 0;;)
            {
              {
                profile_scope("GetGlyphs()");
                hr = text_analyzer->GetGlyphs(fallback_ptr + analysis_result.text_position,
                                              analysis_result.text_length,
                                              mapped_font_face,
                                              false,
                                              is_right_to_left,
                                              &analysis_result.analysis,
                                              locale,
                                              0,
                                              0,
                                              0,
                                              0,
                                              (U32)max_glyph_indices_count,
                                              cluster_map,
                                              text_props,
                                              glyph_indices,
                                              glyph_props,
                                              &actual_glyph_count);
              }

              if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) && ++retry < 8)
              {
                // TODO(hampus): Test this codepath.
                ASSERT(!"FALSE");
#if 0
                max_glyph_indices_count *= 2;
                glyph_props = (DWRITE_SHAPING_GLYPH_PROPERTIES *)realloc(glyph_props, max_glyph_indices_count * sizeof(DWRITE_SHAPING_GLYPH_PROPERTIES));
#endif
              }

              ASSERT(SUCCEEDED(hr));
              break;
            }

            glyph_array->count = actual_glyph_count;
            glyph_array->indices = push_array_no_zero<U16>(scratch.arena, glyph_array->count);
            glyph_array->advances = push_array_no_zero<F32>(scratch.arena, glyph_array->count);
            glyph_array->offsets = push_array_no_zero<DWRITE_GLYPH_OFFSET>(scratch.arena, glyph_array->count);

            memory_copy_typed(glyph_array->indices, glyph_indices, actual_glyph_count);
            {
              profile_scope("GetGlyphPlacements()")
              hr = text_analyzer->GetGlyphPlacements(fallback_ptr + analysis_result.text_position,
                                                     cluster_map,
                                                     text_props,
                                                     analysis_result.text_length,
                                                     glyph_array->indices,
                                                     glyph_props,
                                                     actual_glyph_count,
                                                     mapped_font_face,
                                                     font_size,
                                                     false,
                                                     is_right_to_left,
                                                     &analysis_result.analysis,
                                                     locale,
                                                     0,
                                                     0,
                                                     0,
                                                     glyph_array->advances,
                                                     glyph_array->offsets);
            }
            ASSERT(SUCCEEDED(hr));

            last_glyph_array_chunk->total_glyph_count += glyph_array->count;
          }
        }
      }

      fallback_ptr += complex_mapped_length;
    }

    //----------------------------------------------------------
    // hampus: convert our list of glyph arrays into one big array

    {
      fill_segment_with_glyph_array_chunks(segment, first_glyph_array_chunk, last_glyph_array_chunk);
    }

    fallback_offset += mapped_text_length;
  }

  return result;
}
