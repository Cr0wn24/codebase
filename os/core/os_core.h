#ifndef OS_CORE_H
#define OS_CORE_H

//////////////////////////////
// NOTE(hampus): Handle types

struct OS_Handle
{
  U64 u64[1];
};

//////////////////////////////
// NOTE(hampus): File attributes

struct OS_FileAttributes
{
  B8 directory;
  B8 hidden;
  B8 writable;
};

//////////////////////////////
// NOTE(hampus): Memory

struct OS_MemoryStats
{
  MemorySize reserved;
  MemorySize commited;
};

//////////////////////////////
// NOTE(hampus): Backtrace types

struct OS_BacktraceEntry
{
  String8 module_path;
  U64 address;
};

struct OS_Backtrace
{
  StaticArray<OS_BacktraceEntry, 64> entries;
  U64 entries_count;
};

//////////////////////////////
// NOTE(hampus): Thread types

typedef void ThreadProc(void *);

//////////////////////////////
// NOTE(hampus): Handle functions

function OS_Handle os_handle_zero(void);
function B32 os_handle_match(OS_Handle a, OS_Handle b);

//////////////////////////////
// NOTE(hampus): Memory allocation

function OS_MemoryStats os_get_memory_stats(void);
function void *os_memory_reserve(U64 size);
function void os_memory_commit(void *ptr, U64 size);
function void os_memory_decommit(void *ptr, U64 size);
function void os_memory_release(void *ptr, U64 size);
function void *os_memory_alloc(U64 size);

//////////////////////////////
// NOTE(hampus): File attributes

function OS_FileAttributes os_get_file_attributes(String8 path);

//////////////////////////////
// NOTE(hampus): Basic file operations

function String8 os_read_file(Arena *arena, String8 path);
function B32 os_file_write(String8 path, String8 data);
function B32 os_file_copy(String8 old_path, String8 new_path);
function B32 os_file_rename(String8 old_path, String8 new_path);
function B32 os_file_delete(String8 path);

//////////////////////////////
// NOTE(hampus): File stream

function OS_Handle os_file_stream_open(String8 path);
function B32 os_file_stream_close(OS_Handle file);
function B32 os_file_stream_write(OS_Handle file, String8 data);

//////////////////////////////
// NOTE(hampus): Directory operations

function B32 os_directory_create(String8 path);
function B32 os_directory_delete(String8 path);
function OS_Handle os_file_iterator_init(String8 path);
function void os_file_iterator_end(OS_Handle iterator);
function B32 os_file_iterator_next(Arena *arena, OS_Handle iterator, String8 *result_name);
function String8 os_get_executable_path(Arena *arena);

//////////////////////////////
// NOTE(hampus): Time

function DateTime os_get_universal_time(void);
function DateTime os_get_local_time(void);
function Date os_get_local_date(void);
function DateTime os_local_time_from_universal(DateTime *date_time);
function DateTime os_universal_time_from_local(DateTime *date_time);
function Date os_increment_date_by_day(Date date, S64 days);
function U64 os_get_microseconds(void);
function void os_sleep(U64 time);
function void os_wait_microseconds(U64 end_time_us);

//////////////////////////////
// NOTE(hampus): Library

function OS_Handle os_library_open(String8 path);
function void os_library_close(OS_Handle library);
function void *os_libary_load_function(OS_Handle library, String8 name);

//////////////////////////////
// NOTE(hampus): Threading

function OS_Handle os_semaphore_alloc(U32 initial_value);
function void os_semaphore_free(OS_Handle handle);
function void os_semaphore_signal(OS_Handle handle);
function void os_semaphore_wait(OS_Handle handle);

function OS_Handle os_mutex_alloc(void);
function void os_mutex_free(OS_Handle handle);
function void os_mutex_take(OS_Handle handle);
function void os_mutex_release(OS_Handle handle);

function OS_Handle os_thread_create(ThreadProc *proc, void *data);
function void os_thread_join(OS_Handle handle);
function void os_thread_set_name(String8 string);
function U32 os_get_current_thread_id(void);

//////////////////////////////
// NOTE(hampus): Debug output

function void os_print_debug_string(String8 string);
function void os_print_debug_string(char *fmt, ...);

//////////////////////////////
// NOTE(hampus): Entry point

#define CRASH_HANDLER(name) void name(OS_Backtrace *backtrace)
typedef CRASH_HANDLER(OS_CrashHandlerProc);

function void os_set_crash_handler(OS_CrashHandlerProc *proc);
function void os_exit(S32 exit_code);
function S32 os_entry_point(String8List args);

//////////////////////////////
// NOTE(hampus): Macro helpers

#define os_mutex(mutex) defer_loop(os_mutex_take(mutex), os_mutex_release(mutex))

#endif // OS_CORE_H
