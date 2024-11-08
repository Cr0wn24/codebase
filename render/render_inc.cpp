#include "render/render_core.cpp"
#if OS_WINDOWS
# include "render/render_core_d3d11.cpp"
#elif OS_ANDROID
# include "render/render_core_gles.cpp"
#else
# error No render include exists for this os
#endif
