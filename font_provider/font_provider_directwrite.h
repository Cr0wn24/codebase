#ifndef FONT_PROVIDER_DIRECTWRITE
#define FONT_PROVIDER_DIRECTWRITE

#pragma warning(push, 0)
#include <dwrite_1.h>
#pragma warning(pop)

function void directwrite_test();

struct FP_DW_Font
{
  IDWriteFactory *factory;
  IDWriteFontFile *font_file;
  IDWriteFontFace *font_face;
};

#endif