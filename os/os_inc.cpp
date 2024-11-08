#include "os/core/os_core.cpp"
#include "os/net/os_net_core.cpp"

#if OS_WINDOWS
# include "os/core/os_core_win32.cpp"
# include "os/net/os_net_win32.cpp"
# if OS_FEATURE_GRAPHICAL
#  include "os/gfx/os_gfx_win32.cpp"
# endif
#elif OS_LINUX
# include "os/core/os_core_linux.cpp"
# include "os/net/os_net_linux.cpp"
# if OS_FEATURE_GRAPHICAL
#  include "os/gfx/os_gfx_linux.cpp"
# endif
#elif OS_ANDROID
# include "os/core/os_core_android.cpp"
# if OS_FEATURE_GRAPHICAL
#  include "os/gfx/os_gfx_android.cpp"
# endif
#endif