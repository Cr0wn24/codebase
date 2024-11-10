#ifndef OS_INC_H
#define OS_INC_H

#include "os/core/os_core.h"
#include "os/gfx/os_gfx.h"
#include "os/net/os_net_core.h"

#if OS_WINDOWS
#  include "os/core/os_core_win32.h"
#  include "os/net/os_net_win32.h"
#  if OS_FEATURE_GRAPHICAL
#    include "os/gfx/os_gfx_win32.h"
#  endif
#elif OS_LINUX
#  include "os/core/os_core_linux.h"
#  include "os/net/os_net_linux.h"
#  if OS_FEATURE_GRAPHICAL
#    include "os/gfx/os_gfx_linux.h"
#  endif
#elif OS_ANDROID
#  include "os/core/os_core_android.h"
#  if OS_FEATURE_GRAPHICAL
#    include "os/gfx/os_gfx_android.h"
#  endif
#endif

#endif // OS_INC_H
