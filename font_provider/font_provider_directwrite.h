#ifndef FONT_PROVIDER_DIRECTWRITE
#define FONT_PROVIDER_DIRECTWRITE

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "dwrite.lib")

#pragma warning(push, 0)
#include <dwrite_1.h>
#pragma warning(pop)

static void directwrite_test();

struct FP_DWrite_Font
{
  IDWriteFontFile *font_file;
  IDWriteFontFace *font_face;
};

struct FP_DWrite_State
{
  Arena *arena;
  IDWriteFactory *factory;
  IDWriteRenderingParams *base_rendering_params;
  IDWriteRenderingParams *rendering_params;
  IDWriteGdiInterop *dwrite_gdi_interop;
  FP_FileReadProc *read_file;
};

#endif