#include "font_provider/font_provider_core.cpp"
#if FONT_PROVIDER_STBTT
#  include "font_provider/font_provider_stb_truetype.cpp"
#elif FONT_PROVIDER_DWRITE
#  include "font_provider/font_provider_directwrite.cpp"
#endif
