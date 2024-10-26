#ifndef FONT_PROVIDER_H
#define FONT_PROVIDER_H

#define FONT_PROVIDER_DWRITE 1

#include "font_provider/font_provider_core.h"
#if FONT_PROVIDER_STBTT
#  include "font_provider/font_provider_stb_truetype.h"
#elif FONT_PROVIDER_DWRITE
#  include "font_provider/font_provider_directwrite.h"
#endif

#endif // FONT_PROVIDER_H