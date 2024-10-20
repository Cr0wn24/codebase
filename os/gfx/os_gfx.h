#ifndef OS_GFX_H
#define OS_GFX_H

//////////////////////////////
// NOTE(hampus): Cursors

enum OS_Cursor
{
  OS_Cursor_Arrow,
  OS_Cursor_Hand,
  OS_Cursor_WestEast,
  OS_Cursor_NorthSouth,
  OS_Cursor_NorthWestSouthEast,
  OS_Cursor_NorthEasttSouthWest,
  OS_Cursor_Beam,

  OS_Cursor_COUNT,
};

//////////////////////////////
// NOTE(hampus): Key

enum OS_Key
{
  OS_Key_Null,

  OS_Key_A,
  OS_Key_B,
  OS_Key_C,
  OS_Key_D,
  OS_Key_E,
  OS_Key_F,
  OS_Key_G,
  OS_Key_H,
  OS_Key_I,
  OS_Key_J,
  OS_Key_K,
  OS_Key_L,
  OS_Key_M,
  OS_Key_N,
  OS_Key_O,
  OS_Key_P,
  OS_Key_Q,
  OS_Key_R,
  OS_Key_S,
  OS_Key_T,
  OS_Key_U,
  OS_Key_V,
  OS_Key_W,
  OS_Key_X,
  OS_Key_Y,
  OS_Key_Z,

  OS_Key_0,
  OS_Key_1,
  OS_Key_2,
  OS_Key_3,
  OS_Key_4,
  OS_Key_5,
  OS_Key_6,
  OS_Key_7,
  OS_Key_8,
  OS_Key_9,

  OS_Key_F1,
  OS_Key_F2,
  OS_Key_F3,
  OS_Key_F4,
  OS_Key_F5,
  OS_Key_F6,
  OS_Key_F7,
  OS_Key_F8,
  OS_Key_F9,
  OS_Key_F10,
  OS_Key_F11,
  OS_Key_F12,

  OS_Key_Backspace,
  OS_Key_Space,
  OS_Key_Alt,
  OS_Key_OS,
  OS_Key_Tab,
  OS_Key_Return,
  OS_Key_Shift,
  OS_Key_Control,
  OS_Key_Escape,
  OS_Key_PageUp,
  OS_Key_PageDown,
  OS_Key_End,
  OS_Key_Home,
  OS_Key_Left,
  OS_Key_Right,
  OS_Key_Up,
  OS_Key_Down,
  OS_Key_Delete,
  OS_Key_MouseLeft,
  OS_Key_MouseRight,
  OS_Key_MouseMiddle,

  OS_Key_Plus,
  OS_Key_Minus,

  OS_Key_VolumeUp,
  OS_Key_VolumeDown,

  OS_Key_COUNT,
};

typedef U32 OS_KeyModifiers;
enum
{
  OS_KeyModifier_Ctrl = (1 << 0),
  OS_KeyModifier_Shift = (1 << 1),
  OS_KeyModifier_Alt = (1 << 2),
};

//////////////////////////////
// NOTE(hampus): Event

enum OS_EventKind
{
  OS_EventKind_Quit,
  OS_EventKind_KeyPress,
  OS_EventKind_KeyRelease,
  OS_EventKind_KeyDoublePress,
  OS_EventKind_Char,
  OS_EventKind_Scroll,
  OS_EventKind_MouseMove,
  OS_EventKind_Move,

  // NOTE(hampus): Android-specific
  OS_EventKind_InitWindow,
  OS_EventKind_DestroyWindow,
  OS_EventKind_GainedFocus,
  OS_EventKind_LostFocus,

  // hampus: Gestures
  OS_EventKind_TouchBegin,
  OS_EventKind_TouchEnd,
  OS_EventKind_LongPress,
  OS_EventKind_TapPress,
  OS_EventKind_TapRelease,
  OS_EventKind_DoubleTap,
  OS_EventKind_FlingX,
  OS_EventKind_FlingY,
  OS_EventKind_Back,
};

struct OS_Event
{
  OS_EventKind kind;
  OS_Key key;
  OS_KeyModifiers key_modifiers;
  U32 cp;
  Vec2F32 scroll;
  Vec2F32 mouse;
  Side fling_side;
};

struct OS_EventNode
{
  OS_EventNode *next;
  OS_EventNode *prev;
  OS_Event v;
};

struct OS_EventList
{
  OS_EventNode *first;
  OS_EventNode *last;
  U64 count;
};

//////////////////////////////
// NOTE(hampus): Window types

typedef void OS_WindowRepaintProc(OS_Handle window_os_repaint_handle, void *data);

//////////////////////////////
// NOTE(hampus): Init & destroy

function void os_gfx_init(void);
function void os_gfx_destroy(void);

//////////////////////////////
// NOTE(hampus): Window

function OS_Handle os_window_open(String8 title, U32 width, U32 height);
function void os_window_close(OS_Handle window);
function void os_window_show(OS_Handle handle);
function Vec2U64 os_client_area_from_window(OS_Handle handle);
function RectF32 os_client_rect_from_window(OS_Handle handle);
function void os_window_equip_repaint(OS_Handle window, OS_WindowRepaintProc *proc, void *user_data);
function void os_window_maximize(OS_Handle window);
function void os_window_toggle_fullscreen(OS_Handle window);
function OS_EventList os_events_from_window(Arena *arena, OS_Handle window);
function Vec2F32 os_window_dpi(OS_Handle window); // TODO(hampus): Change this to dots/meter instead of dots/inch?
function Vec2F32 os_mouse_pos(OS_Handle window);
function void os_swap_buffers(OS_Handle handle);

//////////////////////////////
// NOTE(hampus): Cursor

function void os_set_cursor(OS_Cursor cursor);
function void os_set_cursor_visibility(B32 show);

//////////////////////////////
// NOTE(hampus): Clipboard

function String8 os_push_clipboard(Arena *arena);
function void os_set_clipboard(String8 string);
function void os_set_keyboard_visibility(B32 show);

#endif // OS_GFX_H
