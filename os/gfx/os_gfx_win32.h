#ifndef OS_GFX_WIN32_H
#define OS_GFX_WIN32_H

#include <Windows.h>
#include <Windowsx.h>

struct OS_Win32_GfxState
{
  Arena *arena;
  HCURSOR cursors[OS_Cursor_COUNT];
  OS_Cursor active_cursor;
  B32 resizing;
};

struct OS_Win32_Window
{
  HWND hwnd;
  OS_WindowRepaintProc *repaint;
  void *repaint_user_data;
};

#endif // OS_GFX_WIN32_H
