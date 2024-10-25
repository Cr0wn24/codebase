#ifndef FONT_PROVIDER_STB_TRUETYPE_H
#define FONT_PROVIDER_STB_TRUETYPE_H

struct FP_STBTT_State
{
  Arena *arena;
  FP_FileReadProc *read_file;
};

#endif // FONT_PROVIDER_STB_TRUETYPE_H
