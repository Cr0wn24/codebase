#pragma comment(lib, "gdi32")
#pragma comment(lib, "user32")

//////////////////////////////
// NOTE(hampus): Globals

static OS_Win32_GfxState *os_win32_gfx_state;
static Arena *os_win32_event_arena;
static OS_EventList *os_win32_event_list;

static OS_Win32_Window *
os_win32_window_from_handle(OS_Handle handle)
{
  OS_Win32_Window *result = (OS_Win32_Window *)PtrFromInt(handle.u64[0]);
  return result;
}

static LRESULT CALLBACK
os_win32_window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
  LRESULT result = 0;
  TempArena scratch = GetScratch(0, 0);
  OS_Win32_Window *win32_window = (OS_Win32_Window *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
  Arena *event_arena = os_win32_event_arena;
  OS_EventList fallback_list = {};
  OS_EventList *event_list = os_win32_event_list;

  if(os_win32_event_list == 0 ||
     os_win32_event_arena == 0)
  {
    event_arena = scratch.arena;
    event_list = &fallback_list;
  }

  Assert(event_list);
  Assert(event_arena);

  OS_EventNode *event_node = 0;
  switch(message)
  {
    case WM_CLOSE:
    case WM_QUIT:
    case WM_DESTROY:
    {
      event_node = push_array<OS_EventNode>(event_arena, 1);
      event_node->v.kind = OS_EventKind_Quit;
    }
    break;
    case WM_PAINT:
    case WM_SIZE:
    {
      PAINTSTRUCT ps = {};
      BeginPaint(hwnd, &ps);
      if(win32_window->repaint)
      {
        OS_Handle handle = {};
        handle.u64[0] = IntFromPtr(win32_window);
        win32_window->repaint(handle, win32_window->repaint_user_data);
      }
      EndPaint(hwnd, &ps);
      result = DefWindowProcW(hwnd, message, wparam, lparam);
    }
    break;
    case WM_MOUSEMOVE:
    {
      event_node = push_array<OS_EventNode>(event_arena, 1);
      event_node->v.kind = OS_EventKind_MouseMove;
      event_node->v.mouse = v2f32((F32)GET_X_LPARAM(lparam), (F32)GET_Y_LPARAM(lparam));
    }
    break;
    case WM_CHAR:
    {
      static U16 high_surrogate = 0;
      static U16 low_surrogate = 0;
      if(IS_HIGH_SURROGATE(wparam))
      {
        high_surrogate = (U16)wparam;
      }
      else if(IS_LOW_SURROGATE(wparam))
      {
        low_surrogate = (U16)wparam;
      }
      else
      {
        if(wparam >= 32)
        {
          event_node = push_array<OS_EventNode>(event_arena, 1);
          event_node->v.kind = OS_EventKind_Char;
          event_node->v.cp = (U32)wparam;

          high_surrogate = 0;
          high_surrogate = 0;
        }
      }

      if(IS_SURROGATE_PAIR(high_surrogate, low_surrogate))
      {
        U32 cp = (0x10000 +
                  (((U32)high_surrogate & 0x3FF) << 10) +
                  ((U32)low_surrogate & 0x3FF));
        event_node = push_array<OS_EventNode>(event_arena, 1);
        event_node->v.kind = OS_EventKind_Char;
        event_node->v.cp = cp;
        high_surrogate = 0;
        low_surrogate = 0;
      }
    }
    break;
    case WM_MOUSEWHEEL:
    {
      S32 delta_y = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
      event_node = push_array<OS_EventNode>(event_arena, 1);
      event_node->v.kind = OS_EventKind_Scroll;
      event_node->v.scroll = v2f32(0, (F32)delta_y);
    }
    break;
    case WM_ENTERSIZEMOVE:
    {
      os_win32_gfx_state->resizing = true;
    }
    break;
    case WM_EXITSIZEMOVE:
    {
      os_win32_gfx_state->resizing = false;
    }
    break;
    case WM_LBUTTONDBLCLK:
    {
      event_node = push_array<OS_EventNode>(event_arena, 1);
      event_node->v.kind = OS_EventKind_KeyDoublePress;
      event_node->v.key = OS_Key_MouseLeft;
    }
    break;
    case WM_MBUTTONDBLCLK:
    {
      event_node = push_array<OS_EventNode>(event_arena, 1);
      event_node->v.kind = OS_EventKind_KeyDoublePress;
      event_node->v.key = OS_Key_MouseMiddle;
    }
    break;
    case WM_RBUTTONDBLCLK:
    {
      event_node = push_array<OS_EventNode>(event_arena, 1);
      event_node->v.kind = OS_EventKind_KeyDoublePress;
      event_node->v.key = OS_Key_MouseRight;
    }
    break;
    case WM_RBUTTONUP:
    case WM_RBUTTONDOWN:
    {
      event_node = push_array<OS_EventNode>(event_arena, 1);
      event_node->v.kind = message == WM_RBUTTONDOWN ? OS_EventKind_KeyPress : OS_EventKind_KeyRelease;
      event_node->v.key = OS_Key_MouseRight;
    }
    break;
    case WM_MBUTTONUP:
    case WM_MBUTTONDOWN:
    {
      event_node = push_array<OS_EventNode>(event_arena, 1);
      event_node->v.kind = message == WM_MBUTTONDOWN ? OS_EventKind_KeyPress : OS_EventKind_KeyRelease;
      event_node->v.key = OS_Key_MouseMiddle;
    }
    break;
    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
    {
      event_node = push_array<OS_EventNode>(event_arena, 1);
      event_node->v.kind = message == WM_LBUTTONDOWN ? OS_EventKind_KeyPress : OS_EventKind_KeyRelease;
      event_node->v.key = OS_Key_MouseLeft;
    }
    break;
    case WM_SETCURSOR:
    {
      Vec2F32 mouse_pos = {};
      POINT point = {};
      GetCursorPos(&point);
      ScreenToClient(win32_window->hwnd, &point);
      mouse_pos.x = (F32)point.x;
      mouse_pos.y = (F32)point.y;
      Vec2U64 client_area = {};
      RECT rect = {};
      GetClientRect(hwnd, &rect);
      client_area.x = (U64)(rect.right - rect.left);
      client_area.y = (U64)(rect.bottom - rect.top);
      B32 mouse_inside_window = mouse_pos.x >= 0 && mouse_pos.y >= 0 && mouse_pos.x < (F32)client_area.x && mouse_pos.y < (F32)client_area.y;
      if(!os_win32_gfx_state->resizing && mouse_inside_window)
      {
        SetCursor(os_win32_gfx_state->cursors[os_win32_gfx_state->active_cursor]);
      }
      else
      {
        result = DefWindowProcW(hwnd, message, wparam, lparam);
      }
    }
    break;
    case WM_SYSKEYUP:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
      U32 vk_code = (U32)wparam;
      B32 was_down = (lparam & (1 << 30)) != 0;
      B32 is_down = (lparam & (1 << 31)) == 0;
      B32 alt_key_was_down = (lparam & (1 << 20)) != 0;

      static B32 key_table_initialized = false;
      static OS_Key key_table[255];

      if(!key_table_initialized)
      {
        for(U64 i = 0; i < 10; ++i)
        {
          key_table[0x30 + i] = (OS_Key)(OS_Key_0 + i);
        }
        for(U64 i = 0; i < 26; ++i)
        {
          key_table[0x41 + i] = (OS_Key)(OS_Key_A + i);
        }
        for(U64 i = 0; i < 12; ++i)
        {
          key_table[VK_F1 + i] = (OS_Key)(OS_Key_F1 + i);
        }

        key_table[VK_BACK] = OS_Key_Backspace;
        key_table[VK_SPACE] = OS_Key_Space;
        key_table[VK_MENU] = OS_Key_Alt;
        key_table[VK_LWIN] = OS_Key_OS;
        key_table[VK_RWIN] = OS_Key_OS;
        key_table[VK_TAB] = OS_Key_Tab;
        key_table[VK_RETURN] = OS_Key_Return;
        key_table[VK_SHIFT] = OS_Key_Shift;
        key_table[VK_CONTROL] = OS_Key_Control;
        key_table[VK_ESCAPE] = OS_Key_Escape;
        key_table[VK_PRIOR] = OS_Key_PageUp;
        key_table[VK_NEXT] = OS_Key_PageDown;
        key_table[VK_END] = OS_Key_End;
        key_table[VK_HOME] = OS_Key_Home;
        key_table[VK_LEFT] = OS_Key_Left;
        key_table[VK_RIGHT] = OS_Key_Right;
        key_table[VK_UP] = OS_Key_Up;
        key_table[VK_DOWN] = OS_Key_Down;
        key_table[VK_DELETE] = OS_Key_Delete;
        key_table[VK_OEM_PLUS] = OS_Key_Plus;
        key_table[VK_OEM_MINUS] = OS_Key_Minus;

        key_table_initialized = true;
      }

      OS_Key key = key_table[vk_code];
      if(key != 0)
      {
        event_node = push_array<OS_EventNode>(event_arena, 1);
        event_node->v.kind = (message == WM_SYSKEYDOWN || message == WM_KEYDOWN) ? OS_EventKind_KeyPress : OS_EventKind_KeyRelease;
        event_node->v.key = key;
      }
    }
    break;
    default:
    {
      result = DefWindowProcW(hwnd, message, wparam, lparam);
    }
    break;
  }

  if(event_node)
  {
    event_node->v.key_modifiers |= ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0) * OS_KeyModifier_Shift;
    event_node->v.key_modifiers |= ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0) * OS_KeyModifier_Ctrl;
    event_node->v.key_modifiers |= ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0) * OS_KeyModifier_Alt;
    dll_push_back(event_list->first, event_list->last, event_node);
    event_list->count += 1;
  }

  return result;
}

static void
os_gfx_init()
{
  // hampus: Initialize state

  Arena *arena = arena_alloc();
  os_win32_gfx_state = push_array<OS_Win32_GfxState>(arena, 1);
  os_win32_gfx_state->arena = arena;

  // hampus: Load cursors

  os_win32_gfx_state->cursors[OS_Cursor_Arrow] = LoadCursorA(0, IDC_ARROW);
  os_win32_gfx_state->cursors[OS_Cursor_Hand] = LoadCursorA(0, IDC_HAND);
  os_win32_gfx_state->cursors[OS_Cursor_WestEast] = LoadCursorA(0, IDC_SIZEWE);
  os_win32_gfx_state->cursors[OS_Cursor_NorthSouth] = LoadCursorA(0, IDC_SIZENS);
  os_win32_gfx_state->cursors[OS_Cursor_NorthWestSouthEast] = LoadCursorA(0, IDC_SIZENWSE);
  os_win32_gfx_state->cursors[OS_Cursor_NorthEasttSouthWest] = LoadCursorA(0, IDC_SIZENESW);
  os_win32_gfx_state->cursors[OS_Cursor_Beam] = LoadCursorA(0, IDC_IBEAM);

  // hampus: Set DPI awareness

  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
}

//////////////////////////////
// NOTE(hampus): Window

static OS_Handle
os_window_open(String8 title, U32 width, U32 height)
{
  OS_Handle result = {};
  TempArena scratch = GetScratch(0, 0);

  OS_Win32_Window *win32_window = push_array<OS_Win32_Window>(os_win32_gfx_state->arena, 1);

  HINSTANCE instance = GetModuleHandle(0);

  WNDCLASSW window_class = {};
  window_class.lpfnWndProc = os_win32_window_proc;
  window_class.hInstance = instance;
  window_class.lpszClassName = L"ApplicationWindowClassName";
  window_class.style = CS_VREDRAW | CS_HREDRAW;

  RegisterClassW(&window_class);

  wchar_t *title_u16 = cstr16_from_str8(scratch.arena, title);
  HWND hwnd = CreateWindowExW(0,
                              window_class.lpszClassName, title_u16,
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              (S32)width, (S32)height,
                              0, 0, instance, 0);

  win32_window->hwnd = hwnd;
  result.u64[0] = IntFromPtr(win32_window);
  SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)win32_window);
  os_set_cursor(OS_Cursor_Arrow);

  return result;
}

static void
os_window_show(OS_Handle handle)
{
  OS_Win32_Window *win32_window = (OS_Win32_Window *)PtrFromInt(handle.u64[0]);
  ShowWindow(win32_window->hwnd, SW_SHOW);
}

static Vec2U64
os_client_area_from_window(OS_Handle handle)
{
  Vec2U64 result = {};
  OS_Win32_Window *win32_window = (OS_Win32_Window *)PtrFromInt(handle.u64[0]);
  RECT rect = {};
  GetClientRect(win32_window->hwnd, &rect);
  result.x = (U64)(rect.right - rect.left);
  result.y = (U64)(rect.bottom - rect.top);
  return result;
}

static RectF32
os_client_rect_from_window(OS_Handle handle)
{
  RectF32 result = {};
  OS_Win32_Window *win32_window = os_win32_window_from_handle(handle);
  RECT rect = {};
  GetClientRect(win32_window->hwnd, &rect);
  result.min.x = (F32)rect.left;
  result.min.y = (F32)rect.top;
  result.max.x = (F32)rect.right;
  result.max.y = (F32)rect.bottom;
  return result;
}

static RectF32
os_window_rect_from_window(OS_Handle handle)
{
  RectF32 result = {};
  OS_Win32_Window *win32_window = os_win32_window_from_handle(handle);
  RECT rect = {};
  GetWindowRect(win32_window->hwnd, &rect);
  result.min.x = (F32)rect.left;
  result.min.y = (F32)rect.top;
  result.max.x = (F32)rect.right;
  result.max.y = (F32)rect.bottom;
  return result;
}

static void
os_window_equip_repaint(OS_Handle window, OS_WindowRepaintProc *proc, void *user_data)
{
  OS_Win32_Window *win32_window = (OS_Win32_Window *)PtrFromInt(window.u64[0]);
  win32_window->repaint = proc;
  win32_window->repaint_user_data = user_data;
}

static void
os_window_maximize(OS_Handle window)
{
  OS_Win32_Window *win32_window = (OS_Win32_Window *)PtrFromInt(window.u64[0]);
  ShowWindow(win32_window->hwnd, SW_MAXIMIZE);
}

static void
os_window_toggle_fullscreen(OS_Handle window)
{
  OS_Win32_Window *win32_window = (OS_Win32_Window *)PtrFromInt(window.u64[0]);
  static WINDOWPLACEMENT prev_placement = {sizeof(prev_placement)};
  LONG window_style = GetWindowLong(win32_window->hwnd, GWL_STYLE);
  if(window_style & WS_OVERLAPPEDWINDOW)
  {
    MONITORINFO monitor_info = {sizeof(monitor_info)};
    if(GetWindowPlacement(win32_window->hwnd, &prev_placement) &&
       GetMonitorInfo(MonitorFromWindow(win32_window->hwnd, MONITOR_DEFAULTTOPRIMARY), &monitor_info))
    {
      SetWindowLong(win32_window->hwnd, GWL_STYLE, (LONG)(window_style & ~WS_OVERLAPPEDWINDOW));

      SetWindowPos(win32_window->hwnd, HWND_TOP,
                   monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
                   monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                   monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                   SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
  }
  else
  {
    SetWindowLong(win32_window->hwnd, GWL_STYLE, (LONG)(window_style | WS_OVERLAPPEDWINDOW));
    SetWindowPlacement(win32_window->hwnd, &prev_placement);
    SetWindowPos(win32_window->hwnd, 0, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
  }
}

static OS_EventList
os_events_from_window(Arena *arena, OS_Handle window)
{
  OS_EventList result = {};
  os_win32_event_arena = arena;
  os_win32_event_list = &result;
  for(MSG message; PeekMessageW(&message, 0, 0, 0, PM_REMOVE);)
  {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }
  os_win32_event_arena = 0;
  os_win32_event_list = 0;
  return result;
}

static Vec2F32
os_window_dpi(OS_Handle handle)
{
  OS_Win32_Window *win32_window = (OS_Win32_Window *)PtrFromInt(handle.u64[0]);
  UINT dpi = GetDpiForWindow(win32_window->hwnd);
  Vec2F32 result = {};
  result.x = (F32)dpi;
  result.y = (F32)dpi;
  return result;
}

static Vec2F32
os_mouse_pos(OS_Handle window)
{
  OS_Win32_Window *win32_window = (OS_Win32_Window *)PtrFromInt(window.u64[0]);
  Vec2F32 result = {};
  POINT point = {};
  GetCursorPos(&point);
  ScreenToClient(win32_window->hwnd, &point);
  result.x = (F32)point.x;
  result.y = (F32)point.y;
  return result;
}

static void
os_swap_buffers(OS_Handle handle)
{
}

//////////////////////////////
// NOTE(hampus): Cursor

static void
os_set_cursor(OS_Cursor cursor)
{
  os_win32_gfx_state->active_cursor = cursor;
}

static void
os_set_cursor_visibility(B32 show)
{
  CURSORINFO ci = {sizeof(CURSORINFO)};
  B32 showing = false;
  if(GetCursorInfo(&ci))
  {
    if(ci.flags & CURSOR_SHOWING)
    {
      showing = true;
    }
  }
  if(showing != show)
  {
    ShowCursor((BOOL)show);
  }
}

static void
os_set_cursor_pos(Vec2S32 pos)
{
  SetCursorPos(pos.x, pos.y);
}

//////////////////////////////
// NOTE(hampus): Clipboard

static String8
os_push_clipboard(Arena *arena)
{
  String8 result = {};
  OpenClipboard(0);
  HANDLE handle = GetClipboardData(CF_TEXT);
  char *data = (char *)GlobalLock(handle);
  result = str8_cstr(data);
  GlobalUnlock(handle);
  CloseClipboard();
  return result;
}

static void
os_set_clipboard(String8 string)
{
  // TODO(hampus): Memory leak?
  HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, string.size + 1);
  MemoryCopy(GlobalLock(memory), string.data, string.size);
  GlobalUnlock(memory);
  OpenClipboard(0);
  EmptyClipboard();
  SetClipboardData(CF_TEXT, memory);
  CloseClipboard();
}