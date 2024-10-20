#ifndef BASE_MARKUP_H
#define BASE_MARKUP_H

#if !defined(PROFILING_ENABLED)
#  define PROFILING_ENABLED 0
#endif

#if PROFILING_ENABLED

#  include "third_party/spall/spall.h"

static SpallProfile spall_ctx;
static SpallBuffer spall_buffer;

#  define profile_scope_begin(name) spall_buffer_begin_ex(&spall_ctx, &spall_buffer, name, sizeof(name) - 1, (F64)os_get_microseconds(), os_get_current_thread_id(), 0)

#  define profile_scope_end() spall_buffer_end_ex(&spall_ctx, &spall_buffer, (F64)os_get_microseconds(), os_get_current_thread_id(), 0)

#  define profile_function_begin() profile_scope_begin(__FUNCTION__)
#  define profile_function_end() profile_scope_end()

#  define profile_scope(name) defer_loop(profile_scope_begin(name), profile_scope_end())

#  define profile_init(string) spall_ctx = spall_init_file(string, 1)
#  define profile_quit() spall_quit(&spall_ctx)

#  define profile_init_thread()                          \
    {                                                    \
      int buffer_size = 1024;                            \
      spall_buffer = (SpallBuffer){                      \
        .length = buffer_size,                           \
        .data = malloc(buffer_size),                     \
      };                                                 \
      memset(spall_buffer.data, 1, spall_buffer.length); \
      spall_buffer_init(&spall_ctx, &spall_buffer);      \
    }
#  define profile_quit_thread() spall_buffer_quit(&spall_ctx, &spall_buffer)

#else

#  define profile_scope_begin(name)

#  define profile_scope_end()

#  define profile_function_begin()
#  define profile_function_end()

#  define profile_scope(name)

#  define profile_init(string)
#  define profile_quit()

#  define profile_init_thread()

#  define profile_quit_thread()

#endif

#endif // BASE_MARKUP_H
