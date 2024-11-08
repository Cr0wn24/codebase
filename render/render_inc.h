#ifndef RENDER_INC_H
#define RENDER_INC_H

#include "render/render_core.h"
#if OS_WINDOWS
# include "render/render_core_d3d11.h"
#elif OS_ANDROID
# include "render/render_core_gles.h"
#else
# error No render include exists for this os
#endif

#endif // RENDER_INC_H
