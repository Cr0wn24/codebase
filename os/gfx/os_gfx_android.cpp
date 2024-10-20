//////////////////////////////
// NOTE(hampus): Globals

global OS_Android_GfxState *os_gfx_android_state;
global Arena *os_android_event_arena;
global OS_EventList *os_android_event_list;

//////////////////////////////
// NOTE(hampus): Android specific functions

function RectF32
os_android_call_inset_function(char *string)
{
  RectF32 result = {};
  TempArena scratch = get_scratch(0, 0);
  SETUP_FOR_JAVA_CALL_THREAD;
  jobject activity_object = android_app->activity->clazz;
  jclass activity_class = env->GetObjectClass(activity_object);
  jmethodID get_inset_mid = env->GetMethodID(activity_class, string, "()[F");
  if(get_inset_mid != 0)
  {
    jfloatArray float_array = (jfloatArray)env->CallObjectMethod(activity_object, get_inset_mid);
    if(float_array != 0)
    {
      jfloat *elements = env->GetFloatArrayElements(float_array, 0);
      if(elements != 0)
      {
        result.y1 = elements[0];
        result.y0 = elements[1];
        result.x0 = elements[2];
        result.x1 = elements[3];

        env->ReleaseFloatArrayElements(float_array, elements, 0);
      }
      else
      {
        // TODO(hampus): Logging
      }
    }
    else
    {
      // TODO(hampus): Logging
    }
  }
  else
  {
    // TODO(hampus): Logging
  }
  JAVA_CALL_DETACH_THREAD;
  return result;
}

function RectF32
os_android_get_inset_rect_px(OS_Android_Inset inset)
{
  local char *inset_to_activity_function_string[] =
  {
    [OS_Android_Inset_StatusBar] = "getStatusBarInset",
    [OS_Android_Inset_NavBar] = "getNavigationBarInset",
  };

  RectF32 result = os_android_call_inset_function(inset_to_activity_function_string[inset]);
  return result;
}

function F32
os_android_get_pixel_density(void)
{
  F32 result = os_gfx_android_state->display_metrics.pixel_density;
  return result;
}

function F32
os_android_get_font_scale(void)
{
  F32 result = os_gfx_android_state->display_metrics.font_scale;
  return result;
}

function S32
os_android_show_yes_no_message_box(String8 title, String8 message)
{
  S32 result = 0;
  TempArena scratch = get_scratch(0, 0);
  SETUP_FOR_JAVA_CALL_THREAD;
  ANativeActivity *activity = android_app->activity;
  jobject activity_object = activity->clazz;
  jclass activity_class = env->GetObjectClass(activity_object);
  jmethodID show_fatal_error_message_box_mid = env->GetMethodID(activity_class, "showYesNoMessageBox", "(Ljava/lang/String;Ljava/lang/String;)I");
  jstring message_jstring = env->NewStringUTF(cstr_from_str8(scratch.arena, message));
  jstring title_jstring = env->NewStringUTF(cstr_from_str8(scratch.arena, title));
  result = env->CallIntMethod(activity_object, show_fatal_error_message_box_mid, title_jstring, message_jstring);
  JAVA_CALL_DETACH_THREAD;

  return result;
}

function String8
os_android_get_internal_data_path(void)
{
  const char *str = android_app->activity->internalDataPath;
  String8 result = str8_cstr((char *)str);
  return result;
}

function String8
os_android_get_external_data_path(void)
{
  const char *str = android_app->activity->externalDataPath;
  String8 result = str8_cstr((char *)str);
  return result;
}

static long
os_android_get_native_heap_allocation(void)
{
  SETUP_FOR_JAVA_CALL_THREAD;
  jclass clazz = env->FindClass("android/os/Debug");
  if(clazz)
  {
    jmethodID mid = env->GetStaticMethodID(clazz, "getNativeHeapAllocatedSize", "()J");
    if(mid)
    {
      return env->CallStaticLongMethod(clazz, mid);
    }
  }
  JAVA_CALL_DETACH_THREAD;
  return -1L;
}

static long
os_android_get_native_heap_size(void)
{
  SETUP_FOR_JAVA_CALL_THREAD;
  jclass clazz = env->FindClass("android/os/Debug");
  if(clazz != 0)
  {
    jmethodID mid = env->GetStaticMethodID(clazz, "getNativeHeapSize", "()J");
    if(mid != 0)
    {
      return env->CallStaticLongMethod(clazz, mid);
    }
  }
  JAVA_CALL_DETACH_THREAD;
  return -1L;
}

function String8
os_android_load_asset(Arena *arena, String8 name)
{
  String8 result = {};
  TempArena scratch = get_scratch(0, 0);
  AAsset *asset = AAssetManager_open(android_app->activity->assetManager, cstr_from_str8(scratch.arena, name), AASSET_MODE_BUFFER);
  if(asset)
  {
    U8 *data = (U8 *)AAsset_getBuffer(asset);
    result.size = AAsset_getLength(asset);
    result.data = push_array<U8>(arena, result.size);
    memory_copy(result.data, data, result.size);
    AAsset_close(asset);
  }

  return result;
}

function int
os_android_get_unicode_char(int eventType, int keyCode, int metaState)
{
  SETUP_FOR_JAVA_CALL_THREAD;

  jclass class_key_event = env->FindClass("android/view/KeyEvent");
  int unicodeKey;

  if(metaState == 0)
  {
    jmethodID method_get_unicode_char = env->GetMethodID(class_key_event, "getUnicodeChar", "()I");
    jmethodID eventConstructor = env->GetMethodID(class_key_event, "<init>", "(II)V");
    jobject eventObj = env->NewObject(class_key_event, eventConstructor, eventType, keyCode);

    unicodeKey = env->CallIntMethod(eventObj, method_get_unicode_char);
  }
  else
  {
    jmethodID method_get_unicode_char = env->GetMethodID(class_key_event, "getUnicodeChar", "(I)I");
    jmethodID eventConstructor = env->GetMethodID(class_key_event, "<init>", "(II)V");
    jobject eventObj = env->NewObject(class_key_event, eventConstructor, eventType, keyCode);

    unicodeKey = env->CallIntMethod(eventObj, method_get_unicode_char, metaState);
  }

  JAVA_CALL_DETACH_THREAD;
  return unicodeKey;
}

function void
os_android_post_event(OS_Event event)
{
  OS_EventNode *event_node = push_array<OS_EventNode>(os_android_event_arena, 1);
  event_node->v = event;
  dll_push_back(os_android_event_list->first, os_android_event_list->last, event_node);
  os_android_event_list->count += 1;
}

function B32
os_android_gesture_is_disabled(OS_Android_MotionState *motion_state, OS_Android_Gesture gesture)
{
  B32 result = motion_state->gesture_states[gesture].disabled;
  return result;
}

function B32
os_android_gesture_is_sent(OS_Android_MotionState *motion_state, OS_Android_Gesture gesture)
{
  B32 result = motion_state->gesture_states[gesture].sent;
  return result;
}

function void
os_android_disable_gesture(OS_Android_MotionState *motion_state, OS_Android_Gesture gesture)
{
  motion_state->gesture_states[gesture].disabled = true;
}

function void
os_android_mark_gesture_as_sent(OS_Android_MotionState *motion_state, OS_Android_Gesture gesture)
{
  if(!motion_state->gesture_states[gesture].disabled)
  {
    motion_state->gesture_states[gesture].sent = true;
  }
}

function Vec2F32
os_android_clamp_fling_velocity(Vec2F32 velocity)
{
  Vec2F32 result = {};
  result.x = min(velocity.x, (F32)os_gfx_android_state->view_config.max_fling_velocity_px_s);
  result.y = min(velocity.y, (F32)os_gfx_android_state->view_config.max_fling_velocity_px_s);
  result.x = max(velocity.x, -(F32)os_gfx_android_state->view_config.max_fling_velocity_px_s);
  result.y = max(velocity.y, -(F32)os_gfx_android_state->view_config.max_fling_velocity_px_s);
  return result;
}

function S32
os_android_input_callback(struct android_app *android_app, AInputEvent *event)
{
  S32 type = AInputEvent_getType(event);
  switch(type)
  {
    case AINPUT_EVENT_TYPE_KEY:
    {
      S32 key_code = AKeyEvent_getKeyCode(event);

      S32 action = AKeyEvent_getAction(event);
      S32 meta = AKeyEvent_getMetaState(event);

      if(action == AKEY_EVENT_ACTION_DOWN)
      {
        if(key_code == AKEYCODE_VOLUME_DOWN)
        {
          os_gfx_android_state->display_metrics.font_scale -= 0.1f;
        }
        else if(key_code == AKEYCODE_VOLUME_UP)
        {
          os_gfx_android_state->display_metrics.font_scale += 0.1f;
        }
        else if(key_code == AKEYCODE_BACK)
        {
          OS_Event event = {.kind = OS_EventKind_Back};
          os_android_post_event(event);
        }
        else if(key_code == AKEYCODE_VOLUME_UP)
        {
        }
        else
        {
          U32 character = os_android_get_unicode_char(type, key_code, meta);
        }
      }
      else if(action == AKEY_EVENT_ACTION_UP)
      {
      }
      return 1;
    }
    break;
    case AINPUT_EVENT_TYPE_MOTION:
    {
      S32 action_code = AMotionEvent_getAction(event);

      // NOTE(hampus): From https://developer.android.com/ndk/reference/group/input#anonymous-enum-43

      S32 event_action_mask = AMOTION_EVENT_ACTION_MASK;
      S32 pointer_idx_mask = AMOTION_EVENT_ACTION_POINTER_INDEX_MASK;
      S32 pointer_idx_shift = AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

      S32 action = action_code & event_action_mask;
      S32 pointer_idx = (action_code & pointer_idx_mask) >> pointer_idx_shift;

      Vec2F32 primary_pointer_pos = v2f32(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0));

      OS_Android_MotionState *current_motion_state = &os_gfx_android_state->motion_states[os_gfx_android_state->motion_state_pos % 2];
      OS_Android_MotionState *prev_motion_state = &os_gfx_android_state->motion_states[(os_gfx_android_state->motion_state_pos + 1) % 2];

      os_gfx_android_state->mouse_pos = primary_pointer_pos;

      S64 event_time_us = AMotionEvent_getEventTime(event) / 1000;
      // S64 event_time_us = os_get_microseconds();
      switch(action)
      {
        /** A pressed gesture has started, the motion contains the initial starting location. */
        case AMOTION_EVENT_ACTION_DOWN:
        {
          OS_Event event = {.kind = OS_EventKind_TouchBegin};
          os_android_post_event(event);
          current_motion_state->down_timestamp_us = event_time_us;
          current_motion_state->down_pos = primary_pointer_pos;
          current_motion_state->flags |= OS_Android_MotionFlag_Down;
          os_gfx_android_state->scroll_velocity = v2f32(0, 0);
        }
        break;

        /**
         * A pressed gesture has finished, the motion contains the final release location
         * as well as any intermediate points since the last down or move event.
         */
        case AMOTION_EVENT_ACTION_UP:
        {
          OS_Event event = {.kind = OS_EventKind_TouchEnd};
          os_android_post_event(event);
          if(!os_android_gesture_is_sent(current_motion_state, OS_Android_Gesture_Press) &&
             !os_android_gesture_is_disabled(current_motion_state, OS_Android_Gesture_Press))
          {
            if(current_motion_state->flags & OS_Android_MotionFlag_Down)
            {
              if((os_get_microseconds() - current_motion_state->down_timestamp_us) <= os_gfx_android_state->view_config.tap_timeout_ms * 1000)
              {
                OS_Event event = {.kind = OS_EventKind_TapPress};
                os_android_post_event(event);
                os_android_mark_gesture_as_sent(current_motion_state, OS_Android_Gesture_Press);
              }
            }
          }

          if(os_android_gesture_is_sent(current_motion_state, OS_Android_Gesture_Press))
          {
            OS_Event event = {.kind = OS_EventKind_TapRelease};
            os_android_post_event(event);
            os_android_mark_gesture_as_sent(current_motion_state, OS_Android_Gesture_Release);
          }

          if(current_motion_state->flags & OS_Android_MotionFlag_OutsideTouchSlop)
          {
            if((event_time_us - current_motion_state->down_timestamp_us) < 300 * 1000)
            {
              Vec2F32 velocity_sum = {};
              S64 sample_size = min(array_count(current_motion_state->move_actions), current_motion_state->move_actions_pos);
              for(S64 move_action_idx = 0; move_action_idx < sample_size - 2; ++move_action_idx)
              {
                S64 idx = current_motion_state->move_actions_pos - move_action_idx - 2;
                OS_Android_MoveAction *latest_move_action = &current_motion_state->move_actions[idx % sample_size];
                OS_Android_MoveAction *before_latest_move_action = &current_motion_state->move_actions[(idx - 1) % sample_size];
                Vec2F32 delta = latest_move_action->pos - before_latest_move_action->pos;
                S64 dt_us = latest_move_action->timestamp_us - before_latest_move_action->timestamp_us;
                F32 dt_s = (dt_us / (F32)million(1));
                Vec2F32 velocity = delta * dt_s;
                Vec2F32 clamped_velocity = os_android_clamp_fling_velocity(velocity);
                velocity_sum += clamped_velocity;
              }
              Vec2F32 velocity = velocity_sum / (F32)sample_size;
              for_each_enum_val(Axis2, axis)
              {
                if(abs(velocity[axis]) >= (F32)os_gfx_android_state->view_config.min_fling_velocity_px_s)
                {
                  os_gfx_android_state->scroll_velocity[axis] = -velocity[axis];
                  OS_Event event = {};
                  event.kind = axis == Axis2_X ? OS_EventKind_FlingX : OS_EventKind_FlingY;
                  if(velocity[axis] > 0)
                  {
                    event.fling_side = Side_Max;
                  }
                  else
                  {
                    event.fling_side = Side_Min;
                  }
                  os_android_post_event(event);
                }
              }
            }
            else
            {
            }
          }

          current_motion_state->up_timestamp_us = event_time_us;

          os_gfx_android_state->motion_state_pos += 1;
          memory_zero_struct(&os_gfx_android_state->motion_states[os_gfx_android_state->motion_state_pos % 2]);
        }
        break;

        /**
         * A change has happened during a press gesture (between #AMOTION_EVENT_ACTION_DOWN and
         * #AMOTION_EVENT_ACTION_UP).  The motion contains the most recent point, as well as
         * any intermediate points since the last down or move event.
         */
        case AMOTION_EVENT_ACTION_MOVE:
        {
          OS_Android_MoveAction *move_action = &current_motion_state->move_actions[current_motion_state->move_actions_pos % array_count(current_motion_state->move_actions)];
          Vec2F32 oldest_pos = v2f32(AMotionEvent_getHistoricalX(event, 0, 0), AMotionEvent_getHistoricalY(event, 0, 0));
          move_action->pos = oldest_pos;
          move_action->timestamp_us = event_time_us;
          current_motion_state->move_actions_pos += 1;
          {
            F32 allowed_touch_slop = (F32)os_gfx_android_state->view_config.touch_slop_px;
            if(abs(current_motion_state->down_pos.x - primary_pointer_pos.x) > allowed_touch_slop)
            {
              current_motion_state->flags |= OS_Android_MotionFlag_OutsideTouchSlopX;
            }
            if(abs(current_motion_state->down_pos.y - primary_pointer_pos.y) > allowed_touch_slop)
            {
              current_motion_state->flags |= OS_Android_MotionFlag_OutsideTouchSlopY;
            }
            if(current_motion_state->flags & OS_Android_MotionFlag_OutsideTouchSlop)
            {
              os_android_disable_gesture(current_motion_state, OS_Android_Gesture_DoubleTap);
              os_android_disable_gesture(current_motion_state, OS_Android_Gesture_Press);
              os_android_disable_gesture(current_motion_state, OS_Android_Gesture_LongPress);
            }
          }
#if 0
          if(current_motion_state->move_actions_pos >= 2)
          {
            OS_Android_MoveAction *latest_move_action = &current_motion_state->move_actions[(current_motion_state->move_actions_pos - 1) % 20];
            OS_Android_MoveAction *before_latest_move_action = &current_motion_state->move_actions[(current_motion_state->move_actions_pos - 2) % 20];
            Vec2F32 delta = {};
            delta.x = latest_move_action->pos.x - before_latest_move_action->pos.x;
            delta.y = latest_move_action->pos.y - before_latest_move_action->pos.y;
            S64 dt_us = latest_move_action->timestamp_us - before_latest_move_action->timestamp_us;
            Vec2F32 velocity = v2f32(-delta.x / (dt_us / (F32)million(1)), -delta.y / (dt_us / (F32)million(1)));
            os_print_debug_string("vel: %.2f, %.2f", velocity.x, velocity.y);
          }
#endif
          if(current_motion_state->flags & OS_Android_MotionFlag_OutsideTouchSlopX)
          {
            current_motion_state->flags |= OS_Android_MotionFlag_ScrollingX;
          }

          if(current_motion_state->flags & OS_Android_MotionFlag_OutsideTouchSlopY)
          {
            current_motion_state->flags |= OS_Android_MotionFlag_ScrollingY;
          }

          if(AMotionEvent_getHistorySize(event) > 0)
          {
            Vec2F32 latest_pos = v2f32(AMotionEvent_getHistoricalX(event, 0, 1), AMotionEvent_getHistoricalY(event, 0, 1));
            Vec2F32 delta = v2f32(oldest_pos.x - latest_pos.x, oldest_pos.y - latest_pos.y);
            Vec2F32 scroll = {};
            if(current_motion_state->flags & OS_Android_MotionFlag_ScrollingX)
            {
              scroll.x = delta.x;
            }
            if(current_motion_state->flags & OS_Android_MotionFlag_ScrollingY)
            {
              scroll.y = delta.y;
            }
            if(abs(scroll.x) > 0 || abs(scroll.y) > 0)
            {
              OS_Event event = {.kind = OS_EventKind_Scroll, .scroll = scroll};
              os_android_post_event(event);
            }
          }
        }
        break;

        /**
         * The current gesture has been aborted.
         * You will not receive any more points in it.  You should treat this as
         * an up event, but not perform any action that you normally would.
         */
        case AMOTION_EVENT_ACTION_CANCEL:
        {
          OS_Event event = {.kind = OS_EventKind_Back};
          os_android_post_event(event);
        }
        break;

        /**
         * A movement has happened outside of the normal bounds of the UI element.
         * This does not provide a full gesture, but only the initial location of the movement/touch.
         */
        case AMOTION_EVENT_ACTION_OUTSIDE:
        {
        }
        break;

        /**
         * A non-primary pointer has gone down.
         * The bits in #AMOTION_EVENT_ACTION_POINTER_INDEX_MASK indicate which pointer changed.
         */
        case AMOTION_EVENT_ACTION_POINTER_DOWN:
        {
        }
        break;

        /**
         * A non-primary pointer has gone up.
         * The bits in #AMOTION_EVENT_ACTION_POINTER_INDEX_MASK indicate which pointer changed.
         */
        case AMOTION_EVENT_ACTION_POINTER_UP:
        {
        }
        break;

        /**
         * A change happened but the pointer is not down (unlike #AMOTION_EVENT_ACTION_MOVE).
         * The motion contains the most recent point, as well as any intermediate points since
         * the last hover move event.
         */
        case AMOTION_EVENT_ACTION_HOVER_MOVE:
        {
        }
        break;

        /**
         * The motion event contains relative vertical and/or horizontal scroll offsets.
         * Use {@link AMotionEvent_getAxisValue} to retrieve the information from
         * #AMOTION_EVENT_AXIS_VSCROLL and #AMOTION_EVENT_AXIS_HSCROLL.
         * The pointer may or may not be down when this event is dispatched.
         * This action is always delivered to the winder under the pointer, which
         * may not be the window currently touched.
         */
        case AMOTION_EVENT_ACTION_SCROLL:
        {
        }
        break;

        /** The pointer is not down but has entered the boundaries of a window or view. */
        case AMOTION_EVENT_ACTION_HOVER_ENTER:
        {
        }
        break;

        /** The pointer is not down but has exited the boundaries of a window or view. */
        case AMOTION_EVENT_ACTION_HOVER_EXIT:
        {
        }
        break;

        /* One or more buttons have been pressed. */
        case AMOTION_EVENT_ACTION_BUTTON_PRESS:
        {
        }
        break;

        /* One or more buttons have been released. */
        case AMOTION_EVENT_ACTION_BUTTON_RELEASE:
        {
        }
        break;
          invalid_case;
      }

      return 1;
    }
    break;
    case AINPUT_EVENT_TYPE_FOCUS:
    {
      os_print_debug_string(str8_lit("AINPUT_EVENT_TYPE_FOCUS"));
      return 1;
    }
    break;
    case AINPUT_EVENT_TYPE_CAPTURE:
    {
      os_print_debug_string(str8_lit("AINPUT_EVENT_TYPE_CAPTURE"));
      return 1;
    }
    break;
    case AINPUT_EVENT_TYPE_DRAG:
    {
      os_print_debug_string(str8_lit("AINPUT_EVENT_TYPE_DRAG"));
      return 1;
    }
    break;
    case AINPUT_EVENT_TYPE_TOUCH_MODE:
    {
      os_print_debug_string(str8_lit("AINPUT_EVENT_TYPE_TOUCH_MODE"));
      return 1;
    }
    break;
  }

  return 0;
}

function void
os_android_cmd_callback(struct android_app *android_app, S32 cmd)
{
  switch(cmd)
  {
    case APP_CMD_INIT_WINDOW:
    {
      OS_Event event = {.kind = OS_EventKind_InitWindow};
      os_android_post_event(event);
    }
    break;
    case APP_CMD_TERM_WINDOW:
    {
      OS_Event event = {.kind = OS_EventKind_DestroyWindow};
      os_android_post_event(event);
      os_gfx_android_state->finish_all_events = true;
    }
    break;
    case APP_CMD_GAINED_FOCUS:
    {
      OS_Event event = {.kind = OS_EventKind_GainedFocus};
      os_android_post_event(event);
      os_gfx_android_state->active = true;
    }
    break;
    case APP_CMD_LOST_FOCUS:
    {
      OS_Event event = {.kind = OS_EventKind_LostFocus};
      os_android_post_event(event);
      os_gfx_android_state->active = false;
    }
    break;
  }
}

//////////////////////////////
// NOTE(hampus): Init

function void
os_gfx_init(void)
{
  Arena *arena = arena_alloc();
  os_gfx_android_state = push_array<OS_Android_GfxState>(arena, 1);
  os_gfx_android_state->arena = arena;

  android_app->onAppCmd = os_android_cmd_callback;
  android_app->onInputEvent = os_android_input_callback;

  SETUP_FOR_JAVA_CALL_THREAD;

  jobject native_activity = android_app->activity->clazz;

  jclass view_config_class = env->FindClass("android/view/ViewConfiguration");
  if(view_config_class != 0)
  {
    OS_Android_ViewConfig *view_config = &os_gfx_android_state->view_config;

    // NOTE(hampus): Non-static functions

    jmethodID get_method = env->GetStaticMethodID(view_config_class, "get", "(Landroid/content/Context;)Landroid/view/ViewConfiguration;");
    if(get_method != 0)
    {
      jobject view_config_object = env->CallStaticObjectMethod(view_config_class, get_method, native_activity);
      if(view_config_object != 0)
      {
        jmethodID get_scaled_touch_mode_method = env->GetMethodID(view_config_class, "getScaledTouchSlop", "()I");
        if(get_scaled_touch_mode_method != 0)
        {
          view_config->touch_slop_px = env->CallIntMethod(view_config_object, get_scaled_touch_mode_method);
          os_print_debug_string("Got touch slop value: %" PRIS32 " pixels", view_config->touch_slop_px);
        }
        else
        {
          // TODO(hampus): Logging
        }

        jmethodID get_minimum_fling_velocity = env->GetMethodID(view_config_class, "getScaledMinimumFlingVelocity", "()I");
        if(get_minimum_fling_velocity != 0)
        {
          view_config->min_fling_velocity_px_s = env->CallIntMethod(view_config_object, get_minimum_fling_velocity);
          os_print_debug_string("Got minimum fling velocity: %" PRIS32 "px/s", view_config->min_fling_velocity_px_s);
        }
        else
        {
          // TODO(hampus): Logging
        }
        jmethodID get_maxiumum_fling_velocity = env->GetMethodID(view_config_class, "getScaledMaximumFlingVelocity", "()I");
        if(get_maxiumum_fling_velocity != 0)
        {
          view_config->max_fling_velocity_px_s = env->CallIntMethod(view_config_object, get_maxiumum_fling_velocity);
          os_print_debug_string("Got maximum fling velocity: %" PRIS32 "px/s", view_config->max_fling_velocity_px_s);
        }
        else
        {
          // TODO(hampus): Logging
        }
      }
      else
      {
        // TODO(hampus): Logging
      }
    }
    else
    {
      // TODO(hampus): Logging
    }

    // NOTE(hampus): Static functions

    jmethodID get_long_press_timeout = env->GetStaticMethodID(view_config_class, "getLongPressTimeout", "()I");
    if(get_long_press_timeout != 0)
    {
      view_config->long_press_timeout_ms = env->CallStaticIntMethod(view_config_class, get_long_press_timeout);
      os_print_debug_string("Got long press timeout value: %" PRIS32 "ms", view_config->long_press_timeout_ms);
    }
    else
    {
      // TODO(hampus): Logging
    }
    jmethodID get_tap_timeout = env->GetStaticMethodID(view_config_class, "getTapTimeout", "()I");
    if(get_tap_timeout != 0)
    {
      view_config->tap_timeout_ms = env->CallStaticIntMethod(view_config_class, get_tap_timeout);
      os_print_debug_string("Got tap timeout value: %" PRIS32 "ms", view_config->tap_timeout_ms);
    }
    else
    {
      // TODO(hampus): Logging
    }
    jmethodID get_double_tap_timeout = env->GetStaticMethodID(view_config_class, "getDoubleTapTimeout", "()I");
    if(get_double_tap_timeout != 0)
    {
      view_config->double_tap_timeout_ms = env->CallStaticIntMethod(view_config_class, get_double_tap_timeout);
      os_print_debug_string("Got double tap timeout value: %" PRIS32 "ms", view_config->double_tap_timeout_ms);
    }
    else
    {
      // TODO(hampus): Logging
    }
    jmethodID get_jump_tap_timeout = env->GetStaticMethodID(view_config_class, "getJumpTapTimeout", "()I");
    if(get_jump_tap_timeout != 0)
    {
      view_config->jump_tap_timeout_ms = env->CallStaticIntMethod(view_config_class, get_jump_tap_timeout);
      os_print_debug_string("Got jump tap timeout value: %" PRIS32 "ms", view_config->jump_tap_timeout_ms);
    }
    else
    {
      // TODO(hampus): Logging
    }
    jmethodID get_scroll_friction = env->GetStaticMethodID(view_config_class, "getScrollFriction", "()F");
    if(get_scroll_friction != 0)
    {
      view_config->scroll_friction_px_s = env->CallStaticFloatMethod(view_config_class, get_scroll_friction);
      os_print_debug_string("Got scroll friction value: %.4f", view_config->scroll_friction_px_s);
    }
    else
    {
      // TODO(hampus): Logging
    }
  }
  else
  {
    // TODO(hampus): Logging
  }

  jobject activity_object = android_app->activity->clazz;
  jclass activity_class = env->GetObjectClass(activity_object);

  int32_t density = AConfiguration_getDensity(android_app->config);
  if(density != 0)
  {
    os_gfx_android_state->display_metrics.dpi = (F32)density;
  }
  else
  {
    jmethodID get_dpi_mid = env->GetMethodID(activity_class, "getDpi", "()I");
    if(get_dpi_mid != 0)
    {
      S32 dpi_s32 = env->CallIntMethod(activity_object, get_dpi_mid);
    }
    else
    {
      // TODO(hampus): Logging
    }
  }

  if(os_gfx_android_state->display_metrics.dpi != 0)
  {
    os_print_debug_string("Got DPI: %.2f", os_gfx_android_state->display_metrics.dpi);
  }

  jmethodID get_pixel_density_mid = env->GetMethodID(activity_class, "getPixelDensity", "()F");
  if(get_pixel_density_mid != 0)
  {
    os_gfx_android_state->display_metrics.pixel_density = env->CallFloatMethod(activity_object, get_pixel_density_mid);
    os_print_debug_string("Got pixel density: %.2f", os_gfx_android_state->display_metrics.pixel_density);
  }
  else
  {
    // TODO(hampus): Logging
  }

  jmethodID get_font_scale_mid = env->GetMethodID(activity_class, "getFontScale", "()F");
  if(get_font_scale_mid != 0)
  {
    os_gfx_android_state->display_metrics.font_scale = env->CallFloatMethod(activity_object, get_font_scale_mid);
    os_print_debug_string("Got font scale: %.2f", os_gfx_android_state->display_metrics.font_scale);
  }
  else
  {
    // TODO(hampus): Logging
  }

  JAVA_CALL_DETACH_THREAD;
}

function void
os_gfx_destroy(void)
{
  arena_free(os_gfx_android_state->arena);
  os_gfx_android_state = 0;
}

//////////////////////////////
// NOTE(hampus): Window

function OS_Handle
os_window_open(String8 title, U32 width, U32 height)
{
  OS_Android_Window *window = os_gfx_android_state->first_free_window;
  if(window == 0)
  {
    window = push_array<OS_Android_Window>(os_gfx_android_state->arena, 1);
  }
  else
  {
    sll_stack_pop(os_gfx_android_state->first_free_window);
    memory_zero_struct(window);
  }
  EGLint attribs[] =
  {
    EGL_SURFACE_TYPE,
    EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE,
    EGL_OPENGL_ES2_BIT,
    EGL_BLUE_SIZE,
    8,
    EGL_GREEN_SIZE,
    8,
    EGL_RED_SIZE,
    8,
    EGL_NONE,
  };

  if((window->display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) != EGL_NO_DISPLAY)
  {
    if(eglInitialize(window->display, 0, 0))
    {
      EGLConfig config;
      EGLint numConfigs;
      if(eglChooseConfig(window->display, attribs, &config, 1, &numConfigs))
      {
        EGLint format;
        if(eglGetConfigAttrib(window->display, config, EGL_NATIVE_VISUAL_ID, &format))
        {
          ANativeWindow_setBuffersGeometry(android_app->window, 0, 0, format);
          if((window->surface = eglCreateWindowSurface(window->display, config, android_app->window, 0)))
          {
            EGLint ctx_attrib[] =
            {
              EGL_CONTEXT_MAJOR_VERSION,
              3,
              EGL_CONTEXT_MINOR_VERSION,
              0,
              EGL_NONE,
            };
            if((window->context = eglCreateContext(window->display, config, 0, ctx_attrib)))
            {
              if(eglMakeCurrent(window->display, window->surface, window->surface, window->context) != EGL_FALSE)
              {
              }
              else
              {
                // TODO(hampus): Logging
              }
            }
            else
            {
              // TODO(hampus): Logging
            }
          }
          else
          {
            // TODO(hampus): Logging
          }
        }
        else
        {
          // TODO(hampus): Logging
        }
      }
      else
      {
        // TODO(hampus): Logging
      }
    }
    else
    {
      // TODO(hampus): Logging
    }
  }
  else
  {
    // TODO(hampus): Logging
  }

  OS_Handle result = {};
  result.u64[0] = int_from_ptr(window);
  return result;
}

function void
os_window_close(OS_Handle handle)
{
  OS_Android_Window *window = (OS_Android_Window *)ptr_from_int(handle.u64[0]);
  if(window->display != EGL_NO_DISPLAY)
  {
    eglMakeCurrent(window->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if(window->context != EGL_NO_CONTEXT)
    {
      eglDestroyContext(window->display, window->context);
    }
    if(window->surface != EGL_NO_SURFACE)
    {
      eglDestroySurface(window->display, window->surface);
    }
    eglTerminate(window->display);
  }
  sll_stack_push(os_gfx_android_state->first_free_window, window);
}

function void
os_window_show(OS_Handle handle)
{
  not_implemented;
}

function Vec2U64
os_client_area_from_window(OS_Handle handle)
{
  OS_Android_Window *window = (OS_Android_Window *)ptr_from_int(handle.u64[0]);
  Vec2U64 result = {};
  EGLint width = 0;
  EGLint height = 0;
  eglQuerySurface(window->display, window->surface, EGL_WIDTH, &width);
  eglQuerySurface(window->display, window->surface, EGL_HEIGHT, &height);
  result.x = (U64)width;
  result.y = (U64)height;
  return result;
}

function void
os_window_equip_repaint(OS_Handle window, OS_WindowRepaintProc *proc, void *user_data)
{
  not_implemented;
}

function void
os_window_maximize(OS_Handle window)
{
  not_implemented;
}

function void
os_window_toggle_fullscreen(OS_Handle window)
{
  not_implemented;
}

function OS_EventList
os_events_from_window(Arena *arena, OS_Handle window)
{
  static F32 dt = 0;
  static U64 start_counter_us = 0;
  dt = (F32)(os_get_microseconds() - start_counter_us) / (F32)million(1);
  start_counter_us = os_get_microseconds();

  OS_EventList result = {};
  os_android_event_list = &result;
  os_android_event_arena = arena;

  S32 ident = 0;
  S32 events = 0;
  struct android_poll_source *source = 0;

  while((ident = ALooper_pollOnce(0, 0, &events, (void **)&source)) >= 0)
  {
    if(source != 0)
    {
      source->process(android_app, source);
    }

    if(android_app->destroyRequested != 0)
    {
      os_gfx_android_state->active = false;
      {
        OS_Event event = {.kind = OS_EventKind_DestroyWindow};
        os_android_post_event(event);
      }
      {
        OS_Event event = {.kind = OS_EventKind_Quit};
        os_android_post_event(event);
      }
    }
  }

  OS_Android_MotionState *current_motion_state = &os_gfx_android_state->motion_states[os_gfx_android_state->motion_state_pos % 2];
  OS_Android_MotionState *prev_motion_state = &os_gfx_android_state->motion_states[(os_gfx_android_state->motion_state_pos + 1) % 2];

  if(!os_android_gesture_is_sent(current_motion_state, OS_Android_Gesture_Press) &&
     !os_android_gesture_is_disabled(current_motion_state, OS_Android_Gesture_Press))
  {
    if(current_motion_state->flags & OS_Android_MotionFlag_Down)
    {
      if((current_motion_state->down_timestamp_us + os_gfx_android_state->view_config.tap_timeout_ms * 1000) <= os_get_microseconds())
      {
        OS_Event event = {.kind = OS_EventKind_TapPress};
        os_android_post_event(event);
        os_android_mark_gesture_as_sent(current_motion_state, OS_Android_Gesture_Press);
      }
    }
  }

  if(!os_android_gesture_is_sent(current_motion_state, OS_Android_Gesture_LongPress) &&
     !os_android_gesture_is_disabled(current_motion_state, OS_Android_Gesture_LongPress))
  {
    if(current_motion_state->flags & OS_Android_MotionFlag_Down)
    {
      if((current_motion_state->down_timestamp_us + os_gfx_android_state->view_config.long_press_timeout_ms * 1000) <= os_get_microseconds())
      {
        OS_Event event = {.kind = OS_EventKind_LongPress};
        os_android_post_event(event);
        os_android_mark_gesture_as_sent(current_motion_state, OS_Android_Gesture_LongPress);
        os_android_disable_gesture(current_motion_state, OS_Android_Gesture_Release);
      }
    }
  }

  if(current_motion_state->flags & OS_Android_MotionFlag_Down &&
     !os_android_gesture_is_sent(current_motion_state, OS_Android_Gesture_DoubleTap) &&
     !os_android_gesture_is_disabled(prev_motion_state, OS_Android_Gesture_DoubleTap) &&
     !os_android_gesture_is_sent(prev_motion_state, OS_Android_Gesture_DoubleTap))
  {
    if((prev_motion_state->up_timestamp_us + os_gfx_android_state->view_config.double_tap_timeout_ms * 1000) >= os_get_microseconds())
    {
      OS_Event event = {.kind = OS_EventKind_DoubleTap};
      os_android_post_event(event);
      os_android_mark_gesture_as_sent(current_motion_state, OS_Android_Gesture_DoubleTap);
    }
  }

  if(os_gfx_android_state->scroll_velocity.x != 0 || os_gfx_android_state->scroll_velocity.y != 0)
  {
    Vec2F32 ddp = os_gfx_android_state->scroll_velocity * -os_gfx_android_state->view_config.scroll_friction_px_s * 100;
    Vec2F32 scroll = ddp * 0.5f * square(dt) + os_gfx_android_state->scroll_velocity * dt;
    os_gfx_android_state->scroll_velocity += ddp * dt;
    OS_Event event = {};
    event.scroll = scroll;
    event.kind = OS_EventKind_Scroll;
    os_android_post_event(event);
  }

  os_gfx_android_state->finish_all_events = false;

  os_android_event_list = 0;
  os_android_event_arena = 0;

  return result;
}

function Vec2F32
os_window_dpi(OS_Handle handle)
{
  Vec2F32 result = v2f32(os_gfx_android_state->display_metrics.dpi, os_gfx_android_state->display_metrics.dpi);
  return result;
}

function Vec2F32
os_mouse_pos(OS_Handle window)
{
  // TODO(hampus): not thread-safe
  Vec2F32 result = os_gfx_android_state->mouse_pos;
  return result;
}

function void
os_swap_buffers(OS_Handle handle)
{
  OS_Android_Window *window = (OS_Android_Window *)ptr_from_int(handle.u64[0]);
  eglSwapBuffers(window->display, window->surface);
}

//////////////////////////////
// NOTE(hampus): Cursor

function void
os_set_cursor(OS_Cursor cursor)
{
  // TODO(hampus): What if they have a mouse connected?
}

//////////////////////////////
// NOTE(hampus): Clipboard

function String8
os_push_clipboard(Arena *arena)
{
  not_implemented;
  String8 result = {};
  return result;
}

function void
os_set_clipboard(String8 string)
{
  not_implemented;
}

function void
os_set_keyboard_visibility(B32 show)
{
  SETUP_FOR_JAVA_CALL_THREAD;
  ANativeActivity *activity = android_app->activity;
  jobject activity_object = activity->clazz;
  jclass activity_class = env->GetObjectClass(activity_object);
  jmethodID show_keyboard_mid = env->GetMethodID(activity_class, "showKeyboard", "()V");
  env->CallVoidMethod(activity_object, show_keyboard_mid);
  JAVA_CALL_DETACH_THREAD;
  os_gfx_android_state->showing_keyboard = show;
}

function B32
os_showing_keyboard(void)
{
  return os_gfx_android_state->showing_keyboard;
}