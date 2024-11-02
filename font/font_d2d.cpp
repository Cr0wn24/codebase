#pragma comment(lib, "d2d1.lib")

static F_D2D_State *f_d2d_state = 0;

static void
f_init()
{
  Arena *arena = arena_alloc();
  f_d2d_state = push_array<F_D2D_State>(arena, 1);
  f_d2d_state->arena = arena;

  //----------------------------------------------------------
  // hampus: create dwrite factory

  HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                                   __uuidof(IDWriteFactory4),
                                   (IUnknown **)&f_d2d_state->dwrite_factory);
  ASSERT(SUCCEEDED(hr));

  //----------------------------------------------------------
  // hampus: create dwrite font fallback

  hr = f_d2d_state->dwrite_factory->GetSystemFontFallback(&f_d2d_state->font_fallback);
  ASSERT(SUCCEEDED(hr));

  hr = f_d2d_state->font_fallback->QueryInterface(__uuidof(IDWriteFontFallback1), (void **)&f_d2d_state->font_fallback1);
  ASSERT(SUCCEEDED(hr));

  //----------------------------------------------------------
  // hampus: create dwrite font collection

  hr = f_d2d_state->dwrite_factory->GetSystemFontCollection(&f_d2d_state->font_collection);
  ASSERT(SUCCEEDED(hr));

  //----------------------------------------------------------
  // hampus: create dwrite text analyzer

  hr = f_d2d_state->dwrite_factory->CreateTextAnalyzer(&f_d2d_state->text_analyzer);
  ASSERT(SUCCEEDED(hr));

  hr = f_d2d_state->text_analyzer->QueryInterface(__uuidof(IDWriteTextAnalyzer1), (void **)&f_d2d_state->text_analyzer1);
  ASSERT(SUCCEEDED(hr));

  //----------------------------------------------------------
  // hampus: create d2d factory

  D2D1_FACTORY_OPTIONS options = {};
  options.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
  hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, &f_d2d_state->d2d_factory);
  ASSERT(SUCCEEDED(hr));

  //----------------------------------------------------------
  // hampus: create d2d device

  ID2D1Device4 *d2d_device = 0;

  hr = d2d_factory->CreateDevice(dxgi_device, &d2d_device);
  ASSERT(SUCCEEDED(hr));

  if(!GetUserDefaultLocaleName(&f_d2d_state->locale[0], LOCALE_NAME_MAX_LENGTH))
  {
    memory_copy(&f_d2d_state->locale[0], L"en-US", sizeof(L"en-US"));
  }
}

static F_GlyphRun
f_make_glyph_run(Arena *arena, F_Tag tag, U32 size, String32 str32)
{
  TempArena scratch = get_scratch(&arena, 1);
  String16 str16 = cstr16_from_str32(scratch.arena, str32);
  F_DWrite_MapTextToGlyphsResult map_text_to_glyphs_result = f_dwrite_map_text_to_glyphs(f_d2d_state->font_fallback,
                                                                                         f_d2d_state->font_collection,
                                                                                         f_d2d_state->text_analyzer,
                                                                                         f_d2d_state->locale,
                                                                                         L"Segoe UI",
                                                                                         (F32)size, (const wchar_t *)str16.data, str16.size);

  for(F_DWrite_TextToGlyphsSegmentNode *n = map_text_to_glyphs_result.first_segment; n != 0; n = n->next)
  {
    const F_DWrite_TextToGlyphsSegment &segment = n->v;
    for(U64 idx = 0; idx < segment.glyph_count; ++idx)
    {
      U16 glyph_idx = segment.glyph_indices[idx];
      IDWriteFontFace5 *font_face = segment.font_face;
      F_Glyph *glyph = 0;
      U64 slot_idx = glyph_idx % array_count(f_d2d_state->glyph_from_idx_lookup_table);
      for(glyph = f_d2d_state->glyph_from_idx_lookup_table[slot_idx];
          glyph != 0;
          glyph = glyph->hash_next)
      {
        if(glyph->idx == glyph_idx && glyph->font_face == font_face)
        {
          break;
        }
      }

      if(glyph == 0)
      {
        glyph = push_array<F_Glyph>(f_d2d_state->arena, 1);
        glyph->idx = glyph_idx;
        glyph->font_face = font_face;

        sll_stack_push_n(f_d2d_state->glyph_from_idx_lookup_table[slot_idx], glyph, hash_next);
      }
    }
  }

  F_GlyphRun result = {};
  return result;
}
