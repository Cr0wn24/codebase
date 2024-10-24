#pragma comment(lib, "dwrite.lib")

#pragma pack(push, 1)
struct Header
{
  char sig[2];
  uint32_t file_size;
  uint32_t reserved;
  uint32_t data_offset;
};

struct Info_Header
{
  uint32_t size;
  uint32_t width;
  uint32_t height;
  uint16_t planes;
  uint16_t bits_per_pixel;
  uint32_t compression;
  uint32_t image_size;
  uint32_t x_pixels_per_meter;
  uint32_t y_pixels_per_meter;
  uint32_t colors_used;
  uint32_t important_colors;
};

struct Color_Table
{
  uint8_t R;
  uint8_t G;
  uint8_t B;
  uint8_t A;
};
#pragma pack(pop)

function void
directwrite_test()
{
  COLORREF back_color = RGB(0, 0, 0);
  COLORREF fore_color = RGB(255, 0, 0);
  int32_t raster_target_w = 200;
  int32_t raster_target_h = 200;
  float raster_target_x = 100.f;
  float raster_target_y = 100.f;
  wchar_t font_path[] = L"C:\\Windows\\Fonts\\arial.ttf";
  float point_size = 12.f;
  uint32_t codepoint = 'M';
  const char *test_output_file_name = "test.bmp";

  HRESULT error = 0;

  // DWrite Factory
  IDWriteFactory *dwrite_factory = 0;
  error = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)&dwrite_factory);
  ASSERT(error == S_OK);

  // DWrite Font File Reference
  IDWriteFontFile *font_file = 0;
  error = dwrite_factory->CreateFontFileReference(font_path, 0, &font_file);
  ASSERT(error == S_OK);

  // DWrite Font Face
  IDWriteFontFace *font_face = 0;
  error = dwrite_factory->CreateFontFace(DWRITE_FONT_FACE_TYPE_TRUETYPE, 1, &font_file, 0, DWRITE_FONT_SIMULATIONS_NONE, &font_face);
  ASSERT(error == S_OK);

  // DWrite Rendering Params
  IDWriteRenderingParams *base_rendering_params = 0;
  error = dwrite_factory->CreateRenderingParams(&base_rendering_params);
  ASSERT(error == S_OK);

  IDWriteRenderingParams *rendering_params = 0;
  {
    FLOAT gamma = 1.f;
    // FLOAT gamma = base_rendering_params->GetGamma();
    FLOAT enhanced_contrast = base_rendering_params->GetEnhancedContrast();
    FLOAT clear_type_level = base_rendering_params->GetClearTypeLevel();
    error = dwrite_factory->CreateCustomRenderingParams(gamma,
                                                        enhanced_contrast,
                                                        clear_type_level,
                                                        DWRITE_PIXEL_GEOMETRY_RGB,
                                                        DWRITE_RENDERING_MODE_NATURAL,
                                                        &rendering_params);
    ASSERT(error == S_OK);
  }

  // DWrite GDI Interop
  IDWriteGdiInterop *dwrite_gdi_interop = 0;
  error = dwrite_factory->GetGdiInterop(&dwrite_gdi_interop);
  ASSERT(error == S_OK);

  // DWrite Bitmap Render Target
  IDWriteBitmapRenderTarget *render_target = 0;
  error = dwrite_gdi_interop->CreateBitmapRenderTarget(0, (UINT32)raster_target_w, (UINT32)raster_target_h, &render_target);
  ASSERT(error == S_OK);

  // Clear the Render Target
  HDC dc = render_target->GetMemoryDC();

  {
    HGDIOBJ original = SelectObject(dc, GetStockObject(DC_PEN));
    SetDCPenColor(dc, back_color);
    SelectObject(dc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(dc, back_color);
    Rectangle(dc, 0, 0, raster_target_w, raster_target_h);
    SelectObject(dc, original);
  }

  // Find the glyph index for the codepoint we want to render
  uint16_t index = 0;
  error = font_face->GetGlyphIndices(&codepoint, 1, &index);
  ASSERT(error == S_OK);

  // Render the glyph
  DWRITE_GLYPH_RUN glyph_run = {0};
  glyph_run.fontFace = font_face;
  glyph_run.fontEmSize = point_size * 96.f / 72.f;
  glyph_run.glyphCount = 1;
  glyph_run.glyphIndices = &index;
  RECT bounding_box = {0};
  error = render_target->DrawGlyphRun(raster_target_x, raster_target_y, DWRITE_MEASURING_MODE_NATURAL, &glyph_run, rendering_params, fore_color, &bounding_box);
  ASSERT(error == S_OK);

  // Get the Bitmap
  HBITMAP bitmap = (HBITMAP)GetCurrentObject(dc, OBJ_BITMAP);
  DIBSECTION dib = {0};
  GetObject(bitmap, sizeof(dib), &dib);

  // Save the Bitmap
  {
    uint8_t *in_data = (uint8_t *)dib.dsBm.bmBits;

    {
      U32 *ptr = (U32 *)in_data;
      for(U64 i = 0; i < 512; ++i)
      {
        *ptr++ = 0xffffffff;
      }
    }

    int32_t in_pitch = dib.dsBm.bmWidthBytes;
    int32_t width = raster_target_w;
    int32_t height = raster_target_h;
    const char *file_name = test_output_file_name;

    void *memory = (void *)malloc(1 << 20);
    ASSERT(memory != 0);
    void *ptr = memory;
    int32_t out_pitch = (width * 3) + 3;
    out_pitch = out_pitch - (out_pitch % 4);

    Header *header = (Header *)ptr;
    ptr = header + 1;
    Info_Header *info_header = (Info_Header *)ptr;
    ptr = info_header + 1;
    uint8_t *out_data = (uint8_t *)ptr;
    ptr = out_data + out_pitch * height;

    header->sig[0] = 'B';
    header->sig[1] = 'M';
    header->file_size = uint32_t((uint8_t *)ptr - (uint8_t *)memory);
    header->reserved = 0;
    header->data_offset = uint32_t(out_data - (uint8_t *)memory);
    info_header->size = sizeof(*info_header);
    info_header->width = (uint32_t)width;
    info_header->height = (uint32_t)height;
    info_header->planes = 1;
    info_header->bits_per_pixel = 24;
    info_header->compression = 0;
    info_header->image_size = 0;
    info_header->x_pixels_per_meter = 0;
    info_header->y_pixels_per_meter = 0;
    info_header->colors_used = 0;
    info_header->important_colors = 0;

    uint8_t *in_line = (uint8_t *)in_data;
    uint8_t *out_line = out_data + out_pitch * (height - 1);
    for(int32_t y = 0; y < height; y += 1)
    {
      uint8_t *in_pixel = in_line;
      uint8_t *out_pixel = out_line;
      for(int32_t x = 0; x < width; x += 1)
      {
        out_pixel[0] = in_pixel[0];
        out_pixel[1] = in_pixel[1];
        out_pixel[2] = in_pixel[2];
        in_pixel += 4;
        out_pixel += 3;
      }
      in_line += in_pitch;
      out_line -= out_pitch;
    }

    FILE *out = fopen(file_name, "wb");
    ASSERT(out != 0);
    fwrite(memory, 1, header->file_size, out);
    fclose(out);
  }
}

function FP_Handle
fp_font_open_file(Arena *arena, String8 path)
{
  FP_Handle result = fp_handle_zero();
  TempArena scratch = get_scratch(0, 0);
  FP_DW_Font *dw_font = push_array<FP_DW_Font>(arena, 1);
  HRESULT error = 0;

  error = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                              __uuidof(IDWriteFactory),
                              (IUnknown **)&dw_font->factory);
  ASSERT(error == S_OK);

  error = dw_font->factory->CreateFontFileReference((const WCHAR *)cstr16_from_str8(scratch.arena, path),
                                                    0, &dw_font->font_file);
  ASSERT(error == S_OK);

  // DWrite Font Face
  error = dw_font->factory->CreateFontFace(DWRITE_FONT_FACE_TYPE_TRUETYPE,
                                           1,
                                           &dw_font->font_file,
                                           0,
                                           DWRITE_FONT_SIMULATIONS_NONE,
                                           &dw_font->font_face);

  ASSERT(error == S_OK);
  result.u64[0] = int_from_ptr(dw_font);
  return result;
}

function FP_Handle
fp_font_open_memory(Arena *arena, String8 memory)
{
  not_implemented;
  FP_Handle result = fp_handle_zero();
  return result;
}

function void
fp_font_close(FP_Handle font)
{
  ASSERT(!fp_handle_match(font, fp_handle_zero()));
  FP_DW_Font *dw_font = (FP_DW_Font *)ptr_from_int(font.u64[0]);
  dw_font->font_face->Release();
  dw_font->font_file->Release();
  dw_font->factory->Release();
}

function FP_RasterResult
fp_raster(Arena *arena, FP_Handle font, U32 size, U32 cp)
{
  FP_RasterResult result = {};
  TempArena scratch = get_scratch(0, 0);
  ASSERT(!fp_handle_match(font, fp_handle_zero()));
  FP_DW_Font *dw_font = (FP_DW_Font *)ptr_from_int(font.u64[0]);
  HRESULT error = 0;
  // DWrite Rendering Params
  IDWriteRenderingParams *base_rendering_params = 0;
  error = dw_font->factory->CreateRenderingParams(&base_rendering_params);
  ASSERT(error == S_OK);

  IDWriteRenderingParams *rendering_params = 0;
  {
    FLOAT gamma = 1.f;
    // FLOAT gamma = base_rendering_params->GetGamma();
    FLOAT enhanced_contrast = base_rendering_params->GetEnhancedContrast();
    FLOAT clear_type_level = base_rendering_params->GetClearTypeLevel();
    error = dw_font->factory->CreateCustomRenderingParams(gamma,
                                                          enhanced_contrast,
                                                          clear_type_level,
                                                          DWRITE_PIXEL_GEOMETRY_RGB,
                                                          DWRITE_RENDERING_MODE_NATURAL,
                                                          &rendering_params);
    ASSERT(error == S_OK);
  }

  // DWrite GDI Interop
  IDWriteGdiInterop *dwrite_gdi_interop = 0;
  error = dw_font->factory->GetGdiInterop(&dwrite_gdi_interop);
  ASSERT(error == S_OK);

  // Find the glyph index for the codepoint we want to render
  uint16_t index = 0;
  error = dw_font->font_face->GetGlyphIndices(&cp, 1, &index);
  ASSERT(error == S_OK);

  //- rjf: get font metrics
  DWRITE_FONT_METRICS font_metrics = {0};
  if(dw_font->font_face != 0)
  {
    dw_font->font_face->GetMetrics(&font_metrics);
  }
  F32 design_units_per_em = (F32)font_metrics.designUnitsPerEm;

  Vec2U32 raster_target_dim = v2u32(512, 512);

  ASSERT(error == S_OK);

  // DWrite Bitmap Render Target
  IDWriteBitmapRenderTarget *render_target = 0;
  error = dwrite_gdi_interop->CreateBitmapRenderTarget(0, raster_target_dim.x, raster_target_dim.y, &render_target);

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
  glyph_run.fontEmSize = (F32)size * 96.f / 72.f;
  glyph_run.glyphCount = 1;
  glyph_run.glyphIndices = &index;
  RECT bounding_box = {};
  error = render_target->DrawGlyphRun(100, 100,
                                      DWRITE_MEASURING_MODE_NATURAL,
                                      &glyph_run,
                                      rendering_params,
                                      RGB(255, 0, 0),
                                      &bounding_box);
  ASSERT(error == S_OK);

  //- rjf: get metrics info
  U32 glyphs_count = 1;
  DWRITE_GLYPH_METRICS *glyphs_metrics = push_array_no_zero<DWRITE_GLYPH_METRICS>(scratch.arena, glyphs_count);
  error = dw_font->font_face->GetDesignGlyphMetrics(&index,
                                                    glyphs_count,
                                                    glyphs_metrics,
                                                    false);
  F32 advance_width = ceil_f32((F32)size * (F32)glyphs_metrics->advanceWidth * (96.f / 72.f) / design_units_per_em);
  F32 advance_height = ceil_f32((F32)size * (F32)(glyphs_metrics->advanceHeight) * (96.f / 72.f) / design_units_per_em);

  // Get the Bitmap
  HBITMAP bitmap = (HBITMAP)GetCurrentObject(dc, OBJ_BITMAP);
  DIBSECTION dib = {};
  GetObject(bitmap, sizeof(dib), &dib);

  U8 *bitmap_memory = (U8 *)dib.dsBm.bmBits;

  result.dim = v2u64((U64)(bounding_box.right - bounding_box.left), (U64)(bounding_box.bottom - bounding_box.top));
  result.memory = push_array_no_zero<U8>(arena, raster_target_dim.x * raster_target_dim.y * 4);

  memory_copy(result.memory, dib.dsBm.bmBits, result.dim.x * result.dim.y * 4);

  render_target->Release();
  dwrite_gdi_interop->Release();
  rendering_params->Release();
  base_rendering_params->Release();

  return result;
}

function FP_FontMetrics
fp_metrics_from_font_size(FP_Handle font, U32 size)
{
  FP_FontMetrics result = {};
  return result;
}

function FP_GlyphMetrics
fp_metrics_From_font_size_cp(FP_Handle font, U32 size, U32 cp)
{
  FP_GlyphMetrics result = {};
  return result;
}