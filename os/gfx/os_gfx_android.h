#ifndef OS_GFX_ANDROID_H
#define OS_GFX_ANDROID_H

#include <EGL/egl.h>

struct OS_Android_Window
{
  OS_Android_Window *next;

  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
};

typedef U32 OS_Android_MotionFlags;
enum
{
  OS_Android_MotionFlag_Down = (1 << 0),
  OS_Android_MotionFlag_ScrollingX = (1 << 1),
  OS_Android_MotionFlag_ScrollingY = (1 << 2),
  OS_Android_MotionFlag_OutsideTouchSlopX = (1 << 3),
  OS_Android_MotionFlag_OutsideTouchSlopY = (1 << 4),

  OS_Android_MotionFlag_OutsideTouchSlop = OS_Android_MotionFlag_OutsideTouchSlopX | OS_Android_MotionFlag_OutsideTouchSlopY,
};

enum OS_Android_Gesture
{
  OS_Android_Gesture_Press,
  OS_Android_Gesture_Release,
  OS_Android_Gesture_LongPress,
  OS_Android_Gesture_DoubleTap,
  OS_Android_Gesture_COUNT,
};

enum OS_Android_Inset
{
  OS_Android_Inset_NavBar,
  OS_Android_Inset_StatusBar,
};

struct OS_Android_GestureState
{
  B8 sent;
  B8 disabled;
};

struct OS_Android_MoveAction
{
  Vec2F32 pos;
  S64 timestamp_us;
};

struct OS_Android_MotionState
{
  OS_Android_MotionFlags flags;
  StaticArray<OS_Android_GestureState, OS_Android_Gesture_COUNT> gesture_states;
  S64 down_timestamp_us;
  S64 up_timestamp_us;
  Vec2F32 down_pos;
  StaticArray<OS_Android_MoveAction, 20> move_actions;
  S64 move_actions_pos;
};

struct OS_Android_ViewConfig
{
  S32 touch_slop_px;
  S32 long_press_timeout_ms;
  S32 tap_timeout_ms;
  S32 double_tap_timeout_ms;
  S32 jump_tap_timeout_ms;
  S32 min_fling_velocity_px_s;
  S32 max_fling_velocity_px_s;
  F32 scroll_friction_px_s;
};

struct OS_Android_DisplayMetrics
{
  F32 dpi;
  F32 pixel_density;
  F32 font_scale;
};

struct OS_Android_GfxState
{
  Arena *arena;
  Vec2F32 mouse_pos;

  OS_Android_ViewConfig view_config;
  OS_Android_DisplayMetrics display_metrics;

  OS_Android_MotionState motion_states[2];
  U64 motion_state_pos;

  Vec2F32 scroll_velocity;

  OS_Android_Window *first_free_window;

  B32 finish_all_events;
  B32 showing_keyboard;
  B32 active;
};

static RectF32 os_android_get_inset_rect_px(OS_Android_Inset inset);
static F32 os_android_get_pixel_density();
static F32 os_android_get_font_scale();
static String8 os_android_get_internal_data_path();
static String8 os_android_get_external_data_path();
static String8 os_android_load_asset(Arena *arena, String8 name);

#endif