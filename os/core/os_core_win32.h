#ifndef OS_CORE_WIN32_H
#define OS_CORE_WIN32_H

#undef function
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>
#define function static

//////////////////////////////
// NOTE(hampus): Win32 specific types

struct OS_Win32_ThreadArgs
{
  ThreadProc *proc;
  void *data;
};

struct OS_Win32_Entity
{
  OS_Win32_Entity *next;
  HANDLE handle;
  WIN32_FIND_DATAW find_data;
  B32 done;
};

struct OS_Win32_State
{
  Arena *perm_arena;
  OS_Win32_Entity *first_free_entity;
  DWORD tls_idx;
  LARGE_INTEGER frequency;
};

DWORD os_win32_thread_proc(LPVOID data);

#endif // OS_CORE_WIN32_H
