#pragma comment(lib, "d2d1.lib")

static F_D2D_State *f_d2d_state = 0;

static F_FontMetrics
f_dwrite_get_font_metrics(IDWriteFontFace *font_face, U32 size)
{
  DWRITE_FONT_METRICS font_metrics = {0};
  font_face->GetMetrics(&font_metrics);
  F32 design_units_per_em = (F32)font_metrics.designUnitsPerEm;
  F32 font_scale = (F32)size * (96.0f / 72.0f) / design_units_per_em;

  F32 line_gap = floor_f32((F32)font_metrics.lineGap * font_scale);
  F32 ascent = floor_f32((F32)font_metrics.ascent * font_scale);
  F32 descent = floor_f32((F32)font_metrics.descent * font_scale);

  F_FontMetrics result = {};
  result.line_gap = line_gap;
  result.descent = descent;
  result.ascent = ascent;
  return result;
}

static F_Handle
f_handle_zero()
{
  F_Handle result = {};
  return result;
}

static B32
f_handle_match(F_Handle a, F_Handle b)
{
  B32 result = memory_match(&a, &b, sizeof(F_Handle));
  return result;
}

static B32
f_tag_match(F_Tag a, F_Tag b)
{
  B32 result = str8_match(a.path, b.path);
  return result;
}

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

  hr = f_d2d_state->d2d_factory->CreateDevice(r_d3d11_state->dxgi_device, &f_d2d_state->d2d_device);
  ASSERT(SUCCEEDED(hr));

  D2D1_DEVICE_CONTEXT_OPTIONS d2d_deice_context_options = {};
  hr = f_d2d_state->d2d_device->CreateDeviceContext(d2d_deice_context_options, &f_d2d_state->d2d_device_context);
  ASSERT(SUCCEEDED(hr));

  //----------------------------------------------------------
  // hampus: get locale name

  if(!GetUserDefaultLocaleName(&f_d2d_state->locale[0], LOCALE_NAME_MAX_LENGTH))
  {
    memory_copy(&f_d2d_state->locale[0], L"en-US", sizeof(L"en-US"));
  }

  //----------------------------------------------------------
  // hampus: create rendering params

  // TODO(hampus): Since multiple monitors may have different rendering settings,
  // this has to be reset if the monitor where to change. We can look for the
  // WM_WINDOWPOSCHANGED message and check if we actually have changed monitor.

  IDWriteRenderingParams *base_rendering_params = 0;

  hr = f_d2d_state->dwrite_factory->CreateRenderingParams(&base_rendering_params);
  ASSERT(SUCCEEDED(hr));

  FLOAT gamma = base_rendering_params->GetGamma();
  FLOAT enhanced_contrast = base_rendering_params->GetEnhancedContrast();
  FLOAT clear_type_level = base_rendering_params->GetClearTypeLevel();
  hr = f_d2d_state->dwrite_factory->CreateCustomRenderingParams(gamma,
                                                                enhanced_contrast,
                                                                clear_type_level,
                                                                base_rendering_params->GetPixelGeometry(),
                                                                base_rendering_params->GetRenderingMode(),
                                                                &f_d2d_state->rendering_params);
  ASSERT(SUCCEEDED(hr));

  base_rendering_params->Release();

  //----------------------------------------------------------
  // hampus: create atlas & bitmap render target

  f_d2d_state->atlas.atlas = atlas_make(arena, v2u64(1024, 1024));
  f_d2d_state->atlas.handle = r_make_tex2d_from_bitmap(f_d2d_state->atlas.atlas.memory,
                                                       safe_u32_from_u64(f_d2d_state->atlas.atlas.dim.x), safe_u32_from_u64(f_d2d_state->atlas.atlas.dim.y),
                                                       R_PixelFormat_BGRA,
                                                       R_TextureBindFlags_RenderTarget);
  R_D3D11_Texture *d3d11_texture = (R_D3D11_Texture *)ptr_from_int(f_d2d_state->atlas.handle.u64[0]);

  IDXGISurface *dxgi_surface = {};
  hr = d3d11_texture->texture->QueryInterface(__uuidof(IDXGISurface), (void **)&dxgi_surface);
  ASSERT(SUCCEEDED(hr));

  // TODO(hampus): properly query DPI
  // UINT dpi = os_window_dpi(OS_Handle window)
  FLOAT dpi = 90;

  B32 use_cleartype = false;
  D2D1_BITMAP_PROPERTIES1 bitmap_props = {};
  bitmap_props.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
  bitmap_props.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
  bitmap_props.pixelFormat.alphaMode = use_cleartype ? D2D1_ALPHA_MODE_IGNORE : D2D1_ALPHA_MODE_PREMULTIPLIED;
  bitmap_props.dpiY = dpi;
  bitmap_props.dpiX = dpi;

  ID2D1Bitmap1 *bitmap = 0;
  hr = f_d2d_state->d2d_device_context->CreateBitmapFromDxgiSurface(dxgi_surface, bitmap_props, &bitmap);
  ASSERT(SUCCEEDED(hr));

  f_d2d_state->d2d_device_context->SetTarget(bitmap);

  f_d2d_state->d2d_device_context->SetTextRenderingParams(f_d2d_state->rendering_params);

  dxgi_surface->Release();
  bitmap->Release();

  //----------------------------------------------------------
  // hampus: create foregound brush

  D2D1_COLOR_F foreground_color = {1, 1, 1, 1};
  hr = f_d2d_state->d2d_device_context->CreateSolidColorBrush(&foreground_color, 0, &f_d2d_state->foreground_brush);
  ASSERT(SUCCEEDED(hr));
}

static F_Handle
f_open_font(Arena *arena, String8 path)
{
  profile_function();

  TempArena scratch = get_scratch(&arena, 1);
  F_DWrite_Font *dw_font = push_array<F_DWrite_Font>(arena, 1);
  HRESULT hr = 0;

  hr = f_d2d_state->dwrite_factory->CreateFontFileReference((const WCHAR *)cstr16_from_str8(scratch.arena, path),
                                                            0, &dw_font->font_file);
  ASSERT(SUCCEEDED(hr));

  hr = f_d2d_state->dwrite_factory->CreateFontFace(DWRITE_FONT_FACE_TYPE_TRUETYPE,
                                                   1,
                                                   &dw_font->font_file,
                                                   0,
                                                   DWRITE_FONT_SIMULATIONS_NONE,
                                                   &dw_font->font_face);
  ASSERT(SUCCEEDED(hr));
  F_Handle result = {};
  result.u64[0] = int_from_ptr(dw_font);
  return result;
}

static F_Handle
f_handle_from_tag(F_Tag tag)
{
  F_Handle result = f_handle_zero();
  U64 cold_idx = U64_MAX;

  // hampus: Cache lookup

  for(U64 i = 0; i < array_count(f_d2d_state->font_tag_table); ++i)
  {
    if(f_handle_match(f_d2d_state->dwrite_font_table[i], f_handle_zero()) &&
       cold_idx == U64_MAX)
    {
      cold_idx = i;
    }
    else if(f_tag_match(f_d2d_state->font_tag_table[i], tag))
    {
      result = f_d2d_state->dwrite_font_table[i];
    }
  }

  // hampus: Cache miss

  if(f_handle_match(result, f_handle_zero()))
  {
    if(cold_idx != U64_MAX)
    {
      F_Handle f_handle = f_open_font(f_d2d_state->arena, tag.path);
      ASSERT(!f_handle_match(f_handle, f_handle_zero()));

      // TODO(hampus): Close the old font handle
      f_d2d_state->font_tag_table[cold_idx] = tag;
      f_d2d_state->dwrite_font_table[cold_idx] = f_handle;

      result = f_handle;
    }
    else
    {
      // TODO(hampus): What to do here? Cache is hot and full
      invalid_code_path;
    }
  }

  return result;
}

static F_GlyphRun
f_make_glyph_run(Arena *arena, F_Tag tag, U32 size, String32 str32)
{
  profile_function();

  F_GlyphRun result = {};

  TempArena scratch = get_scratch(&arena, 1);
  String16 str16 = cstr16_from_str32(scratch.arena, str32);

  // TODO(hampus): Convert the tag to a dwrite font
  // and query the name and use that instead of "Fira Code"

  F_DWrite_MapTextToGlyphsResult map_text_to_glyphs_result = f_dwrite_map_text_to_glyphs(scratch.arena, f_d2d_state->font_fallback1,
                                                                                         f_d2d_state->font_collection,
                                                                                         f_d2d_state->text_analyzer1,
                                                                                         f_d2d_state->locale,
                                                                                         L"Consolas",
                                                                                         (F32)size, (const wchar_t *)str16.data, (U32)str16.size);

  for(F_DWrite_TextToGlyphsSegmentNode *n = map_text_to_glyphs_result.first_segment; n != 0; n = n->next)
  {
    const F_DWrite_TextToGlyphsSegment &segment = n->v;
    for(U64 idx = 0; idx < segment.glyph_count; ++idx)
    {
      U16 glyph_idx = segment.glyph_indices[idx];
      IDWriteFontFace5 *font_face = segment.font_face;
      if(font_face != 0)
      {
        // hampus: lookup glyph in table

        F_Glyph *glyph = 0;
        U64 slot_idx = glyph_idx % array_count(f_d2d_state->glyph_from_idx_lookup_table);
        for(glyph = f_d2d_state->glyph_from_idx_lookup_table[slot_idx];
            glyph != 0;
            glyph = glyph->hash_next)
        {
          if(glyph->idx == glyph_idx && glyph->font_face == font_face && glyph->size == size)
          {
            break;
          }
        }

        if(glyph == 0)
        {
          // hampus: create dwrite glyph run

          DWRITE_GLYPH_RUN dwrite_glyph_run = {};
          dwrite_glyph_run.glyphCount = 1;
          dwrite_glyph_run.fontFace = font_face;
          dwrite_glyph_run.fontEmSize = (FLOAT)size;
          dwrite_glyph_run.glyphIndices = &glyph_idx;
          dwrite_glyph_run.glyphOffsets = &segment.glyph_offsets[idx];
          dwrite_glyph_run.glyphAdvances = &segment.glyph_advances[idx];

          // hampus: get glyph run world bounds

          D2D1_RECT_F glyph_world_bounds = {};
          B32 is_whitespace = false;
          {
            D2D1_POINT_2F baseline = {0, 0};
            HRESULT hr = f_d2d_state->d2d_device_context->GetGlyphRunWorldBounds(baseline, &dwrite_glyph_run, DWRITE_MEASURING_MODE_NATURAL, &glyph_world_bounds);
            ASSERT(SUCCEEDED(hr));
            is_whitespace = !(glyph_world_bounds.right > glyph_world_bounds.left && glyph_world_bounds.bottom > glyph_world_bounds.top);
          }

          // hampus: rasterize glyph if it is not whitespace

          Vec2U64 bitmap_dim = {};
          RectU64 atlas_region = {};
          if(!is_whitespace)
          {
            profile_scope("rasterize glyph");
            F_FontMetrics font_metrics = f_dwrite_get_font_metrics(font_face, size);
            f_d2d_state->d2d_device_context->BeginDraw();
            bitmap_dim = v2u64((U64)(glyph_world_bounds.right - glyph_world_bounds.left), (U64)(font_metrics.ascent + font_metrics.descent + font_metrics.line_gap));
            atlas_region = atlas_region_alloc(f_d2d_state->arena, &f_d2d_state->atlas.atlas, bitmap_dim);
            D2D1_POINT_2F baseline = {(FLOAT)atlas_region.x0 - glyph_world_bounds.left, (FLOAT)atlas_region.y1 - font_metrics.descent};
            f_d2d_state->d2d_device_context->DrawGlyphRun(baseline, &dwrite_glyph_run, f_d2d_state->foreground_brush);
            HRESULT hr = f_d2d_state->d2d_device_context->EndDraw();
            ASSERT(SUCCEEDED(hr));
          }

          // hampus: fill in glyph data

          glyph = push_array<F_Glyph>(f_d2d_state->arena, 1);
          glyph->idx = glyph_idx;
          glyph->font_face = font_face;
          glyph->bitmap_size = bitmap_dim;
          glyph->metrics.advance = round_f32(segment.glyph_advances[idx]);
          glyph->metrics.left_bearing = glyph_world_bounds.left;
          glyph->size = size;
          glyph->region_uv = r4f32((F32)atlas_region.x0 / (F32)f_d2d_state->atlas.atlas.dim.x, (F32)atlas_region.y0 / (F32)f_d2d_state->atlas.atlas.dim.y,
                                   (F32)atlas_region.x1 / (F32)f_d2d_state->atlas.atlas.dim.x, (F32)atlas_region.y1 / (F32)f_d2d_state->atlas.atlas.dim.y);
          sll_stack_push_n(f_d2d_state->glyph_from_idx_lookup_table[slot_idx], glyph, hash_next);
        }

        // hampus: fill in glyph run data

        F_GlyphRunNode *glyph_run_node = push_array<F_GlyphRunNode>(arena, 1);
        glyph_run_node->bitmap_size = glyph->bitmap_size;
        glyph_run_node->metrics = glyph->metrics;
        glyph_run_node->region_uv = glyph->region_uv;

        dll_push_back(result.first, result.last, glyph_run_node);
      }
      else
      {
        // TODO(hampus): No font was available for this glyph index. Render
        // with an empty glyph.
        ASSERT(!"NOT IMPLEMENTED");
      }
    }
  }

  return result;
}

static F_GlyphRun
f_make_glyph_run(Arena *arena, F_Tag tag, U32 size, String8 string)
{
  TempArena scratch = get_scratch(&arena, 1);
  String32 str32 = str32_from_str8(scratch.arena, string);
  F_GlyphRun result = f_make_glyph_run(arena, tag, size, str32);
  return result;
}

static F32
f_get_advance(F_Tag tag, U32 size, String32 string)
{
  F32 result = {};
  TempArena scratch = get_scratch(0, 0);
  F_GlyphRun glyph_run = f_make_glyph_run(scratch.arena, tag, size, string);
  for(F_GlyphRunNode *n = glyph_run.first; n != 0; n = n->next)
  {
    result += n->metrics.advance;
  }
  return result;
}

static F32
f_get_advance(F_Tag tag, U32 size, String8 string)
{
  profile_function();
  F32 result = {};
  TempArena scratch = get_scratch(0, 0);
  F_GlyphRun glyph_run = f_make_glyph_run(scratch.arena, tag, size, string);
  for(F_GlyphRunNode *n = glyph_run.first; n != 0; n = n->next)
  {
    result += n->metrics.advance;
  }
  return result;
}

static F32
f_get_advance(F_Tag tag, U32 size, U32 cp)
{
  F32 result = {};
  TempArena scratch = get_scratch(0, 0);
  F_GlyphRun glyph_run = f_make_glyph_run(scratch.arena, tag, size, str32(&cp, 1));
  result = glyph_run.first->metrics.advance;
  return result;
}

static F32
f_line_height_from_tag_size(F_Tag tag, U32 size)
{
  F32 result = 0;
  F_FontMetrics font_metrics = f_get_font_metrics(tag, size);
  result = font_metrics.ascent + font_metrics.descent + font_metrics.line_gap;
  return result;
}

static F_FontMetrics
f_get_font_metrics(F_Tag tag, U32 size)
{
  F_Handle f_handle = f_handle_from_tag(tag);
  F_DWrite_Font *dwrite_font = (F_DWrite_Font *)ptr_from_int(f_handle.u64[0]);
  ASSERT(dwrite_font != 0);
  F_FontMetrics result = f_dwrite_get_font_metrics(dwrite_font->font_face, size);
  return result;
}

static F_Atlas *
f_atlas()
{
  F_Atlas *result = &f_d2d_state->atlas;
  return result;
}

static void
f_destroy()
{
  f_d2d_state->foreground_brush->Release();
  f_d2d_state->d2d_device_context->Release();
  f_d2d_state->d2d_device->Release();
  f_d2d_state->d2d_factory->Release();
}