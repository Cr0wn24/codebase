static FP_DWrite_State *fp_dwrite_state;

static void
fp_init()
{
  profile_function();

  Arena *arena = arena_alloc();
  fp_dwrite_state = push_array<FP_DWrite_State>(arena, 1);
  fp_dwrite_state->arena = arena;
  HRESULT error = 0;

  error = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                              __uuidof(IDWriteFactory),
                              (IUnknown **)&fp_dwrite_state->factory);
  ASSERT(error == S_OK);

  // DWrite Rendering Params
  error = fp_dwrite_state->factory->CreateRenderingParams(&fp_dwrite_state->base_rendering_params);
  ASSERT(error == S_OK);

  {
    FLOAT gamma = 1.0f;
#if 1
    // FLOAT gamma = base_rendering_params->GetGamma();
    FLOAT enhanced_contrast = fp_dwrite_state->base_rendering_params->GetEnhancedContrast();
    FLOAT clear_type_level = fp_dwrite_state->base_rendering_params->GetClearTypeLevel();
    error = fp_dwrite_state->factory->CreateCustomRenderingParams(gamma,
                                                                  enhanced_contrast,
                                                                  clear_type_level,
                                                                  DWRITE_PIXEL_GEOMETRY_FLAT,
                                                                  DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC,
                                                                  &fp_dwrite_state->rendering_params);
#else
    // FLOAT gamma = base_rendering_params->GetGamma();
    FLOAT enhanced_contrast = fp_dwrite_state->base_rendering_params->GetEnhancedContrast();
    FLOAT clear_type_level = fp_dwrite_state->base_rendering_params->GetClearTypeLevel();
    error = fp_dwrite_state->factory->CreateCustomRenderingParams(gamma,
                                                                  enhanced_contrast,
                                                                  clear_type_level,
                                                                  DWRITE_PIXEL_GEOMETRY_FLAT,
                                                                  DWRITE_RENDERING_MODE_GDI_NATURAL,
                                                                  &fp_dwrite_state->rendering_params);
#endif
    ASSERT(error == S_OK);
  }

  // DWrite GDI Interop
  error = fp_dwrite_state->factory->GetGdiInterop(&fp_dwrite_state->dwrite_gdi_interop);
  ASSERT(error == S_OK);

  fp_dwrite_state->read_file = os_file_read;
}

static void
fp_set_file_read_proc(FP_FileReadProc *proc)
{
  fp_dwrite_state->read_file = proc;
}

static FP_Handle
fp_font_open_file(Arena *arena, String8 path)
{
  profile_function();

  FP_Handle result = fp_handle_zero();
  TempArena scratch = get_scratch(&arena, 1);
  FP_DWrite_Font *dw_font = push_array<FP_DWrite_Font>(arena, 1);
  HRESULT error = 0;

  error = fp_dwrite_state->factory->CreateFontFileReference((const WCHAR *)cstr16_from_str8(scratch.arena, path),
                                                            0, &dw_font->font_file);
  ASSERT(error == S_OK);

  // DWrite Font Face
  error = fp_dwrite_state->factory->CreateFontFace(DWRITE_FONT_FACE_TYPE_TRUETYPE,
                                                   1,
                                                   &dw_font->font_file,
                                                   0,
                                                   DWRITE_FONT_SIMULATIONS_NONE,
                                                   &dw_font->font_face);
  ASSERT(error == S_OK);
  result.u64[0] = int_from_ptr(dw_font);
  return result;
}

static FP_Handle
fp_font_open_static_data_string(Arena *arena, String8 *data_ptr)
{
  not_implemented;
  FP_Handle result = {};
  return result;
}

static void
fp_font_close(FP_Handle font)
{
  ASSERT(!fp_handle_match(font, fp_handle_zero()));
  FP_DWrite_Font *dw_font = (FP_DWrite_Font *)ptr_from_int(font.u64[0]);
  dw_font->font_face->Release();
  dw_font->font_file->Release();
  fp_dwrite_state->factory->Release();
}

static FP_RasterResult
fp_raster(Arena *arena, FP_Handle font, U32 size, U32 cp)
{
  profile_function();

  FP_RasterResult result = {};
  TempArena scratch = get_scratch(&arena, 1);
  ASSERT(!fp_handle_match(font, fp_handle_zero()));
  FP_DWrite_Font *dw_font = (FP_DWrite_Font *)ptr_from_int(font.u64[0]);
  HRESULT error = 0;

  // Find the glyph index for the codepoint we want to render
  uint16_t index = 0;
  error = dw_font->font_face->GetGlyphIndices(&cp, 1, &index);
  ASSERT(error == S_OK);

  Vec2U32 raster_target_dim = v2u32(512, 512);

  ASSERT(error == S_OK);

  // DWrite Bitmap Render Target
  IDWriteBitmapRenderTarget *render_target = 0;
  error = fp_dwrite_state->dwrite_gdi_interop->CreateBitmapRenderTarget(0, raster_target_dim.x, raster_target_dim.y, &render_target);

  ASSERT(error == S_OK);

  // Clear the Render Target
  HDC dc = render_target->GetMemoryDC();

  {
    COLORREF back_color = RGB(0, 0, 0);
    HGDIOBJ original = SelectObject(dc, GetStockObject(DC_PEN));
    SetDCPenColor(dc, back_color);
    SelectObject(dc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(dc, back_color);
    Rectangle(dc, 0, 0, safe_s32_from_u32(raster_target_dim.x), safe_s32_from_u32(raster_target_dim.y));
    SelectObject(dc, original);
  }

  // Render the glyph
  DWRITE_GLYPH_RUN glyph_run = {};
  glyph_run.fontFace = dw_font->font_face;
  glyph_run.fontEmSize = (F32)size * 96.0f / 72.0f;
  glyph_run.glyphCount = 1;
  glyph_run.glyphIndices = &index;
  RECT bounding_box = {};
  error = render_target->DrawGlyphRun(100, 100,
                                      DWRITE_MEASURING_MODE_NATURAL,
                                      &glyph_run,
                                      fp_dwrite_state->rendering_params,
                                      RGB(255, 0, 0),
                                      &bounding_box);

  ASSERT(error == S_OK);

  // Get the Bitmap
  HBITMAP bitmap = (HBITMAP)GetCurrentObject(dc, OBJ_BITMAP);
  DIBSECTION dib = {};
  GetObject(bitmap, sizeof(dib), &dib);

  // Get font metrics
  DWRITE_FONT_METRICS font_metrics = {};
  if(dw_font->font_face != 0)
  {
    dw_font->font_face->GetMetrics(&font_metrics);
  }

  F32 design_units_per_em = (F32)font_metrics.designUnitsPerEm;
  F32 font_scale = (F32)size * (96.0f / 72.0f) / design_units_per_em;

  U64 ascent = (U64)((F32)font_metrics.ascent * font_scale);
  U64 descent = (U64)((F32)font_metrics.descent * font_scale);

  // Rasterize into our own RGBA bitmap
  {
    U8 *bitmap_memory = (U8 *)dib.dsBm.bmBits;
    U64 bbox_width = (U64)(bounding_box.right - bounding_box.left);
    U64 bbox_height = (U64)(bounding_box.bottom - bounding_box.top);

    result.dim = v2u64(bbox_width, ascent + descent);
    result.memory = push_array<U8>(arena, result.dim.x * result.dim.y * 4);
    result.left_bearing = (F32)bounding_box.left - 100.0f;

    U64 src_pitch = raster_target_dim.x * 4;
    U8 *src_line = bitmap_memory + bounding_box.top * src_pitch + bounding_box.left * 4;
    U64 dst_pitch = result.dim.x * 4;
    U64 bbox_distance_over_baseline = (U64)(100 - bounding_box.top);
    U8 *dst_line = (U8 *)result.memory + (ascent - bbox_distance_over_baseline) * dst_pitch;
    for(U64 y = 0; y < bbox_height; ++y)
    {
      U8 *src = src_line;
      U8 *dst = dst_line;
      for(U64 x = 0; x < bbox_width; ++x)
      {
        dst[0] = 0xff;
        dst[1] = 0xff;
        dst[2] = 0xff;
        dst[3] = src[2];
        src += 4;
        dst += 4;
      }
      src_line += src_pitch;
      dst_line += dst_pitch;
    }
  }

  render_target->Release();

  return result;
}

static FP_FontMetrics
fp_get_font_metrics(FP_Handle font, U32 size)
{
  profile_function();

  FP_FontMetrics result = {};
  FP_DWrite_Font *dw_font = (FP_DWrite_Font *)ptr_from_int(font.u64[0]);

  //- rjf: get font metrics
  DWRITE_FONT_METRICS font_metrics = {0};
  if(dw_font->font_face != 0)
  {
    dw_font->font_face->GetMetrics(&font_metrics);
  }
  F32 design_units_per_em = (F32)font_metrics.designUnitsPerEm;
  F32 font_scale = (F32)size * (96.f / 72.f) / design_units_per_em;

  F32 line_gap = floor_f32((F32)font_metrics.lineGap * font_scale);
  F32 ascent = floor_f32((F32)font_metrics.ascent * font_scale);
  F32 descent = floor_f32((F32)font_metrics.descent * font_scale);

  result.line_gap = line_gap;
  result.descent = descent;
  result.ascent = ascent;
  return result;
}

static FP_GlyphMetrics
fp_get_glyph_metrics(FP_Handle font, U32 size, U32 cp)
{
  profile_function();

  FP_GlyphMetrics result = {};
  ASSERT(!fp_handle_match(font, fp_handle_zero()));
  FP_DWrite_Font *dw_font = (FP_DWrite_Font *)ptr_from_int(font.u64[0]);

  HRESULT error = 0;
  // Find the glyph index for the codepoint we want to render
  uint16_t index = 0;
  error = dw_font->font_face->GetGlyphIndices(&cp, 1, &index);
  ASSERT(error == S_OK);

  // Get font metrics
  DWRITE_FONT_METRICS font_metrics = {0};
  if(dw_font->font_face != 0)
  {
    dw_font->font_face->GetMetrics(&font_metrics);
  }
  F32 design_units_per_em = (F32)font_metrics.designUnitsPerEm;

  // Get metrics info
  U32 glyphs_count = 1;
  DWRITE_GLYPH_METRICS glyphs_metrics = {};

  error = dw_font->font_face->GetDesignGlyphMetrics(&index, glyphs_count, &glyphs_metrics);
  ASSERT(error == S_OK);

  F32 font_scale = (F32)size * (96.0f / 72.0f) / design_units_per_em;

  F32 advance = (F32)glyphs_metrics.advanceWidth * font_scale;
  F32 left_side_bearing = (F32)glyphs_metrics.leftSideBearing * font_scale;

  result.advance = round_f32(advance);
  result.left_bearing = left_side_bearing;

  return result;
}