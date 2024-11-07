#pragma comment(lib, "dwrite.lib")

static F_DWrite_MapTextToGlyphsState *f_dwrite_map_text_to_glyphs_state;

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
    text_pos = Min(text_pos, _text_length);
    *text_string = _text + text_pos;
    *text_length = _text_length - text_pos;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE
  GetTextBeforePosition(U32 text_pos, const WCHAR **text_string, U32 *text_length) noexcept override
  {
    text_pos = Min(text_pos, _text_length);
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
    ProfileFunction();
    TextAnalysisSinkResultChunk *chunk = last_result_chunk;
    if(chunk == 0 || chunk->count == ARRAYSIZE(chunk->v))
    {
      chunk = f_dwrite_map_text_to_glyphs_state->first_text_analsys_sink_chunk;
      if(chunk == 0)
      {
        chunk = push_array<TextAnalysisSinkResultChunk>(f_dwrite_map_text_to_glyphs_state->arena, 1);
      }
      else
      {
        SLLStackPop(f_dwrite_map_text_to_glyphs_state->first_text_analsys_sink_chunk);
        MemoryZeroStruct(chunk);
      }
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
      SLLStackPush(f_dwrite_map_text_to_glyphs_state->first_text_analsys_sink_chunk, chunk);
    }
  }
};

static F_DWrite_GlyphArray *
allocate_and_push_back_glyph_array(Arena *arena, F_DWrite_GlyphArrayChunk **first_chunk, F_DWrite_GlyphArrayChunk **last_chunk, U32 glyph_count)
{
  ProfileFunction();
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

  glyph_array->count = glyph_count;
  glyph_array->advances = push_array_no_zero<F32>(arena, glyph_array->count);
  glyph_array->offsets = push_array<DWRITE_GLYPH_OFFSET>(arena, glyph_array->count);
  glyph_array->indices = push_array_no_zero<U16>(arena, glyph_array->count);

  (*last_chunk)->total_glyph_count += glyph_array->count;

  return glyph_array;
}

static F_DWrite_TextToGlyphsSegmentNode *
allocate_and_push_back_segment_node(Arena *arena, F_DWrite_TextToGlyphsSegmentNode **first_segment, F_DWrite_TextToGlyphsSegmentNode **last_segment)
{
  ProfileFunction();
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
fill_segment_with_glyph_array_chunks(Arena *arena, F_DWrite_TextToGlyphsSegment *segment, F_DWrite_GlyphArrayChunk *first_chunk, F_DWrite_GlyphArrayChunk *last_chunk)
{
  ProfileFunction();
  U64 total_glyph_count = 0;
  for(F_DWrite_GlyphArrayChunk *chunk = first_chunk; chunk != 0; chunk = chunk->next)
  {
    total_glyph_count += chunk->total_glyph_count;
  }

  segment->glyph_count = total_glyph_count;
  segment->glyph_indices = push_array_no_zero<U16>(arena, segment->glyph_count);
  segment->glyph_advances = push_array_no_zero<F32>(arena, segment->glyph_count);
  segment->glyph_offsets = push_array_no_zero<DWRITE_GLYPH_OFFSET>(arena, segment->glyph_count);
  U64 glyph_idx_offset = 0;
  for(F_DWrite_GlyphArrayChunk *chunk = first_chunk; chunk != 0; chunk = chunk->next)
  {
    for(U64 glyph_array_idx = 0; glyph_array_idx < chunk->count; ++glyph_array_idx)
    {
      F_DWrite_GlyphArray &glyph_array = chunk->v[glyph_array_idx];
      MemoryCopyTyped(segment->glyph_indices + glyph_idx_offset, glyph_array.indices, glyph_array.count);
      MemoryCopyTyped(segment->glyph_advances + glyph_idx_offset, glyph_array.advances, glyph_array.count);
      MemoryCopyTyped(segment->glyph_offsets + glyph_idx_offset, glyph_array.offsets, glyph_array.count);
      glyph_idx_offset += glyph_array.count;
    }
  }
}

static U64
f_dwrite_hash_from_string(String16 string)
{
  U64 hash = 1;
  if(string.size != 0)
  {
    for(U64 i = 0; i < string.size; ++i)
    {
      hash = ((hash << 5) + hash) + string[i];
    }
  }
  return hash;
}

static F_DWrite_MapTextToGlyphsResult
f_dwrite_map_text_to_glyphs(IDWriteFontFallback1 *font_fallback, IDWriteFontCollection *font_collection, IDWriteTextAnalyzer1 *text_analyzer, const wchar_t *locale, String16 base_family, U32 font_size, String16 text)
{
  ProfileFunction();

  F_DWrite_MapTextToGlyphsResult result = {};

  if(f_dwrite_map_text_to_glyphs_state == 0)
  {
    Arena *state_arena = arena_alloc();
    f_dwrite_map_text_to_glyphs_state = push_array<F_DWrite_MapTextToGlyphsState>(state_arena, 1);
    f_dwrite_map_text_to_glyphs_state->arena = state_arena;
  }

  if(text.size != 0)
  {
    // hampus: lookup text to glyph mapping in cache
    F_DWrite_MapTextToGlyphsResultSlot *slot = 0;
    U64 hash = f_dwrite_hash_from_string(text);
    U64 slot_idx = hash % array_count(f_dwrite_map_text_to_glyphs_state->text_to_glyphs_result_map);
    {
      for(slot = f_dwrite_map_text_to_glyphs_state->text_to_glyphs_result_map[slot_idx]; slot != 0; slot = slot->hash_next)
      {
        if(slot->text.size == text.size && slot->base_family.size == base_family.size)
        {
          if(MemoryMatch(text.data, slot->text.data, slot->text.size * sizeof(U16)) &&
             MemoryMatch(base_family.data, slot->base_family.data, slot->base_family.size * sizeof(U16)) &&
             slot->font_size == font_size)
          {
            result = slot->v;
            break;
          }
        }
      }
    }

    // hampus: cache miss
    if(slot == 0)
    {
      TempArena function_scratch = GetScratch(0, 0);

      wchar_t *cstr16_base_family = cstr16_from_str16(function_scratch.arena, base_family);
      wchar_t *cstr_text = cstr16_from_str16(function_scratch.arena, text);
      U32 text_length_u32 = safe_u32_from_u64(text.size);

      HRESULT hr = 0;

      for(U32 fallback_offset = 0; fallback_offset < text_length_u32;)
      {
        TempArena scratch = GetScratch(&function_scratch.arena, 1);

        U32 max_glyph_indices_count = text_length_u32 * 256;
        U16 *glyph_indices = push_array_no_zero<U16>(scratch.arena, max_glyph_indices_count);
        DWRITE_SHAPING_GLYPH_PROPERTIES *glyph_props = push_array_no_zero<DWRITE_SHAPING_GLYPH_PROPERTIES>(scratch.arena, max_glyph_indices_count);
        U16 *cluster_map = push_array_no_zero<U16>(scratch.arena, max_glyph_indices_count);
        DWRITE_SHAPING_TEXT_PROPERTIES *text_props = push_array_no_zero<DWRITE_SHAPING_TEXT_PROPERTIES>(scratch.arena, max_glyph_indices_count);

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

        //----------------------------------------------------------
        // hampus: get mapped font and length

        IDWriteFontFace5 *mapped_font_face = 0;
        U32 mapped_text_length = 0;
        {
          // NOTE(hampus): We need an analysis source that holds the text and the locale
          TextAnalysisSource analysis_source{locale, cstr_text, text_length_u32};

          // NOTE(hampus): This get the appropiate font required for rendering the text
          F32 scale = 0;
          {
            ProfileScope("MapCharacters()");
            hr = font_fallback->MapCharacters(&analysis_source,
                                              fallback_offset,
                                              text_length_u32,
                                              font_collection,
                                              cstr16_base_family,
                                              0,
                                              0,
                                              &mapped_text_length,
                                              &scale,
                                              &mapped_font_face);
          }
          Assert(SUCCEEDED(hr));
          if(mapped_font_face == 0)
          {
            // NOTE(hampus): This means that no font was available for this character.
            // mapped_text_length is the number of characters to skip. We will
            // replace these characters with a missing glyph.

            F_DWrite_TextToGlyphsSegmentNode *segment_node = allocate_and_push_back_segment_node(f_dwrite_map_text_to_glyphs_state->arena, &result.first_segment, &result.last_segment);
            segment = &segment_node->v;
            segment->font_face = mapped_font_face;
            segment->font_size_em = font_size;

            F_DWrite_GlyphArray *glyph_array = allocate_and_push_back_glyph_array(scratch.arena, &first_glyph_array_chunk, &last_glyph_array_chunk, mapped_text_length);
            MemoryZeroTyped(glyph_array->indices, glyph_array->count);

            fill_segment_with_glyph_array_chunks(f_dwrite_map_text_to_glyphs_state->arena, segment, first_glyph_array_chunk, last_glyph_array_chunk);
            first_glyph_array_chunk = 0;
            last_glyph_array_chunk = 0;
            segment = 0;

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

        const wchar_t *fallback_ptr = cstr_text + fallback_offset;
        const wchar_t *fallback_opl = fallback_ptr + mapped_text_length;
        while(fallback_ptr < fallback_opl)
        {
          U32 fallback_remaining = (U32)(fallback_opl - fallback_ptr);

          BOOL is_simple = FALSE;
          U32 complex_mapped_length = 0;

          hr = text_analyzer->GetTextComplexity(fallback_ptr,
                                                fallback_remaining,
                                                mapped_font_face,
                                                &is_simple,
                                                &complex_mapped_length,
                                                glyph_indices);
          Assert(SUCCEEDED(hr));

          if(is_simple)
          {
            // NOTE(hampus): This text was simple. This means we can just use
            // the indices directly without having to do any more shaping work.

            if(segment != 0)
            {
              if(segment->bidi_level != 0)
              {
                fill_segment_with_glyph_array_chunks(f_dwrite_map_text_to_glyphs_state->arena, segment, first_glyph_array_chunk, last_glyph_array_chunk);
                first_glyph_array_chunk = 0;
                last_glyph_array_chunk = 0;
                segment = 0;
              }
            }

            if(segment == 0)
            {
              F_DWrite_TextToGlyphsSegmentNode *segment_node = allocate_and_push_back_segment_node(f_dwrite_map_text_to_glyphs_state->arena, &result.first_segment, &result.last_segment);
              segment = &segment_node->v;
              segment->font_face = mapped_font_face;
              segment->font_size_em = font_size;
            }

            // hampus: get a new glyph array

            F_DWrite_GlyphArray *glyph_array = allocate_and_push_back_glyph_array(scratch.arena, &first_glyph_array_chunk, &last_glyph_array_chunk, complex_mapped_length);

            // hampus: fill in indices

            MemoryCopyTyped(glyph_array->indices, glyph_indices, glyph_array->count);

            // hampus: fill in advances

            {
              DWRITE_FONT_METRICS1 font_metrics = {};
              mapped_font_face->GetMetrics(&font_metrics);
              S32 *design_advances = push_array_no_zero<S32>(scratch.arena, glyph_array->count);
              hr = mapped_font_face->GetDesignGlyphAdvances((U32)glyph_array->count, glyph_array->indices, design_advances);
              F32 scale = (F32)font_size / (F32)font_metrics.designUnitsPerEm;
              for(U64 idx = 0; idx < glyph_array->count; idx++)
              {
                glyph_array->advances[idx] = (F32)design_advances[idx] * scale;
              }
            }
          }
          else
          {
            // NOTE(hampus): This text was not simple. We have to do extra work. :(

            TextAnalysisSource analysis_source{locale, fallback_ptr, complex_mapped_length};
            TextAnalysisSink analysis_sink = {};

            hr = text_analyzer->AnalyzeScript(&analysis_source, 0, complex_mapped_length, &analysis_sink);
            Assert(SUCCEEDED(hr));

            // NOTE(hampus): The text range given to AnalyzeBidi should not split a paragraph.
            // It is meant for one paragraph as a whole or multiple paragraphs.
            // TODO(hampus): What to do about it? What is a paragraph?
            hr = text_analyzer->AnalyzeBidi(&analysis_source, 0, complex_mapped_length, &analysis_sink);
            Assert(SUCCEEDED(hr));

            for(TextAnalysisSinkResultChunk *chunk = analysis_sink.first_result_chunk; chunk != 0; chunk = chunk->next)
            {
              for(U64 text_analysis_sink_result_idx = 0; text_analysis_sink_result_idx < chunk->count; ++text_analysis_sink_result_idx)
              {
                TextAnalysisSinkResult &analysis_result = chunk->v[text_analysis_sink_result_idx];

                if(segment != 0)
                {
                  if(segment->bidi_level != analysis_result.resolved_bidi_level)
                  {
                    fill_segment_with_glyph_array_chunks(f_dwrite_map_text_to_glyphs_state->arena, segment, first_glyph_array_chunk, last_glyph_array_chunk);
                    first_glyph_array_chunk = 0;
                    last_glyph_array_chunk = 0;
                    segment = 0;
                  }
                }

                if(segment == 0)
                {
                  F_DWrite_TextToGlyphsSegmentNode *segment_node = allocate_and_push_back_segment_node(f_dwrite_map_text_to_glyphs_state->arena, &result.first_segment, &result.last_segment);
                  segment = &segment_node->v;
                  segment->font_face = mapped_font_face;
                  segment->font_size_em = font_size;
                  segment->bidi_level = analysis_result.resolved_bidi_level;
                }

                U32 actual_glyph_count = 0;

                BOOL is_right_to_left = (BOOL)(analysis_result.resolved_bidi_level & 1);
                for(int retry = 0;;)
                {
                  {
                    ProfileScope("GetGlyphs()");
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
                    NotImplemented;
                  }

                  Assert(SUCCEEDED(hr));
                  break;
                }

                F_DWrite_GlyphArray *glyph_array = allocate_and_push_back_glyph_array(scratch.arena, &first_glyph_array_chunk, &last_glyph_array_chunk, actual_glyph_count);

                MemoryCopyTyped(glyph_array->indices, glyph_indices, actual_glyph_count);
                {
                  ProfileScope("GetGlyphPlacements()");
                  hr = text_analyzer->GetGlyphPlacements(fallback_ptr + analysis_result.text_position,
                                                         cluster_map,
                                                         text_props,
                                                         analysis_result.text_length,
                                                         glyph_array->indices,
                                                         glyph_props,
                                                         actual_glyph_count,
                                                         mapped_font_face,
                                                         (FLOAT)font_size,
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
                Assert(SUCCEEDED(hr));
              }
            }
          }

          fallback_ptr += complex_mapped_length;
        }

        //----------------------------------------------------------
        // hampus: convert our list of glyph arrays into one big array

        {
          fill_segment_with_glyph_array_chunks(f_dwrite_map_text_to_glyphs_state->arena, segment, first_glyph_array_chunk, last_glyph_array_chunk);
        }

        fallback_offset += mapped_text_length;
      }
      // hampus: insert slot into cache
      slot = push_array<F_DWrite_MapTextToGlyphsResultSlot>(f_dwrite_map_text_to_glyphs_state->arena, 1);
      slot->text = str16_copy(f_dwrite_map_text_to_glyphs_state->arena, text);
      slot->font_size = font_size;
      slot->base_family = str16_copy(f_dwrite_map_text_to_glyphs_state->arena, base_family);
      slot->v = result;
      SLLStackPushN(f_dwrite_map_text_to_glyphs_state->text_to_glyphs_result_map[slot_idx], slot, hash_next);
    }
  }

  return result;
}
