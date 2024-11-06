#pragma comment(lib, "winmm.lib")

//////////////////////////////
// NOTE(hampus): Defines

#define OS_WIN32_MAX_PATH (MAX_PATH)

//////////////////////////////
// NOTE(hampus): Global state

static OS_Win32_State *os_win32_state;

//////////////////////////////
// NOTE(hampus): Win32 helpers

static OS_Win32_Entity *
os_win32_entity_alloc()
{
  OS_Win32_Entity *result = os_win32_state->first_free_entity;
  if(result == 0)
  {
    result = push_array<OS_Win32_Entity>(os_win32_state->perm_arena, 1);
  }
  else
  {
    sll_stack_pop(os_win32_state->first_free_entity);
  }
  MemoryZeroStruct(result);
  return result;
}

static void
os_win32_entity_free(OS_Win32_Entity *e)
{
  sll_stack_push(os_win32_state->first_free_entity, e);
}

static B32
os_win32_handle_is_valid(HANDLE handle)
{
  B32 result = !(handle == INVALID_HANDLE_VALUE || handle == 0);
  return result;
}

static String16
os_win32_str16_from_wchar(WCHAR *wide_char)
{
  String16 result = {};
  result.data = (U16 *)wide_char;
  while(*wide_char++)
  {
    result.size++;
  }
  return result;
}

static DateTime
os_win32_date_time_from_system_time(SYSTEMTIME *system_time)
{
  DateTime result = {};
  result.millisecond = system_time->wMilliseconds;
  result.second = (U8)system_time->wSecond;
  result.minute = (U8)system_time->wMinute;
  result.hour = (U8)system_time->wHour;
  result.day = (U8)system_time->wDay;
  result.month = (U8)system_time->wMonth;
  result.year = (S16)system_time->wYear;
  return result;
}

static SYSTEMTIME
os_win32_system_time_from_date_time(DateTime *date_time)
{
  SYSTEMTIME result = {};
  result.wMilliseconds = (WORD)date_time->millisecond;
  result.wSecond = (WORD)date_time->second;
  result.wMinute = (WORD)date_time->minute;
  result.wHour = (WORD)date_time->hour;
  result.wDay = (WORD)date_time->day;
  result.wMonth = (WORD)date_time->month;
  result.wYear = (WORD)date_time->year;
  return result;
}

DWORD
os_win32_thread_proc(LPVOID data)
{
  ProfileInitThread();
  OS_Win32_ThreadArgs *args = (OS_Win32_ThreadArgs *)data;
  args->proc(args->data);
  ProfileQuitThread();
  return 0;
}

//////////////////////////////
// NOTE(hampus): Memory allocation

static void *
os_memory_reserve(U64 size)
{
  void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
  return result;
}

static void
os_memory_commit(void *ptr, U64 size)
{
  VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
}

static void
os_memory_decommit(void *ptr, U64 size)
{
  VirtualFree(ptr, size, MEM_DECOMMIT);
}

static void
os_memory_release(void *ptr, U64 size)
{
  VirtualFree(ptr, 0, MEM_RELEASE);
}

static void *
os_memory_alloc(U64 size)
{
  void *result = os_memory_reserve(size);
  os_memory_commit(result, size);
  return result;
}

static void
os_memory_free(void *memory)
{
  os_memory_release(memory, 0);
}

//////////////////////////////
// NOTE(hampus): File attributes

static OS_FileAttributes
os_get_file_attributes(String8 path)
{
  OS_FileAttributes result = {};
  TempArena scratch = GetScratch(0, 0);
  wchar_t *path_cstr16 = cstr16_from_str8(scratch.arena, path);
  DWORD attribs = GetFileAttributesW(path_cstr16);
  result.directory = (attribs & FILE_ATTRIBUTE_DIRECTORY) != 0;
  result.hidden = (attribs & FILE_ATTRIBUTE_HIDDEN) != 0;
  result.writable = (attribs & FILE_ATTRIBUTE_READONLY) == 0;

  return result;
}

//////////////////////////////
// NOTE(hampus): Basic file operations

static String8
os_file_read(Arena *arena, String8 path)
{
  String8 result = {};
  Assert(path.size < MAX_PATH);
  if(path.size <= OS_WIN32_MAX_PATH)
  {
    TempArena scratch = GetScratch(&arena, 1);
    HANDLE file = CreateFileW(cstr16_from_str8(scratch.arena, path),
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              0,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              0);

    if(os_win32_handle_is_valid(file))
    {
      LARGE_INTEGER file_size = {};
      GetFileSizeEx(file, &file_size);
      U32 file_size_u32 = safe_u32_from_s64(file_size.QuadPart);
      U32 push_amount = file_size_u32;
      result.data = push_array_no_zero<U8>(arena, push_amount);
      result.size = file_size_u32;
      DWORD bytes_read;
      BOOL read_file_result = ReadFile(file, result.data, file_size_u32, &bytes_read, 0);
      if(!read_file_result || bytes_read != file_size_u32)
      {
        result.data = 0;
        result.size = 0;
        arena_pop_amount(arena, push_amount);
      }
      CloseHandle(file);
    }
  }

  return result;
}

static B32
os_file_write(String8 path, String8 data, OS_FileWriteMode mode)
{
  B32 result = false;
  if(path.size <= OS_WIN32_MAX_PATH)
  {
    DWORD m = 0;
    switch(mode)
    {
      case OS_FileWriteMode_CreateAlways:
      {
        m = CREATE_ALWAYS;
      }
      break;
      case OS_FileWriteMode_OpenAlways:
      {
        m = OPEN_ALWAYS;
      }
      break;
      default:
      {
        InvalidCodePath;
      }
      break;
    }
    TempArena scratch = GetScratch(0, 0);
    HANDLE file = CreateFileW(cstr16_from_str8(scratch.arena, path),
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              0,
                              m,
                              FILE_ATTRIBUTE_NORMAL,
                              0);
    if(os_win32_handle_is_valid(file))
    {
      BOOL write_file_result = WriteFile(file, data.data, (DWORD)safe_s32_from_u64(data.size), 0, 0);
      result = (B32)write_file_result;
      CloseHandle(file);
    }
  }
  return result;
}

static B32
os_file_copy(String8 old_path, String8 new_path)
{
  B32 result = false;
  if(old_path.size <= OS_WIN32_MAX_PATH && new_path.size < OS_WIN32_MAX_PATH)
  {
    TempArena scratch = GetScratch(0, 0);
    wchar_t *old_path16 = cstr16_from_str8(scratch.arena, old_path);
    wchar_t *new_path16 = cstr16_from_str8(scratch.arena, new_path);
    result = (B32)CopyFileW(old_path16, new_path16, TRUE);
  }
  return result;
}

static B32
os_file_rename(String8 old_path, String8 new_path)
{
  B32 result = false;
  if(old_path.size <= OS_WIN32_MAX_PATH && new_path.size < OS_WIN32_MAX_PATH)
  {
    TempArena scratch = GetScratch(0, 0);
    wchar_t *old_path16 = cstr16_from_str8(scratch.arena, old_path);
    wchar_t *new_path16 = cstr16_from_str8(scratch.arena, new_path);
    result = (B32)MoveFileW(old_path16, new_path16);
  }
  return result;
}

static B32
os_file_delete(String8 path)
{
  B32 result = false;
  if(path.size <= OS_WIN32_MAX_PATH)
  {
    TempArena scratch = GetScratch(0, 0);
    result = (B32)CreateDirectoryW(cstr16_from_str8(scratch.arena, path), 0);
  }
  return result;
}

//////////////////////////////
// NOTE(hampus): File stream

static OS_Handle
os_file_stream_open(String8 path)
{
  OS_Handle result = os_handle_zero();
  if(path.size <= OS_WIN32_MAX_PATH)
  {
    TempArena scratch = GetScratch(0, 0);
    wchar_t *path16 = cstr16_from_str8(scratch.arena, path);
    HANDLE file_handle = CreateFileW(path16,
                                     GENERIC_WRITE | GENERIC_READ,
                                     0, 0, CREATE_ALWAYS, 0, 0);
    if(os_win32_handle_is_valid(file_handle))
    {
      result.u64[0] = IntFromPtr(file_handle);
    }
  }
  return result;
}

static B32
os_file_stream_close(OS_Handle file)
{
  B32 success = false;
  HANDLE file_handle = PtrFromInt(file.u64[0]);
  if(os_win32_handle_is_valid(file_handle))
  {
    CloseHandle(file_handle);
    success = true;
  }
  return (success);
}

static B32
os_file_stream_write(OS_Handle file, String8 data)
{
  B32 result = false;
  HANDLE file_handle = PtrFromInt(file.u64[0]);
  if(os_win32_handle_is_valid(file_handle))
  {
    DWORD bytes_written = 0;
    Assert(data.size <= U32_MAX);
    BOOL write_file_result = WriteFile(file_handle, data.data, (DWORD)safe_s32_from_u64(data.size), &bytes_written, 0);
    if(bytes_written == data.size && write_file_result)
    {
      result = true;
    }
  }
  return result;
}

//////////////////////////////
// NOTE(hampus): Directory operations

static B32
os_directory_create(String8 path)
{
  B32 result = false;
  if(path.size <= OS_WIN32_MAX_PATH)
  {
    TempArena scratch = GetScratch(0, 0);
    result = (B32)CreateDirectoryW(cstr16_from_str8(scratch.arena, path), 0);
    if(!result)
    {
      DWORD error = GetLastError();
      if(error == ERROR_ALREADY_EXISTS)
      {
        result = true;
      }
    }
  }
  return result;
}

static B32
os_directory_delete(String8 path)
{
  B32 result = false;
  if(path.size <= OS_WIN32_MAX_PATH)
  {
    TempArena scratch = GetScratch(0, 0);
    result = (B32)RemoveDirectoryW(cstr16_from_str8(scratch.arena, path));
  }
  return result;
}

static OS_Handle
os_file_iterator_init(String8 path)
{
  OS_Handle result = {};
  if(path.size <= OS_WIN32_MAX_PATH)
  {
    String8Node nodes[2] = {};
    String8List list = {};
    str8_list_push(&list, path, nodes + 0);
    str8_list_push(&list, Str8Lit("\\*"), nodes + 1);
    TempArena scratch = GetScratch(0, 0);
    String8 path_star = str8_join(scratch.arena, &list);
    wchar_t *path16 = cstr16_from_str8(scratch.arena, path_star);
    OS_Win32_Entity *win32_iter = os_win32_entity_alloc();
    win32_iter->handle = FindFirstFileW(path16, &win32_iter->find_data);

    result.u64[0] = IntFromPtr(win32_iter);
  }
  return result;
}

static void
os_file_iterator_end(OS_Handle iterator)
{
  OS_Win32_Entity *win32_iter = (OS_Win32_Entity *)PtrFromInt(iterator.u64[0]);
  if(os_win32_handle_is_valid(win32_iter->handle))
  {
    FindClose(win32_iter->handle);
  }
}

static B32
os_file_iterator_next(Arena *arena, OS_Handle iterator, String8 *result_name)
{
  B32 result = false;
  OS_Win32_Entity *win32_iter = (OS_Win32_Entity *)PtrFromInt(iterator.u64[0]);
  if(os_win32_handle_is_valid(win32_iter->handle))
  {
    for(; !win32_iter->done;)
    {
      WCHAR *file_name = win32_iter->find_data.cFileName;
      B32 is_dot = (file_name[0] == '.' && file_name[1] == 0);
      B32 is_dotdot = (file_name[0] == '.' && file_name[1] == '.' && file_name[2] == 0);
      B32 emit = (!is_dot && !is_dotdot);
      WIN32_FIND_DATAW data = {};
      if(emit)
      {
        MemoryCopyStruct(&data, &win32_iter->find_data);
      }
      if(!FindNextFileW(win32_iter->handle, &win32_iter->find_data))
      {
        win32_iter->done = true;
      }
      if(emit)
      {
        *result_name = str8_from_str16(arena, os_win32_str16_from_wchar(data.cFileName));
        result = true;
        break;
      }
    }
  }
  return result;
}

static String8
os_get_executable_path(Arena *arena)
{
  String8 result = {};
  StaticArray<WCHAR, 1024> buffer = {};
  U64 length = GetModuleFileNameW(0, buffer.val, (DWORD)array_count(buffer));
  result = str8_from_str16(arena, str16((U16 *)buffer.val, length));
  return result;
}

//////////////////////////////
// NOTE(hampus): Time

static DateTime
os_get_universal_time()
{
  SYSTEMTIME universal_time = {};
  GetSystemTime(&universal_time);
  DateTime result = os_win32_date_time_from_system_time(&universal_time);
  return result;
}

static DateTime
os_get_local_time()
{
  SYSTEMTIME local_time = {};
  GetLocalTime(&local_time);
  DateTime result = os_win32_date_time_from_system_time(&local_time);
  return result;
}

static Date
os_get_local_date()
{
  SYSTEMTIME local_time = {};
  GetLocalTime(&local_time);
  DateTime time = os_win32_date_time_from_system_time(&local_time);
  Date result = {};
  result.day = time.day;
  result.month = time.month;
  result.year = time.year;
  return result;
}

static DateTime
os_local_time_from_universal(DateTime *date_time)
{
  TIME_ZONE_INFORMATION time_zone_information;
  GetTimeZoneInformation(&time_zone_information);
  SYSTEMTIME universal_time = os_win32_system_time_from_date_time(date_time);
  SYSTEMTIME local_time;
  SystemTimeToTzSpecificLocalTime(&time_zone_information, &universal_time, &local_time);
  DateTime result = os_win32_date_time_from_system_time(&local_time);
  return result;
}

static DateTime
os_universal_time_from_local(DateTime *date_time)
{
  TIME_ZONE_INFORMATION time_zone_information;
  GetTimeZoneInformation(&time_zone_information);
  SYSTEMTIME local_time = os_win32_system_time_from_date_time(date_time);
  SYSTEMTIME universal_time;
  TzSpecificLocalTimeToSystemTime(&time_zone_information, &local_time, &universal_time);
  DateTime result = os_win32_date_time_from_system_time(&universal_time);
  return result;
}

static U64
os_get_microseconds()
{
  U64 result = 0;
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  counter.QuadPart *= Million(1);
  counter.QuadPart /= os_win32_state->frequency.QuadPart;
  result = safe_u64_from_s64(counter.QuadPart);
  return result;
}

static void
os_sleep(U64 ms)
{
  Sleep((DWORD)ms);
}

static void
os_wait_microseconds(U64 end_time_us)
{
  U64 begin_time_us = os_get_microseconds();
  if(end_time_us > begin_time_us)
  {
    U64 time_to_wait_us = end_time_us - begin_time_us;
    os_sleep(time_to_wait_us / 1000);

    for(; os_get_microseconds() < end_time_us;)
      ;
  }
}

//////////////////////////////
// NOTE(hampus): Library

static OS_Handle
os_library_open(String8 path)
{
  OS_Handle result = {};
  TempArena scratch = GetScratch(0, 0);
  String8List part_list = {};
  String8Node parts[2];
  str8_list_push(&part_list, path, &parts[0]);
  str8_list_push(&part_list, Str8Lit(".dll"), &parts[1]);
  String8 full_path = str8_join(scratch.arena, &part_list);
  HMODULE lib = LoadLibraryW(cstr16_from_str8(scratch.arena, full_path));
  if(lib)
  {
    result.u64[0] = IntFromPtr(lib);
  }

  return result;
}

static void
os_library_close(OS_Handle library)
{
  if(library.u64[0])
  {
    HMODULE hmodule = (HMODULE)PtrFromInt(library.u64[0]);
    FreeLibrary(hmodule);
  }
}

static void *
os_libary_load_static(OS_Handle library, String8 name)
{
  void *result = 0;
  if(library.u64[0])
  {
    HMODULE lib = (HMODULE)PtrFromInt(library.u64[0]);
    TempArena scratch = GetScratch(0, 0);
    result = (void *)GetProcAddress(lib, cstr_from_str8(scratch.arena, name));
  }
  return result;
}

//////////////////////////////
// NOTE(hampus): Threading

static OS_Handle
os_semaphore_alloc(U32 initial_value)
{
  OS_Handle result = {};
  HANDLE win32_handle = CreateSemaphore(0, safe_s32_from_u32(initial_value), S32_MAX, 0);
  if(os_win32_handle_is_valid(win32_handle))
  {
    result.u64[0] = IntFromPtr(win32_handle);
  }
  return result;
}

static void
os_semaphore_free(OS_Handle handle)
{
  HANDLE win32_handle = (HMODULE)PtrFromInt(handle.u64[0]);
  if(os_win32_handle_is_valid(win32_handle))
  {
    CloseHandle(win32_handle);
  }
}

static void
os_semaphore_signal(OS_Handle handle)
{
  HANDLE win32_handle = (HMODULE)PtrFromInt(handle.u64[0]);
  if(os_win32_handle_is_valid(win32_handle))
  {
    ReleaseSemaphore(win32_handle, 1, 0);
  }
}

static void
os_semaphore_wait(OS_Handle handle)
{
  HANDLE win32_handle = (HMODULE)PtrFromInt(handle.u64[0]);
  if(os_win32_handle_is_valid(win32_handle))
  {
    WaitForSingleObject(win32_handle, INFINITE);
  }
}

static OS_Handle
os_mutex_alloc()
{
  OS_Handle result = {};
  HANDLE win32_handle = CreateMutex(0, FALSE, 0);
  if(os_win32_handle_is_valid(win32_handle))
  {
    result.u64[0] = IntFromPtr(win32_handle);
  }
  return result;
}

static void
os_mutex_free(OS_Handle handle)
{
  HANDLE win32_handle = (HMODULE)PtrFromInt(handle.u64[0]);
  if(os_win32_handle_is_valid(win32_handle))
  {
    CloseHandle(win32_handle);
  }
}

static void
os_mutex_take(OS_Handle handle)
{
  HANDLE win32_handle = (HMODULE)PtrFromInt(handle.u64[0]);
  if(os_win32_handle_is_valid(win32_handle))
  {
    WaitForSingleObject(win32_handle, INFINITE);
  }
}

static void
os_mutex_release(OS_Handle handle)
{
  HANDLE win32_handle = (HMODULE)PtrFromInt(handle.u64[0]);
  if(os_win32_handle_is_valid(win32_handle))
  {
    ReleaseMutex(win32_handle);
  }
}

static OS_Handle
os_thread_create(ThreadProc *proc, void *data)
{
  OS_Handle result = {};
  OS_Win32_ThreadArgs *args = push_array<OS_Win32_ThreadArgs>(os_win32_state->perm_arena, 1);
  args->proc = proc;
  args->data = data;
  HANDLE handle = CreateThread(0, 0, os_win32_thread_proc, args, 0, 0);
  result.u64[0] = IntFromPtr(handle);
  return result;
}

static void
os_thread_join(OS_Handle handle)
{
  HANDLE win32_handle = PtrFromInt(handle.u64[0]);
  if(os_win32_handle_is_valid(win32_handle))
  {
    WaitForSingleObject(win32_handle, INFINITE);
  }
}

static void
os_thread_set_name(String8 string)
{
  HANDLE handle = GetCurrentThread();
  TempArena scratch = GetScratch(0, 0);
  SetThreadDescription(handle, (PCWSTR)cstr16_from_str8(scratch.arena, string));
}

static U32
os_get_current_thread_id()
{
  U32 result = GetCurrentThreadId();
  return result;
}

//////////////////////////////
// NOTE(hampus): Debug output

static void
os_print_debug_string(String8 string)
{
  TempArena scratch = GetScratch(0, 0);
  wchar_t *cstr16 = cstr16_from_str8(scratch.arena, string);
  OutputDebugStringW(cstr16);
}

static void
os_print_debug_string(const char *fmt, ...)
{
  TempArena scratch = GetScratch(0, 0);
  va_list args;
  va_start(args, fmt);
  String8 result = str8_push(scratch.arena, (char *)fmt, args);
  os_print_debug_string(result);
  va_end(args);
}

//////////////////////////////
// NOTE(hampus): Entry point

#if !defined(OS_NO_ENTRY_POINT)

#  if 1
S32 APIENTRY
WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR command_line, S32 show_code)
#  else
S32
main(S32 argc, char **argv)
#  endif
{
  timeBeginPeriod(1);
  Arena *win32_perm_arena = arena_alloc();
  os_win32_state = push_array<OS_Win32_State>(win32_perm_arena, 1);
  os_win32_state->tls_idx = TlsAlloc();
  os_win32_state->perm_arena = win32_perm_arena;
  QueryPerformanceFrequency(&os_win32_state->frequency);
  ThreadCtx *tctx = thread_ctx_init(Str8Lit("Main"));
  // NOTE(hampus): 'command_line' handed to WinMain doesn't include the program name
  LPSTR command_line_with_exe_path = GetCommandLineA();
  String8List argument_list = str8_split_by_codepoints(os_win32_state->perm_arena, str8_cstr(command_line_with_exe_path), Str8Lit(" "));
  S32 exit_code = os_entry_point(argument_list);
  ExitProcess(safe_u32_from_s32(exit_code));
}

#endif

//////////////////////////////
// NOTE(hampus): Exit

static void
os_exit(S32 exit_code)
{
  ExitProcess(safe_u32_from_s32(exit_code));
}