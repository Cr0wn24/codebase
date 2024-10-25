//////////////////////////////
// NOTE(hampus): Memory allocation

static void *
os_memory_reserve(U64 size)
{
  not_implemented;
  void *result = 0;
  return result;
}

static void
os_memory_commit(void *ptr, U64 size)
{
  not_implemented;
}

static void
os_memory_decommit(void *ptr, U64 size)
{
  not_implemented;
}

static void
os_memory_release(void *ptr, U64 size)
{
  not_implemented;
}

static void *
os_memory_alloc(U64 size)
{
  not_implemented;
  void *result = 0;
  return result;
}

//////////////////////////////
// NOTE(hampus): File attributes

static OS_FileAttributes
os_get_file_attributes(String8 path)
{
  not_implemented;
  OS_FileAttributes result = {};
  return result;
}

//////////////////////////////
// NOTE(hampus): Basic file operations

static String8
os_file_read(Arena *arena, String8 path)
{
  not_implemented;
  String8 result = {};
  return result;
}

static B32
os_file_write(String8 path, String8 data)
{
  not_implemented;
  B32 result = false;
  return result;
}

static B32
os_file_copy(String8 old_path, String8 new_path)
{
  not_implemented;
  B32 result = false;
  return result;
}

static B32
os_file_rename(String8 old_path, String8 new_path)
{
  not_implemented;
  B32 result = false;
  return result;
}

static B32
os_file_delete(String8 path)
{
  not_implemented;
  B32 result = false;
  return result;
}

//////////////////////////////
// NOTE(hampus): File stream

static OS_Handle
os_file_stream_open(String8 path)
{
  not_implemented;
  OS_Handle result = os_handle_zero();
  return result;
}

static B32
os_file_stream_close(OS_Handle file)
{
  not_implemented;
  B32 result = false;
  return result;
}

static B32
os_file_stream_write(OS_Handle file, String8 data)
{
  not_implemented;
  B32 result = false;
  return result;
}

//////////////////////////////
// NOTE(hampus): Directory operations

static B32
os_directory_create(String8 path)
{
  not_implemented;
  B32 result = false;
  return result;
}

static B32
os_directory_delete(String8 path)
{
  not_implemented;
  B32 result = false;
  return result;
}

static OS_Handle
os_file_iterator_init(String8 path)
{
  not_implemented;
  OS_Handle result = os_handle_zero();
  return result;
}

static void
os_file_iterator_end(OS_Handle iterator)
{
  not_implemented;
}

static B32
os_file_iterator_next(Arena *arena, OS_Handle iterator, String8 *result_name)
{
  not_implemented;
  B32 result = false;
  return result;
}

static String8
os_get_executable_path(Arena *arena)
{
  not_implemented;
  String8 result = {};
  return result;
}

//////////////////////////////
// NOTE(hampus): Time

static DateTime
os_get_universal_time()
{
  not_implemented;
  DateTime result = {};
  return result;
}

static DateTime
os_get_local_time()
{
  not_implemented;
  DateTime result = {};
  return result;
}

static Date
os_get_local_date()
{
  not_implemented;
  Date result = {};
  return result;
}

static DateTime
os_local_time_from_universal(DateTime *date_time)
{
  not_implemented;
  DateTime result = {};
  return result;
}

static DateTime
os_universal_time_from_local(DateTime *date_time)
{
  not_implemented;
  DateTime result = {};
  return result;
}

static Date
os_increment_date_by_day(Date date, S64 days)
{
  not_implemented;
  Date result = {};
  return result;
}

static U64
os_get_microseconds()
{
  not_implemented;
  U64 result = 0;
  return result;
}

static void
os_sleep(U64 time)
{
  not_implemented;
}

static void
os_wait_microseconds(U64 end_time_us)
{
  not_implemented;
}

//////////////////////////////
// NOTE(hampus): Library

static OS_Handle
os_library_open(String8 path)
{
  not_implemented;
  OS_Handle result = os_handle_zero();
  return result;
}

static void
os_library_close(OS_Handle library)
{
  not_implemented;
}

static void *
os_libary_load_static(OS_Handle library, String8 name)
{
  not_implemented;
  void *result = 0;
  return result;
}

//////////////////////////////
// NOTE(hampus): Threading

static OS_Handle
os_semaphore_alloc(U32 initial_value)
{
  not_implemented;
  OS_Handle result = os_handle_zero();
}

static void
os_semaphore_free(OS_Handle handle)
{
  not_implemented;
}

static void
os_semaphore_signal(OS_Handle handle)
{
  not_implemented;
}

static void
os_semaphore_wait(OS_Handle handle)
{
  not_implemented;
}

static OS_Handle
os_mutex_alloc()
{
  not_implemented;
  OS_Handle result = os_handle_zero();
}

static void
os_mutex_free(OS_Handle handle)
{
  not_implemented;
}

static void
os_mutex_take(OS_Handle handle)
{
  not_implemented;
}

static void
os_mutex_release(OS_Handle handle)
{
  not_implemented;
}

static OS_Handle
os_thread_create(ThreadProc *proc, void *data)
{
  not_implemented;
  OS_Handle result = os_handle_zero();
  return result;
}

static void
os_thread_join(OS_Handle handle)
{
  not_implemented;
}

static void
os_thread_set_name(String8 string)
{
  not_implemented;
}

static U32
os_get_current_thread_id()
{
  not_implemented;
  U32 result = 0;
  return result;
}

//////////////////////////////
// NOTE(hampus): Debug output

static void
os_print_debug_string(String8 string)
{
  not_implemented;
}

static void
os_print_debug_string(char *fmt, ...)
{
  not_implemented;
}

//////////////////////////////
// NOTE(hampus): Exit

static void
os_exit(S32 exit_code)
{
  abort();
}

int
main(int argc, char **argv)
{
  not_implemented;
  return 0;
}