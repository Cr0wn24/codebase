#ifndef BASE_MARKUP_H
#define BASE_MARKUP_H

#if !defined(PROFILING_ENABLED)
# define PROFILING_ENABLED 0
#endif

#if PROFILING_ENABLED

function U64 os_get_microseconds(void);
function U32 os_get_current_thread_id(void);

# pragma warning(push, 0)
# include "third_party/spall/spall.h"
# pragma warning(pop)

static SpallProfile spall_ctx;
static SpallBuffer spall_buffer;

# define profile_scope_begin_ex(name, size) spall_buffer_begin_ex(&spall_ctx, &spall_buffer, name, size, (F64)os_get_microseconds(), os_get_current_thread_id(), 0)

# define profile_scope_end_ex() spall_buffer_end_ex(&spall_ctx, &spall_buffer, (F64)os_get_microseconds(), os_get_current_thread_id(), 0)

# define profile_scope_begin(name) spall_buffer_begin(&spall_ctx, &spall_buffer, name, (F64)os_get_microseconds(), os_get_current_thread_id(), 0)

# define profile_scope_end() spall_buffer_end(&spall_ctx, &spall_buffer, (F64)os_get_microseconds(), os_get_current_thread_id(), 0)

# define profile_function_begin() profile_scope_begin(__FUNCTION__)
# define profile_function_end() profile_scope_end()

# define profile_init(string) spall_ctx = spall_init_file(string, 1)
# define profile_quit() spall_quit(&spall_ctx)

# define profile_init_thread()                        \
  {                                                   \
   size_t buffer_size = 1024;                         \
   spall_buffer.data = malloc(buffer_size);           \
   spall_buffer.length = buffer_size;                 \
   memset(spall_buffer.data, 1, spall_buffer.length); \
   spall_buffer_init(&spall_ctx, &spall_buffer);      \
  }
# define profile_quit_thread() spall_buffer_quit(&spall_ctx, &spall_buffer)

#else

# define profile_scope_begin_ex(name, size)

# define profile_scope_end_ex()

# define profile_scope_begin(name)

# define profile_scope_end()

# define profile_function_begin()
# define profile_function_end()

# define profile_init(string)
# define profile_quit()

# define profile_init_thread()

# define profile_quit_thread()

#endif

struct ProfileBlock
{
 ProfileBlock(const char *name, S32 size)
 {
  profile_scope_begin_ex(name, size);
 }
 ~ProfileBlock()
 {
  profile_scope_end_ex();
 }
};

#define profile_scope(name) ProfileBlock glue(profile_block, __LINE__)(name, sizeof(name) - 1);
#define profile_function() profile_scope(__func__)

#endif // BASE_MARKUP_H
