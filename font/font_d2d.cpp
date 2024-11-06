#pragma comment(lib, "d2d1.lib")

static F_D2D_State *f_d2d_state = 0;

static F_DWrite_Font *
f_dwrite_font_from_handle(F_Handle handle)
{
  F_DWrite_Font *result = (F_DWrite_Font *)PtrFromInt(handle.u64[0]);
  return result;
}

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

static F_Glyph *
f_dwrite_get_glyph()
{
  F_Glyph *result = 0;
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
  B32 result = MemoryMatch(&a, &b, sizeof(F_Handle));
  return result;
}

static B32
f_tag_match(F_Tag a, F_Tag b)
{
  B32 result = str8_match(a.string, b.string);
  return result;
}

static F_Tag
f_make_tag(String8 string)
{
  F_Tag result = {};
  result.string = string;
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
  Assert(SUCCEEDED(hr));

  //----------------------------------------------------------
  // hampus: create dwrite font fallback

  hr = f_d2d_state->dwrite_factory->GetSystemFontFallback(&f_d2d_state->font_fallback);
  Assert(SUCCEEDED(hr));

  hr = f_d2d_state->font_fallback->QueryInterface(__uuidof(IDWriteFontFallback1), (void **)&f_d2d_state->font_fallback1);
  Assert(SUCCEEDED(hr));

  //----------------------------------------------------------
  // hampus: create dwrite font collection

  hr = f_d2d_state->dwrite_factory->GetSystemFontCollection(&f_d2d_state->font_collection);
  Assert(SUCCEEDED(hr));

  //----------------------------------------------------------
  // hampus: create dwrite text analyzer

  hr = f_d2d_state->dwrite_factory->CreateTextAnalyzer(&f_d2d_state->text_analyzer);
  Assert(SUCCEEDED(hr));

  hr = f_d2d_state->text_analyzer->QueryInterface(__uuidof(IDWriteTextAnalyzer1), (void **)&f_d2d_state->text_analyzer1);
  Assert(SUCCEEDED(hr));

  //----------------------------------------------------------
  // hampus: create d2d factory

  D2D1_FACTORY_OPTIONS options = {};
#ifndef NDEBUG
  options.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
#endif
  hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, &f_d2d_state->d2d_factory);
  Assert(SUCCEEDED(hr));

  //----------------------------------------------------------
  // hampus: create d2d device

  hr = f_d2d_state->d2d_factory->CreateDevice(r_d3d11_state->dxgi_device, &f_d2d_state->d2d_device);
  Assert(SUCCEEDED(hr));

  D2D1_DEVICE_CONTEXT_OPTIONS d2d_deice_context_options = {};
  hr = f_d2d_state->d2d_device->CreateDeviceContext(d2d_deice_context_options, &f_d2d_state->d2d_device_context);
  Assert(SUCCEEDED(hr));

  //----------------------------------------------------------
  // hampus: get locale name

  if(!GetUserDefaultLocaleName(&f_d2d_state->locale[0], LOCALE_NAME_MAX_LENGTH))
  {
    MemoryCopy(&f_d2d_state->locale[0], L"en-US", sizeof(L"en-US"));
  }

  //----------------------------------------------------------
  // hampus: create rendering params

  // TODO(hampus): Since multiple monitors may have different rendering settings,
  // this has to be reset if the monitor where to change. We can look for the
  // WM_WINDOWPOSCHANGED message and check if we actually have changed monitor.

  IDWriteRenderingParams *base_rendering_params = 0;

  hr = f_d2d_state->dwrite_factory->CreateRenderingParams(&base_rendering_params);
  Assert(SUCCEEDED(hr));

  FLOAT gamma = base_rendering_params->GetGamma();
  FLOAT enhanced_contrast = base_rendering_params->GetEnhancedContrast();
  FLOAT clear_type_level = base_rendering_params->GetClearTypeLevel();
  hr = f_d2d_state->dwrite_factory->CreateCustomRenderingParams(gamma,
                                                                enhanced_contrast,
                                                                clear_type_level,
                                                                base_rendering_params->GetPixelGeometry(),
                                                                base_rendering_params->GetRenderingMode(),
                                                                &f_d2d_state->rendering_params);
  Assert(SUCCEEDED(hr));

  base_rendering_params->Release();

  //----------------------------------------------------------
  // hampus: create atlas & bitmap render target

  f_d2d_state->atlas.atlas = atlas_make(arena, v2u64(1024, 1024));
  f_d2d_state->atlas.handle = r_make_tex2d_from_bitmap(f_d2d_state->atlas.atlas.memory,
                                                       safe_u32_from_u64(f_d2d_state->atlas.atlas.dim.x), safe_u32_from_u64(f_d2d_state->atlas.atlas.dim.y),
                                                       R_PixelFormat_BGRA,
                                                       R_TextureBindFlags_RenderTarget);
  R_D3D11_Texture *d3d11_texture = (R_D3D11_Texture *)PtrFromInt(f_d2d_state->atlas.handle.u64[0]);

  IDXGISurface *dxgi_surface = {};
  hr = d3d11_texture->texture->QueryInterface(__uuidof(IDXGISurface), (void **)&dxgi_surface);
  Assert(SUCCEEDED(hr));

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
  Assert(SUCCEEDED(hr));

  f_d2d_state->d2d_device_context->SetTarget(bitmap);

  f_d2d_state->d2d_device_context->SetTextRenderingParams(f_d2d_state->rendering_params);

  dxgi_surface->Release();
  bitmap->Release();

  //----------------------------------------------------------
  // hampus: create foregound brush

  D2D1_COLOR_F foreground_color = {1, 1, 1, 1};
  hr = f_d2d_state->d2d_device_context->CreateSolidColorBrush(&foreground_color, 0, &f_d2d_state->foreground_brush);
  Assert(SUCCEEDED(hr));

  //----------------------------------------------------------
  // hampus: enumerate system fonts

  U32 font_family_count = f_d2d_state->font_collection->GetFontFamilyCount();

  for(U32 font_family_idx = 0; font_family_idx < font_family_count; ++font_family_idx)
  {
    IDWriteFontFamily *font_family = 0;
    hr = f_d2d_state->font_collection->GetFontFamily(font_family_idx, &font_family);
    Assert(SUCCEEDED(hr));

    IDWriteLocalizedStrings *family_names = 0;
    hr = font_family->GetFamilyNames(&family_names);
    Assert(SUCCEEDED(hr));

    BOOL locale_exists = false;
    U32 locale_idx = 0;
    hr = family_names->FindLocaleName(f_d2d_state->locale, &locale_idx, &locale_exists);
    Assert(SUCCEEDED(hr));

    if(!locale_exists)
    {
      locale_idx = 0;
    }

    U32 font_name_length = 0;
    hr = family_names->GetStringLength(locale_idx, &font_name_length);
    Assert(SUCCEEDED(hr));

    wchar_t *font_name = push_array_no_zero<wchar_t>(f_d2d_state->arena, font_name_length + 1);

    hr = family_names->GetString(locale_idx, font_name, font_name_length + 1);
    Assert(SUCCEEDED(hr));
  }
}

static F_Handle
f_open_font(Arena *arena, String8 name)
{
  ProfileFunction();

  TempArena scratch = GetScratch(&arena, 1);
  F_DWrite_Font *dw_font = push_array<F_DWrite_Font>(arena, 1);
  HRESULT hr = 0;

  wchar_t *name_str16 = cstr16_from_str8(scratch.arena, name);

  UINT32 family_idx = 0;
  BOOL family_exists = false;
  IDWriteFontFamily *font_family = 0;
  hr = f_d2d_state->font_collection->FindFamilyName((wchar_t *)name_str16, &family_idx, &family_exists);
  Assert(SUCCEEDED(hr));

  if(family_exists)
  {
    hr = f_d2d_state->font_collection->GetFontFamily(family_idx, &font_family);
    Assert(SUCCEEDED(hr));

    hr = font_family->GetFirstMatchingFont(DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL, &dw_font->font);
    Assert(SUCCEEDED(hr));

    hr = hr = dw_font->font->CreateFontFace(&dw_font->font_face);
    Assert(SUCCEEDED(hr));

    font_family->Release();
  }
  else
  {
    NotImplemented;
  }

  F_Handle result = {};
  result.u64[0] = IntFromPtr(dw_font);
  return result;
}

static F_Handle
f_open_font_file(Arena *arena, String8 path)
{
  F_Handle result = {};
  TempArena scratch = GetScratch(&arena, 1);
  F_DWrite_Font *dw_font = push_array<F_DWrite_Font>(arena, 1);
  HRESULT hr = 0;

  hr = f_d2d_state->dwrite_factory->CreateFontFileReference(cstr16_from_str8(scratch.arena, path),
                                                            0, &dw_font->font_file);
  Assert(SUCCEEDED(hr));

  hr = f_d2d_state->dwrite_factory->CreateFontFace(DWRITE_FONT_FACE_TYPE_TRUETYPE,
                                                   1,
                                                   &dw_font->font_file,
                                                   0,
                                                   DWRITE_FONT_SIMULATIONS_NONE,
                                                   &dw_font->font_face);
  Assert(SUCCEEDED(hr));
  result.u64[0] = IntFromPtr(dw_font);
  return result;
}

static F_Handle
f_handle_from_tag(F_Tag tag)
{
  ProfileFunction();

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
      F_Handle f_handle = f_open_font(f_d2d_state->arena, tag.string);
      Assert(!f_handle_match(f_handle, f_handle_zero()));

      // TODO(hampus): Close the old font handle
      f_d2d_state->font_tag_table[cold_idx] = tag;
      f_d2d_state->dwrite_font_table[cold_idx] = f_handle;

      result = f_handle;
    }
    else
    {
      // TODO(hampus): What to do here? Cache is hot and full
      InvalidCodePath;
    }
  }

  return result;
}

static F_Glyph *
f_dwrite_lookup_glyph(U32 glyph_idx, IDWriteFontFace *font_face, U32 size)
{
  ProfileFunction();

  F_Glyph *result = 0;
  U64 slot_idx = glyph_idx % array_count(f_d2d_state->glyph_from_idx_lookup_table);
  for(result = f_d2d_state->glyph_from_idx_lookup_table[slot_idx];
      result != 0;
      result = result->hash_next)
  {
    if(result->idx == glyph_idx && result->font_face == font_face && result->size == size)
    {
      break;
    }
  }
  return result;
}

struct F_DWrite_RasterResult
{
  D2D1_RECT_F glyph_world_bounds;
  B32 is_whitespace;
  RectU64 atlas_region;
  Vec2U64 bitmap_dim;
};

static F_DWrite_RasterResult
f_dwrite_raster_glyph(DWRITE_GLYPH_RUN dwrite_glyph_run, IDWriteFontFace *font_face, U32 size)
{
  ProfileFunction();

  F_DWrite_RasterResult result = {};

  // hampus: get glyph run world bounds

  D2D1_RECT_F glyph_world_bounds = {};
  B32 is_whitespace = false;
  {
    D2D1_POINT_2F baseline = {0, 0};
    HRESULT hr = f_d2d_state->d2d_device_context->GetGlyphRunWorldBounds(baseline, &dwrite_glyph_run, DWRITE_MEASURING_MODE_NATURAL, &glyph_world_bounds);
    Assert(SUCCEEDED(hr));
    is_whitespace = !(glyph_world_bounds.right > glyph_world_bounds.left && glyph_world_bounds.bottom > glyph_world_bounds.top);
  }

  // hampus: rasterize glyph if it is not whitespace

  Vec2U64 bitmap_dim = {};
  RectU64 atlas_region = {};
  if(!is_whitespace)
  {
    ProfileScope("rasterize glyph");
    F_FontMetrics font_metrics = f_dwrite_get_font_metrics(font_face, size);
    f_d2d_state->d2d_device_context->BeginDraw();
    bitmap_dim = v2u64((U64)(glyph_world_bounds.right - glyph_world_bounds.left), (U64)(font_metrics.ascent + font_metrics.descent + font_metrics.line_gap));
    atlas_region = atlas_region_alloc(f_d2d_state->arena, &f_d2d_state->atlas.atlas, bitmap_dim);
    D2D1_POINT_2F baseline = {(FLOAT)atlas_region.x0 - glyph_world_bounds.left, (FLOAT)atlas_region.y1 - font_metrics.descent};
    IDWriteColorGlyphRunEnumerator1 *run_enumerator = 0;
    const DWRITE_GLYPH_IMAGE_FORMATS desired_glyph_image_formats = DWRITE_GLYPH_IMAGE_FORMATS_TRUETYPE |
                                                                   DWRITE_GLYPH_IMAGE_FORMATS_CFF |
                                                                   DWRITE_GLYPH_IMAGE_FORMATS_COLR |
                                                                   DWRITE_GLYPH_IMAGE_FORMATS_SVG |
                                                                   DWRITE_GLYPH_IMAGE_FORMATS_PNG |
                                                                   DWRITE_GLYPH_IMAGE_FORMATS_JPEG |
                                                                   DWRITE_GLYPH_IMAGE_FORMATS_TIFF |
                                                                   DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8;
    HRESULT hr = f_d2d_state->dwrite_factory->TranslateColorGlyphRun(baseline, &dwrite_glyph_run, 0, desired_glyph_image_formats, DWRITE_MEASURING_MODE_NATURAL, 0, 0, &run_enumerator);
    if(hr == DWRITE_E_NOCOLOR)
    {
      // NOTE(hampus): There was no colored glyph. We can draw them as normal
      f_d2d_state->foreground_brush->SetColor({1, 1, 1, 1});
      f_d2d_state->d2d_device_context->DrawGlyphRun(baseline, &dwrite_glyph_run, f_d2d_state->foreground_brush);
    }
    else
    {
      // NOTE(hampus): There was colored glyph. We have to draw them differently
      for(;;)
      {
        BOOL have_run = FALSE;
        hr = run_enumerator->MoveNext(&have_run);
        Assert(SUCCEEDED(hr));
        if(!have_run)
        {
          break;
        }

        const DWRITE_COLOR_GLYPH_RUN1 *color_glyph_run = 0;
        hr = run_enumerator->GetCurrentRun(&color_glyph_run);
        Assert(SUCCEEDED(hr));

        f_d2d_state->foreground_brush->SetColor(color_glyph_run->runColor);

        switch(color_glyph_run->glyphImageFormat)
        {
          case DWRITE_GLYPH_IMAGE_FORMATS_NONE:
          {
            // NOTE(hampus): Do nothing
            // TODO(hampus): Find out when this is the case.
          }
          break;
          case DWRITE_GLYPH_IMAGE_FORMATS_PNG:
          case DWRITE_GLYPH_IMAGE_FORMATS_JPEG:
          case DWRITE_GLYPH_IMAGE_FORMATS_TIFF:
          case DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8:
          {
            Assert(!"Not tested");
            f_d2d_state->d2d_device_context->DrawColorBitmapGlyphRun(color_glyph_run->glyphImageFormat, {color_glyph_run->baselineOriginX, color_glyph_run->baselineOriginY}, &color_glyph_run->glyphRun, color_glyph_run->measuringMode, D2D1_COLOR_BITMAP_GLYPH_SNAP_OPTION_DEFAULT);
          }
          break;
          case DWRITE_GLYPH_IMAGE_FORMATS_SVG:
          {
            Assert(!"Not tested");
            f_d2d_state->d2d_device_context->DrawSvgGlyphRun({color_glyph_run->baselineOriginX, color_glyph_run->baselineOriginY}, &color_glyph_run->glyphRun, f_d2d_state->foreground_brush, 0, 0, color_glyph_run->measuringMode);
          }
          break;
          default:
          {
            f_d2d_state->d2d_device_context->DrawGlyphRun({color_glyph_run->baselineOriginX, color_glyph_run->baselineOriginY}, &color_glyph_run->glyphRun, color_glyph_run->glyphRunDescription, f_d2d_state->foreground_brush, color_glyph_run->measuringMode);
          }
          break;
        }
      }
    }

    hr = f_d2d_state->d2d_device_context->EndDraw();
    Assert(SUCCEEDED(hr));
  }

  result.glyph_world_bounds = glyph_world_bounds;
  result.is_whitespace = is_whitespace;
  result.atlas_region = atlas_region;
  result.bitmap_dim = bitmap_dim;

  return result;
}

static F_GlyphRun
f_make_simple_glyph_run(Arena *arena, F_Tag tag, U32 size, String32 str32)
{
  ProfileFunction();

  TempArena scratch = GetScratch(0, 0);
  F_GlyphRun result = {};
  F_Handle f_handle = f_handle_from_tag(tag);
  F_DWrite_Font *dw_font = f_dwrite_font_from_handle(f_handle);
  U16 *glyph_indices = push_array_no_zero<U16>(scratch.arena, str32.size);
  HRESULT hr = dw_font->font_face->GetGlyphIndices(str32.data, safe_u32_from_u64(str32.size), glyph_indices);

  F32 *glyph_advances = push_array_no_zero<F32>(scratch.arena, str32.size);
  {
    DWRITE_FONT_METRICS font_metrics = {};
    dw_font->font_face->GetMetrics(&font_metrics);

    DWRITE_GLYPH_METRICS *glyph_metrics = push_array_no_zero<DWRITE_GLYPH_METRICS>(scratch.arena, str32.size);
    hr = dw_font->font_face->GetDesignGlyphMetrics(glyph_indices, (U32)str32.size, glyph_metrics);
    Assert(SUCCEEDED(hr));
    F32 scale = (F32)size / (F32)font_metrics.designUnitsPerEm;
    for(U64 idx = 0; idx < str32.size; idx++)
    {
      glyph_advances[idx] = (F32)glyph_metrics[idx].advanceWidth * scale;
    }
  }

  Assert(SUCCEEDED(hr));
  for(U32 idx = 0; idx < str32.size; ++idx)
  {
    U16 glyph_idx = glyph_indices[idx];
    F_Glyph *glyph = f_dwrite_lookup_glyph(glyph_idx, dw_font->font_face, size);
    if(glyph == 0)
    {
      // hampus: create dwrite glyph run

      DWRITE_GLYPH_RUN dwrite_glyph_run = {};
      dwrite_glyph_run.glyphCount = 1;
      dwrite_glyph_run.fontFace = dw_font->font_face;
      dwrite_glyph_run.fontEmSize = (FLOAT)size;
      dwrite_glyph_run.glyphIndices = &glyph_idx;
      dwrite_glyph_run.glyphAdvances = &glyph_advances[idx];

      // hampus: rasterize glyph

      F_DWrite_RasterResult raster_result = f_dwrite_raster_glyph(dwrite_glyph_run, dw_font->font_face, size);

      // hampus: fill in glyph data

      U64 slot_idx = glyph_idx % array_count(f_d2d_state->glyph_from_idx_lookup_table);

      glyph = push_array<F_Glyph>(f_d2d_state->arena, 1);
      glyph->idx = glyph_idx;
      glyph->font_face = dw_font->font_face;
      glyph->bitmap_size = raster_result.bitmap_dim;
      glyph->metrics.advance = round_f32(glyph_advances[idx]);
      glyph->metrics.left_bearing = raster_result.glyph_world_bounds.left;
      glyph->size = size;
      glyph->region_uv = r4f32((F32)raster_result.atlas_region.x0 / (F32)f_d2d_state->atlas.atlas.dim.x, (F32)raster_result.atlas_region.y0 / (F32)f_d2d_state->atlas.atlas.dim.y,
                               (F32)raster_result.atlas_region.x1 / (F32)f_d2d_state->atlas.atlas.dim.x, (F32)raster_result.atlas_region.y1 / (F32)f_d2d_state->atlas.atlas.dim.y);
      sll_stack_push_n(f_d2d_state->glyph_from_idx_lookup_table[slot_idx], glyph, hash_next);
    }

    // hampus: fill in glyph run data

    F_GlyphRunNode *glyph_run_node = push_array<F_GlyphRunNode>(arena, 1);
    glyph_run_node->bitmap_size = glyph->bitmap_size;
    glyph_run_node->metrics = glyph->metrics;
    glyph_run_node->region_uv = glyph->region_uv;

    dll_push_back(result.first, result.last, glyph_run_node);
  }
  return result;
}

static F_GlyphRun
f_make_simple_glyph_run(Arena *arena, F_Tag tag, U32 size, String8 string)
{
  TempArena scratch = GetScratch(&arena, 1);
  String32 str32 = str32_from_str8(scratch.arena, string);
  F_GlyphRun result = f_make_simple_glyph_run(arena, tag, size, str32);
  return result;
}

static F_GlyphRun
f_make_complex_glyph_run(Arena *arena, F_Tag tag, U32 size, String32 str32)
{
  ProfileFunction();

  F_GlyphRun result = {};

  TempArena scratch = GetScratch(&arena, 1);
  String16 str16 = cstr16_from_str32(scratch.arena, str32);
  String16 tag_str16 = str16_from_str8(scratch.arena, tag.string);

  F_DWrite_MapTextToGlyphsResult map_text_to_glyphs_result = f_dwrite_map_text_to_glyphs(f_d2d_state->font_fallback1,
                                                                                         f_d2d_state->font_collection,
                                                                                         f_d2d_state->text_analyzer1,
                                                                                         f_d2d_state->locale,
                                                                                         tag_str16,
                                                                                         size, str16);

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

        F_Glyph *glyph = f_dwrite_lookup_glyph(glyph_idx, font_face, size);

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

          // hampus: rasterize glyph

          F_DWrite_RasterResult raster_result = f_dwrite_raster_glyph(dwrite_glyph_run, font_face, size);

          // hampus: fill in glyph data

          U64 slot_idx = glyph_idx % array_count(f_d2d_state->glyph_from_idx_lookup_table);

          glyph = push_array<F_Glyph>(f_d2d_state->arena, 1);
          glyph->idx = glyph_idx;
          glyph->font_face = font_face;
          glyph->bitmap_size = raster_result.bitmap_dim;
          glyph->metrics.advance = round_f32(segment.glyph_advances[idx]);
          glyph->metrics.left_bearing = raster_result.glyph_world_bounds.left;
          glyph->size = size;
          glyph->region_uv = r4f32((F32)raster_result.atlas_region.x0 / (F32)f_d2d_state->atlas.atlas.dim.x, (F32)raster_result.atlas_region.y0 / (F32)f_d2d_state->atlas.atlas.dim.y,
                                   (F32)raster_result.atlas_region.x1 / (F32)f_d2d_state->atlas.atlas.dim.x, (F32)raster_result.atlas_region.y1 / (F32)f_d2d_state->atlas.atlas.dim.y);
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
        Assert(!"NOT IMPLEMENTED");
      }
    }
  }

  return result;
}

static F_GlyphRun
f_make_complex_glyph_run(Arena *arena, F_Tag tag, U32 size, String8 string)
{
  TempArena scratch = GetScratch(&arena, 1);
  String32 str32 = str32_from_str8(scratch.arena, string);
  F_GlyphRun result = f_make_complex_glyph_run(arena, tag, size, str32);
  return result;
}

static F32
f_get_advance(F_Tag tag, U32 size, String32 string)
{
  F32 result = {};
  TempArena scratch = GetScratch(0, 0);
  F_GlyphRun glyph_run = f_make_complex_glyph_run(scratch.arena, tag, size, string);
  for(F_GlyphRunNode *n = glyph_run.first; n != 0; n = n->next)
  {
    result += n->metrics.advance;
  }
  return result;
}

static F32
f_get_advance(F_Tag tag, U32 size, String8 string)
{
  ProfileFunction();
  F32 result = {};
  TempArena scratch = GetScratch(0, 0);
  F_GlyphRun glyph_run = f_make_complex_glyph_run(scratch.arena, tag, size, string);
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
  TempArena scratch = GetScratch(0, 0);
  F_GlyphRun glyph_run = f_make_complex_glyph_run(scratch.arena, tag, size, str32(&cp, 1));
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
  F_DWrite_Font *dwrite_font = f_dwrite_font_from_handle(f_handle);
  Assert(dwrite_font != 0);
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