#include "third_party/libunwind/libunwind.h"

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_NONE 0x0
#define MAP_ANONYMOUS 0x20
#define MADV_DONTNEED 4

//////////////////////////////
// NOTE(hampus): Android helpers

function void *
linux_thread_proc(void *data)
{
  OS_Android_ThreadArgs args = *(OS_Android_ThreadArgs *)data;
  os_memory_release(data, sizeof(OS_Android_ThreadArgs));

  args.proc(args.data);

  return (0);
}

function B32
os_android_sync_file_descriptor(S32 file_descriptor)
{
  B32 success = true;

  S32 return_code = 0;
  do
  {
    errno = 0;
    return_code = fsync(file_descriptor);
  } while(return_code == -1 && errno == EINTR);

  if(return_code == -1)
  {
    success = false;
  }

  return success;
}

function DateTime
os_android_date_time_from_tm_and_milliseconds(struct tm *time, U16 milliseconds)
{
  DateTime result = {};
  result.millisecond = milliseconds;
  result.second = (U8)time->tm_sec;
  result.minute = (U8)time->tm_min;
  result.hour = (U8)time->tm_hour;
  result.day = (U8)(time->tm_mday - 1);
  result.month = (U8)time->tm_mon;
  result.year = (S16)(time->tm_year + 1900);
  return result;
}

function struct tm
os_android_tm_from_date_time(DateTime *date_time)
{
  struct tm result = {};
  result.tm_sec = date_time->second;
  result.tm_min = date_time->minute;
  result.tm_hour = date_time->hour;
  result.tm_mday = date_time->day + 1;
  result.tm_mon = date_time->month;
  result.tm_year = date_time->year - 1900;
  return result;
}

//////////////////////////////
// NOTE(hampus): Memory allocation

function OS_MemoryStats
os_get_memory_stats(void)
{
  OS_MemoryStats result = {};
  FILE *file = fopen("/proc/self/smaps", "r");
  if(file != 0)
  {
    char buffer[4096] = {};
    U64 total_reserve_size = 0;
    U64 total_commit_size = 0;

    // TODO(hampus): This really slow. Optimize
    while(fgets(buffer, sizeof(buffer), file))
    {
      String8 line = str8_cstr(buffer);
      {
        String8 label = str8_lit("Size:");
        if(str8_match(str8_prefix(line, label.size), label))
        {
          U64 size_kb = 0;
          sscanf(buffer, "Size: %" PRIU64 " kB", &size_kb);
          total_reserve_size += size_kb;
        }
      }
      {
        String8 label = str8_lit("Private_Dirty:");
        if(str8_match(str8_prefix(line, label.size), label))
        {
          U64 size_kb = 0;
          sscanf(buffer, "Private_Dirty: %" PRIU64 " kB", &size_kb);
          total_commit_size += size_kb;
        }
      }
    }

    fclose(file);
    result.reserved = memory_size_from_bytes(kilobytes(total_reserve_size));
    result.commited = memory_size_from_bytes(kilobytes(total_commit_size));
  }
  return result;
}

function void *
os_memory_reserve(U64 size)
{
  void *result = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  ASSERT(result != MAP_FAILED);
  ASSERT(result != MAP_FAILED);
  return result;
}

function void
os_memory_commit(void *ptr, U64 size)
{
  mprotect(ptr, size, PROT_READ | PROT_WRITE);
}

function void
os_memory_decommit(void *ptr, U64 size)
{
  mprotect(ptr, size, PROT_NONE);
  madvise(ptr, size, MADV_DONTNEED);
}

function void
os_memory_release(void *ptr, U64 size)
{
  S32 munmap_result = munmap(ptr, size);
  ASSERT(munmap_result == 0);
}

function void *
os_memory_alloc(U64 size)
{
  not_implemented;
  void *result = 0;
  return result;
}

//////////////////////////////
// NOTE(hampus): File attributes

function OS_FileAttributes
os_get_file_attributes(String8 path)
{
  not_implemented;
  OS_FileAttributes result = {};
  return result;
}

//////////////////////////////
// NOTE(hampus): Basic file operations

function String8
os_read_file(Arena *arena, String8 path)
{
  String8 result = {};
  B32 success = true;

  TempArena scratch = get_scratch(&arena, 1);
  U64 restore_pos = arena->pos;

  char *cstr_path = cstr_from_str8(scratch.arena, path);

  S32 file_descriptor = open(cstr_path, O_RDONLY);
  if(file_descriptor != -1)
  {
    struct stat status = {};
    if(fstat(file_descriptor, &status) != -1)
    {
      result.size = (U64)status.st_size;
      result.data = push_array_no_zero<U8>(arena, result.size);

      U8 *ptr = result.data;
      U8 *opl = result.data + result.size;

      // NOTE(simon): `read` doesn't neccessarily read all data in one
      // call, so we need to loop until all data has been read.
      while(ptr < opl)
      {
        U64 total_to_read = min((U64)(opl - ptr), SSIZE_MAX);
        errno = 0;
        S64 actual_read = read(file_descriptor, ptr, total_to_read);
        if(!(actual_read == -1 && errno == EINTR) && !(actual_read != -1))
        {
          success = false;
          break;
        }

        ptr += actual_read;
      }
    }
    else
    {
      success = false;
    }

    close(file_descriptor);
  }
  else
  {
    success = false;
  }

  if(!success)
  {
    arena_pop_to(arena, restore_pos);
  }
  return result;
}

function B32
os_file_write(String8 path, String8 data)
{
  B32 result = false;
  TempArena scratch = get_scratch(0, 0);
  char *cstr_path = cstr_from_str8(scratch.arena, path);
  S32 mode_flags = O_TRUNC;
  S32 file_descriptor = open(cstr_path, O_WRONLY | mode_flags | O_CREAT, S_IRUSR | S_IWUSR);

  if(file_descriptor != -1)
  {
    U8 *ptr = data.data;
    U8 *opl = data.data + data.size;

    // NOTE(simon): `write` doesn't necessarily write all data in one
    // call, so we need to loop until all data has been written.
    while(ptr < opl)
    {
      U64 to_write = min((U64)(opl - ptr), SSIZE_MAX);
      errno = 0;
      S64 actual_write = write(file_descriptor, ptr, to_write);
      if(!(actual_write == -1 && errno == EINTR) && !(actual_write != -1))
      {
        result = false;
        break;
      }

      ptr += actual_write;
    }

    result &= os_android_sync_file_descriptor(file_descriptor);

    close(file_descriptor);
  }
  return result;
}

function B32
os_file_copy(String8 old_path, String8 new_path)
{
  not_implemented;
  B32 result = false;
  return result;
}

function B32
os_file_rename(String8 old_path, String8 new_path)
{
  not_implemented;
  B32 result = false;
  return result;
}

function B32
os_file_delete(String8 path)
{
  not_implemented;
  B32 result = false;
  return result;
}

//////////////////////////////
// NOTE(hampus): File stream

function OS_Handle
os_file_stream_open(String8 path)
{
  not_implemented;
  OS_Handle result = os_handle_zero();
  return result;
}

function B32
os_file_stream_close(OS_Handle file)
{
  not_implemented;
  B32 result = false;
  return result;
}

function B32
os_file_stream_write(OS_Handle file, String8 data)
{
  not_implemented;
  B32 result = false;
  return result;
}

//////////////////////////////
// NOTE(hampus): Directory operations

function B32
os_directory_create(String8 path)
{
  not_implemented;
  B32 result = false;
  return result;
}

function B32
os_directory_delete(String8 path)
{
  not_implemented;
  B32 result = false;
  return result;
}

function OS_Handle
os_file_iterator_init(String8 path)
{
  not_implemented;
  OS_Handle result = os_handle_zero();
  return result;
}

function void
os_file_iterator_end(OS_Handle iterator)
{
  not_implemented;
}

function B32
os_file_iterator_next(Arena *arena, OS_Handle iterator, String8 *result_name)
{
  not_implemented;
  B32 result = false;
  return result;
}

function String8
os_get_executable_path(Arena *arena)
{
  not_implemented;
  String8 result = {};
  return result;
}

//////////////////////////////
// NOTE(hampus): Time

function DateTime
os_get_universal_time(void)
{
  not_implemented;
  DateTime result = {};
  return result;
}

function DateTime
os_get_local_time(void)
{
  not_implemented;
  DateTime result = {};
  return result;
}

function Date
os_get_local_date(void)
{
  Date result = {};

  struct timeval time = {};
  S32 return_code = gettimeofday(&time, 0);
  ASSERT(return_code == 0);

  struct tm deconstructed_time = {};
  if(localtime_r(&time.tv_sec, &deconstructed_time) == &deconstructed_time)
  {
    DateTime date_time = os_android_date_time_from_tm_and_milliseconds(&deconstructed_time, (U16)(time.tv_usec / 1000));
    result.day = date_time.day;
    result.month = date_time.month;
    result.year = date_time.year;
  }

  return result;
}

function DateTime
os_local_time_from_universal(DateTime *date_time)
{
  not_implemented;
  DateTime result = {};
  return result;
}

function DateTime
os_universal_time_from_local(DateTime *date_time)
{
  not_implemented;
  DateTime result = {};
  return result;
}

function Date
os_increment_date_by_day(Date date, S64 days)
{
  DateTime time0 = {};
  time0.year = date.year;
  time0.month = date.month;
  time0.day = date.day;
  struct tm local_tm = os_android_tm_from_date_time(&time0);
  time_t local_time = mktime(&local_tm);
  local_time += days * 24 * 60 * 60;
  local_tm = *localtime(&local_time);
  DateTime time1 = os_android_date_time_from_tm_and_milliseconds(&local_tm, time0.millisecond);
  Date result = {};
  result.year = time1.year;
  result.month = time1.month;
  result.day = time1.day;
  return result;
}

function U64
os_get_microseconds(void)
{
  struct timespec time = {};
  S32 return_code = clock_gettime(CLOCK_MONOTONIC_RAW, &time);
  ASSERT(return_code == 0);
  U64 result = (U64)time.tv_sec * million(1) + (U64)time.tv_nsec / 1000;
  return result;
}

function void
os_sleep(U64 time)
{
  not_implemented;
}

function void
os_wait_microseconds(U64 end_time_us)
{
  not_implemented;
}

//////////////////////////////
// NOTE(hampus): Library

function OS_Handle
os_library_open(String8 path)
{
  not_implemented;
  OS_Handle result = os_handle_zero();
  return result;
}

function void
os_library_close(OS_Handle library)
{
  not_implemented;
}

function void *
os_libary_load_function(OS_Handle library, String8 name)
{
  not_implemented;
  void *result = 0;
  return result;
}

//////////////////////////////
// NOTE(hampus): Threading

function OS_Handle
os_semaphore_alloc(U32 initial_value)
{
  ASSERT(initial_value < SEM_VALUE_MAX);
  OS_Handle result = os_handle_zero();
  OS_Android_Semaphore *semaphore = push_array<OS_Android_Semaphore>(os_android_state->arena, 1);
  sem_init(&semaphore->semaphore, 0, initial_value);
  result.u64[0] = int_from_ptr(semaphore);
  return result;
}

function void
os_semaphore_free(OS_Handle handle)
{
  OS_Android_Semaphore *semaphore = (OS_Android_Semaphore *)ptr_from_int(handle.u64[0]);
  ASSERT(semaphore != 0);
  sem_destroy(&semaphore->semaphore);
}

function void
os_semaphore_signal(OS_Handle handle)
{
  OS_Android_Semaphore *semaphore = (OS_Android_Semaphore *)ptr_from_int(handle.u64[0]);
  ASSERT(semaphore != 0);
  sem_post(&semaphore->semaphore);
}

function void
os_semaphore_wait(volatile OS_Handle handle)
{
  OS_Android_Semaphore *semaphore = (OS_Android_Semaphore *)ptr_from_int(handle.u64[0]);
  ASSERT(semaphore != 0);
  int success = 0;
  do
  {
    errno = 0;
    success = sem_wait(&semaphore->semaphore);
  } while(success == -1 && errno == EINTR);
}

function OS_Handle
os_mutex_alloc(void)
{
  OS_Handle result = os_handle_zero();
  OS_Android_Mutex *mutex = push_array<OS_Android_Mutex>(os_android_state->arena, 1);

  pthread_mutexattr_t attributes = {};
  pthread_mutexattr_init(&attributes);
  pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_ERRORCHECK);
  // pthread_mutexattr_setrobust(&attributes, PTHREAD_MUTEX_STALLED);
  pthread_mutexattr_setpshared(&attributes, PTHREAD_PROCESS_PRIVATE);
  pthread_mutex_init(&mutex->mutex, 0);
  pthread_mutexattr_destroy(&attributes);

  result.u64[0] = int_from_ptr(mutex);
  return result;
}

function void
os_mutex_free(OS_Handle handle)
{
  not_implemented;
}

function void
os_mutex_take(OS_Handle handle)
{
  OS_Android_Mutex *mutex = (OS_Android_Mutex *)ptr_from_int(handle.u64[0]);
  ASSERT(mutex != 0);
  pthread_mutex_lock(&mutex->mutex);
}

function void
os_mutex_release(OS_Handle handle)
{
  OS_Android_Mutex *mutex = (OS_Android_Mutex *)ptr_from_int(handle.u64[0]);
  ASSERT(mutex != 0);
  pthread_mutex_unlock(&mutex->mutex);
}

function OS_Handle
os_thread_create(ThreadProc *proc, void *data)
{
  OS_Handle result = os_handle_zero();
  OS_Android_ThreadArgs *args = (OS_Android_ThreadArgs *)os_memory_reserve(sizeof(OS_Android_ThreadArgs));
  os_memory_commit(args, sizeof(OS_Android_ThreadArgs));
  args->proc = proc;
  args->data = data;

  pthread_t thread = {};
  pthread_create(&thread, 0, linux_thread_proc, args);
  return result;
}

function void
os_thread_join(OS_Handle handle)
{
  not_implemented;
}

function void
os_thread_set_name(String8 string)
{
  TempArena scratch = get_scratch(0, 0);
  char *cstr_name = cstr_from_str8(scratch.arena, string);
  pthread_t thread = pthread_self();
  pthread_setname_np(thread, cstr_name);
}

function U32
os_get_current_thread_id(void)
{
  U32 result = pthread_self();
  return result;
}

//////////////////////////////
// NOTE(hampus): Debug output

function void
os_print_debug_string(String8 string)
{
  __android_log_print(ANDROID_LOG_INFO, "NativeExample", "%.*s", str8_expand(string));
}

function void
os_print_debug_string(char *fmt, ...)
{
  TempArena scratch = get_scratch(0, 0);
  va_list args;
  va_start(args, fmt);
  String8 string = str8_push(scratch.arena, fmt, args);
  os_print_debug_string(string);
  va_end(args);
}

//////////////////////////////
// NOTE(hampus): Exit

function void
os_exit(S32 exit_code)
{
  ANativeActivity_finish(android_app->activity);
}

function void
os_set_crash_handler(OS_CrashHandlerProc *proc)
{
  os_android_state->crash_handler_proc = proc;
}

function void
os_android_capture_backtrace__libunwind_register_method(OS_Android_BacktraceState *state)
{
  OS_Backtrace backtrace = {};
  ASSERT(state);
  // Initialize unw_context and unw_cursor.
  unw_context_t unw_context = {};
  unw_getcontext(&unw_context);
  unw_cursor_t unw_cursor = {};
  unw_init_local(&unw_cursor, &unw_context);

  // Get more contexts.
  const ucontext_t *signal_ucontext = state->signal_ucontext;
  ASSERT(signal_ucontext);
  const struct sigcontext *signal_mcontext = (struct sigcontext *)&signal_ucontext->uc_mcontext;
  ASSERT(signal_mcontext);

#if ARCH_ARM64
  unw_set_reg(&unw_cursor, UNW_REG_IP, signal_mcontext->pc);
  state->addresses[state->address_count++] = signal_mcontext->pc;
#else

  unw_set_reg(&unw_cursor, UNW_REG_IP, signal_mcontext->rip);
  unw_set_reg(&unw_cursor, UNW_REG_SP, signal_mcontext->rsp);
  state->addresses[state->address_count++] = signal_mcontext->rip;
#endif

  // unw_step() does not return the first IP,
  // the address of the instruction which caused the crash.
  // Thus let's add this address manually.

  // Unwind frames one by one, going up the frame stack.
  while(unw_step(&unw_cursor) > 0)
  {
    unw_word_t ip = 0;
    unw_get_reg(&unw_cursor, UNW_REG_IP, &ip);
    if(state->address_count < MAX_ADDRESS_COUNT)
    {
      state->addresses[state->address_count++] = ip;
    }
    else
    {
      break;
    }

    Dl_info dl_info = {};
    int dladdr_result = dladdr(ptr_from_int(ip), &dl_info);
    U64 shared_object_base_address = int_from_ptr(dl_info.dli_fbase);
    U64 address_offset = ip - shared_object_base_address;

    backtrace.entries[backtrace.entries_count].address = address_offset;
    backtrace.entries[backtrace.entries_count].module_path = str8_cstr((char *)dl_info.dli_fname);
    backtrace.entries_count += 1;
  }

  if(os_android_state->crash_handler_proc != 0)
  {
    os_android_state->crash_handler_proc(&backtrace);
  }
}

void
os_android_sig_action_handler(int sig, siginfo_t *info, void *ucontext)
{
  const ucontext_t *signal_ucontext = (const ucontext_t *)ucontext;
  ASSERT(signal_ucontext);
  OS_Android_BacktraceState backtrace_state = {};
  backtrace_state.signal_ucontext = signal_ucontext;

  os_android_capture_backtrace__libunwind_register_method(&backtrace_state);
}

void
android_main(struct android_app *_android_app)
{
  Arena *android_perm_arena = arena_alloc();
  os_android_state = push_array<OS_Android_State>(android_perm_arena, 1);
  os_android_state->arena = android_perm_arena;
  android_app = _android_app;
  ThreadCtx *tctx = thread_ctx_init(str8_lit("Main"));
  // Set up signal handler.
  struct sigaction action = {};
  memset(&action, 0, sizeof(action));
  sigemptyset(&action.sa_mask);
  action.sa_sigaction = os_android_sig_action_handler;
  action.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
  sigaction(SIGSEGV, &action, 0);
  sigaction(SIGFPE, &action, 0);
  sigaction(SIGILL, &action, 0);
  sigaction(SIGINT, &action, 0);
  sigaction(SIGQUIT, &action, 0);
  sigaction(SIGABRT, &action, 0);
  String8List argument_list = {};
  os_entry_point(argument_list);
}
