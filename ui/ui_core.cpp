META_EMBED_FILE("../assets/fontello.ttf", fontello_ttf_data)
META_EMBED_FILE("../assets/Roboto-Bold.ttf", roboto_regular_ttf_data)

//////////////////////////////
// NOTE(hampus): Helpers

function Arena *
ui_frame_arena(void)
{
  Arena *arena = ui_state->frame_arenas[ui_state->build_idx % array_count(ui_state->frame_arenas)];
  return arena;
}

function String8
ui_str8_from_icon(UI_Icon icon)
{
  String8 string = ui_state->ui_icon_to_string_table[icon];
  return string;
}

//////////////////////////////
// NOTE(hampus): Keying

function UI_Key
ui_key_zero(void)
{
  UI_Key key = {};
  return key;
}

function U64
ui_hash_from_seed_string(U64 seed, String8 string)
{
  U64 hash = 0;
  if(string.size != 0)
  {
    hash = seed;
    for(U64 i = 0; i < string.size; ++i)
    {
      hash = ((hash << 5) + hash) + string[i];
    }
  }
  return hash;
}

function UI_Key
ui_key_from_string(U64 seed, String8 string)
{
  UI_Key key = {};
  key.u64[0] = ui_hash_from_seed_string(seed, string);
  return key;
}

//////////////////////////////
// NOTE(hampus): Contex Menu

function B32
ui_ctx_menu_begin(UI_Key key)
{
  B32 is_open = key == ui_state->ctx_menu_key;

  ui_push_parent(ui_state->ctx_menu_root);
  ui_push_seed(key.u64[0]);

  if(is_open)
  {
#if 0
        ui_next_extra_box_flags(UI_BoxFlag_DrawBackground |
                                UI_BoxFlag_DrawBorder |
                                UI_BoxFlag_AnimateHeight |
                                UI_BoxFlag_DrawDropShadow |
                                UI_BoxFlag_Clip);
#endif
  }

#if 0    
    F32 r = ui_top_font_line_height() * 0.1f;
    ui_next_corner_radius(v4f32(r, r, r, r));
    ui_next_pref_width(ui_children_sum(1));
    ui_next_pref_height(ui_children_sum(1));
    ui_begin_column();
#endif

  return (is_open);
}

function void
ui_ctx_menu_end(void)
{
#if 0
    ui_end_column();
#endif
  ui_pop_seed();
  ui_pop_parent();
}

function void
ui_ctx_menu_open(UI_Key anchor, Vec2F32 offset, UI_Key menu)
{
  ui_state->next_ctx_menu_key = menu;
  ui_state->next_ctx_menu_anchor_key = anchor;
  ui_state->next_anchor_offset = offset;
}

function void
ui_ctx_menu_close(void)
{
  ui_state->next_ctx_menu_key = ui_key_zero();
  ui_state->next_ctx_menu_anchor_key = ui_key_zero();
}

function B32
ui_ctx_menu_is_open(void)
{
  B32 result = ui_state->ctx_menu_key != ui_key_zero();
  return result;
}

function UI_Key
ui_ctx_menu_key(void)
{
  UI_Key result = ui_state->ctx_menu_key;
  return result;
}

//////////////////////////////
// NOTE(hampus): Text action

function UI_TextAction
ui_text_action_from_event(OS_Event *event)
{
  UI_TextAction result = {};

  if(event->key_modifiers & OS_KeyModifier_Ctrl)
  {
    result.flags |= UI_TextActionFlag_WordScan;
  }

  if(event->key_modifiers & OS_KeyModifier_Shift)
  {
    result.flags |= UI_TextActionFlag_KeepMark;
  }
  if(event->kind == OS_EventKind_KeyPress)
  {
    switch(event->key)
    {
      case OS_Key_Left:
      {
        result.delta.x = -1;
        result.flags |= UI_TextActionFlag_DeltaPicksSelectionSide;
      }
      break;
      case OS_Key_Right:
      {
        result.delta.x = 1;
        result.flags |= UI_TextActionFlag_DeltaPicksSelectionSide;
      }
      break;
      case OS_Key_Up:
      {
        result.delta.y = -1;
      }
      break;
      case OS_Key_Down:
      {
        result.delta.y = 1;
      }
      break;
      case OS_Key_Home:
      {
        result.delta.x = S64_MIN;
      }
      break;
      case OS_Key_End:
      {
        result.delta.x = S64_MAX;
      }
      break;
      case OS_Key_A:
      {
        if(event->key_modifiers & OS_KeyModifier_Ctrl)
        {
          result.flags |= UI_TextActionFlag_SelectAll;
        }
      }
      break;
      case OS_Key_C:
      {
        if(event->key_modifiers & OS_KeyModifier_Ctrl)
        {
          result.flags |= UI_TextActionFlag_Copy;
        }
      }
      break;
      case OS_Key_X:
      {
        if(event->key_modifiers & OS_KeyModifier_Ctrl)
        {
          result.flags |= UI_TextActionFlag_Delete | UI_TextActionFlag_Copy;
        }
      }
      break;
      case OS_Key_V:
      {
        result.flags |= UI_TextActionFlag_Paste;
      }
      break;
      case OS_Key_Backspace:
      {
        result.delta.x = -1;
        result.flags |= UI_TextActionFlag_Delete | UI_TextActionFlag_ZeroDeltaWithSelection;
      }
      break;
      case OS_Key_Delete:
      {
        result.delta.x = +1;
        result.flags |= UI_TextActionFlag_Delete | UI_TextActionFlag_ZeroDeltaWithSelection;
      }
      break;
      default:
        break;
    }
  }
  else if(event->kind == OS_EventKind_Char)
  {
    result.codepoint = event->cp;
  }
  return result;
}

function UI_TextActionList
ui_text_action_list_from_events(Arena *arena, OS_EventList *event_list)
{
  UI_TextActionList result = {};
  for(OS_EventNode *n = event_list->first; n != 0; n = n->next)
  {
    OS_Event *event = &n->v;
    UI_TextAction text_action = ui_text_action_from_event(event);
    UI_TextAction text_action_zero = {};

    if(!memory_match(&text_action, &text_action_zero, sizeof(UI_TextAction)))
    {
      dll_remove(event_list->first, event_list->last, n);
      UI_TextActionNode *node = push_array<UI_TextActionNode>(arena, 1);
      node->action = text_action;
      dll_push_back(result.first, result.last, node);
    }
  }
  return result;
}

function UI_TextOp
ui_text_of_from_state_and_action(Arena *arena, String8 edit_str, UI_TextEditState *state, UI_TextAction *action)
{
  UI_TextOp result = {};

  result.new_cursor = state->cursor;
  result.new_mark = state->mark;

  S64 delta = action->delta.x;

  if((action->flags & UI_TextActionFlag_ZeroDeltaWithSelection) &&
     (state->mark != state->cursor))
  {
    delta = 0;
  }

  if(action->flags & UI_TextActionFlag_DeltaPicksSelectionSide &&
     !(action->flags & UI_TextActionFlag_KeepMark) &&
     state->cursor != state->mark)
  {
    if(delta < 0)
    {
      result.new_cursor = min(state->cursor, state->mark);
    }
    else if(delta > 0)
    {
      result.new_cursor = min(state->cursor, state->mark);
    }
  }
  else if(action->flags & UI_TextActionFlag_WordScan)
  {
    // TODO(simon): Implement Unicode algorithm for finding word boundaries.
    // (https://www.unicode.org/reports/tr29/#Word_Boundaries) This should
    // probably end up in `base_string`.
    // TODO(hampus): Collapse these two ifs and make it more robust.
    // This is just to get it started.
    if(delta < 0)
    {
      S64 after_word_cursor = 0;
      for(S64 i = result.new_cursor - 1; i > 0; --i)
      {
        U8 character = edit_str.data[i];
        U8 next_character = edit_str.data[i - 1];
        if(character != ' ' && next_character == ' ')
        {
          after_word_cursor = i;
          break;
        }
      }
      result.new_cursor = after_word_cursor;
    }
    else if(delta > 0)
    {
      S64 after_word_cursor = (S64)edit_str.size;
      for(S64 i = result.new_cursor; i < (S64)edit_str.size - 1; ++i)
      {
        U8 character = edit_str.data[i];
        U8 next_character = edit_str.data[i + 1];
        if(character != ' ' && next_character == ' ')
        {
          after_word_cursor = i + 1;
          break;
        }
      }
      result.new_cursor = after_word_cursor;
    }
  }
  else
  {
    if(delta == S64_MIN)
    {
      result.new_cursor = 0;
    }
    else if(delta == S64_MAX)
    {
      result.new_cursor = (S64)edit_str.size + 1;
    }
    else if(delta < 0)
    {
      for(S64 i = 0; i < -delta; ++i)
      {
        U8 *ptr = &edit_str.data[result.new_cursor];
        if(ptr > edit_str.data)
        {
          --ptr;
          while(ptr > edit_str.data && 0x80 <= *ptr && *ptr <= 0xBF)
          {
            --ptr;
          }
        }

        result.new_cursor = (S64)(ptr - edit_str.data);
      }
    }
    else if(delta > 0)
    {
      for(S64 i = 0; i < delta; ++i)
      {
        U8 *ptr = &edit_str.data[result.new_cursor];
        U8 *opl = &edit_str.data[edit_str.size];
        if(ptr < opl)
        {
          ++ptr;
          while(ptr < opl && 0x80 <= *ptr && *ptr <= 0xBF)
          {
            ++ptr;
          }
        }

        result.new_cursor = (S64)(ptr - edit_str.data);
      }
    }
  }

  if(action->flags & UI_TextActionFlag_Copy)
  {
    U64 min = (U64)min(result.new_cursor, result.new_mark);
    U64 max = (U64)max(result.new_cursor, result.new_mark);
    result.copy_string = str8_substr8(edit_str, min, max - min);
  }

  if(action->flags & UI_TextActionFlag_Delete)
  {
    result.range = v2s64(result.new_cursor, result.new_mark);
    result.new_cursor = min(result.new_cursor, result.new_mark);
  }

  if(!(action->flags & UI_TextActionFlag_KeepMark))
  {
    if(delta != 0 || (action->flags & UI_TextActionFlag_Delete))
    {
      result.new_mark = result.new_cursor;
    }
  }

  if(action->codepoint)
  {
    // NOTE(simon): Allocate enough space for encoding the longest Unicode
    // codepoint in UTF-8.
    result.replace_string.data = push_array<U8>(arena, 4);
    result.replace_string.size =
    string_encode_utf8(result.replace_string.data, action->codepoint);
    result.range = v2s64(result.new_cursor, result.new_mark);
    if(state->cursor > state->mark)
    {
      result.new_cursor = result.new_mark + (S64)result.replace_string.size;
    }
    else
    {
      result.new_cursor += (S64)result.replace_string.size;
    }
    result.new_mark = result.new_cursor;
  }

  if(action->flags & UI_TextActionFlag_Paste)
  {
    result.replace_string = os_push_clipboard(arena);
    result.range = v2s64(result.new_cursor, result.new_mark);
    if(result.new_cursor > result.new_mark)
    {
      result.new_cursor = result.new_mark + (S64)result.replace_string.size;
    }
    else
    {
      result.new_cursor += (S64)result.replace_string.size;
    }
    result.new_mark = result.new_cursor;
  }

  if(action->flags & UI_TextActionFlag_SelectAll)
  {
    result.new_mark = 0;
    result.new_cursor = S64_MAX;
  }

  return result;
}

//////////////////////////////
// NOTE(hampus): Init

function void
ui_init(void)
{
  // hampus: Allocate state

  Arena *arena = arena_alloc();
  ui_state = push_array<UI_State>(arena, 1);
  ui_state->arena = arena;

  for(U64 i = 0; i < array_count(ui_state->frame_arenas); ++i)
  {
    ui_state->frame_arenas[i] = arena_alloc();
  }

  // hampus: Nil state boxes

  ui_state->parent_stack = 0;
  ui_state->first_free_box = (UI_FreeBox *)&ui_nil_box;

  // hampus: Init icon strings

  // ui_state->icon_font_tag.name = str8_lit("fontello.ttf");
  ui_state->icon_font_tag.data = fontello_ttf_data;

  for(U64 i = 0; i < array_count(ui_state->ui_icon_to_string_table); ++i)
  {
    U32 cp = (U32)(0xE801 + i);
    String32 string32 = {&cp, 1};
    ui_state->ui_icon_to_string_table[i] = str8_from_str32(ui_state->arena, string32);
  }
}

function void
ui_destroy(void)
{
  arena_free(ui_state->arena);

  for(U64 i = 0; i < array_count(ui_state->frame_arenas); ++i)
  {
    arena_free(ui_state->frame_arenas[i]);
  }
  ui_state = 0;
}

//////////////////////////////
// NOTE(hampus): Begin/End

function B32
ui_tree_contains_click(UI_Box *root, Vec2F32 mouse_pos)
{
  B32 result = r4f32_contains_2f32(root->fixed_rect, mouse_pos) && (root->flags & UI_BoxFlag_Clickable) != 0;

  if(!result)
  {
    for(UI_Box *child = root->first; !ui_box_is_nil(child); child = child->next)
    {
      result = ui_tree_contains_click(child, mouse_pos);
      if(result)
      {
        break;
      }
    }
  }

  return result;
}

function void
ui_begin_build(OS_Handle window, OS_EventList *os_events, F64 dt)
{
  profile_function_begin();

  ui_state->parent_stack = 0;
  ui_state->seed_stack = 0;
  ui_state->build_idx += 1;
  ui_state->prev_mouse_pos = ui_state->mouse_pos;
  ui_state->prev_hot_key = ui_state->hot_key;
  ui_state->prev_active_key = ui_state->active_key;
#define X(name_upper, name_lower, type) ui_state->name_lower##_stack = 0;
  stack_values
#undef X
  arena_clear(ui_frame_arena());

  ui_state->dt = dt;
  ui_state->os_window = window;
  ui_state->mouse_pos = os_mouse_pos(ui_state->os_window);
  ui_state->os_events = os_events;

  ui_state->ctx_menu_key = ui_state->next_ctx_menu_key;
  ui_state->ctx_menu_anchor_key = ui_state->next_ctx_menu_anchor_key;
  ui_state->anchor_offset = ui_state->next_anchor_offset;

  // hampus: Gather release/presses for active box

  B32 left_mouse_released = false;
  B32 left_mouse_pressed = false;
  B32 escape_key_pressed = false;
  B32 return_key_pressed = false;

  OS_EventNode *next_event = 0;
  for(OS_EventNode *n = os_events->first; n != 0; n = next_event)
  {
    next_event = n->next;
    OS_Event *event = &n->v;
    if(event->kind == OS_EventKind_KeyRelease)
    {
      if(event->key == OS_Key_MouseLeft)
      {
        left_mouse_released = true;
      }
    }
    else if(event->kind == OS_EventKind_KeyPress)
    {
      if(event->key == OS_Key_Return)
      {
        return_key_pressed = true;
      }
      else if(event->key == OS_Key_Escape)
      {
        escape_key_pressed = true;
      }
      else if(event->key == OS_Key_MouseLeft)
      {
        left_mouse_pressed = true;
      }
    }
    else if(event->kind == OS_EventKind_TapRelease)
    {
      left_mouse_released = true;
    }
    else if(event->kind == OS_EventKind_TapPress)
    {
      left_mouse_pressed = true;
    }
    else if(event->kind == OS_EventKind_TouchEnd)
    {
      ui_state->hot_key = ui_key_zero();
    }
    else if(event->kind == OS_EventKind_Scroll)
    {
      ui_state->active_key = ui_key_zero();
    }
  }

  // hampus: Prune old boxes

  for(U64 i = 0; i < array_count(ui_state->box_map); ++i)
  {
    UI_Box *box = ui_state->box_map[i];
    while(!ui_box_is_nil(box))
    {
      UI_Box *next = box->hash_next;
      if(box->last_build_touched_idx < (ui_state->build_idx - 1))
      {
        if(box == ui_state->box_map[i])
        {
          ui_state->box_map[i] = box->hash_next;
        }

        if(!ui_box_is_nil(box->hash_next))
        {
          box->hash_next->hash_prev = box->hash_prev;
        }

        if(!ui_box_is_nil(box->hash_prev))
        {
          box->hash_prev->hash_next = box->hash_next;
        }

        ui_box_free(box);
      }
      box = next;
    }
  }

  // hampus: Active/hot/focus box control

  UI_Box *active_box = ui_box_from_key(ui_state->active_key);

  if(left_mouse_released || ui_box_is_nil(active_box))
  {
    ui_state->active_key = ui_key_zero();
  }
#if 0
  if(ui_key_match(ui_state->active_key, ui_key_zero()))
  {
    ui_state->hot_key = ui_key_zero();
  }
#endif
  if(escape_key_pressed)
  {
    ui_state->focus_key = ui_key_zero();
  }

  if(!ui_box_is_nil(ui_state->ctx_menu_root))
  {
    if(ui_state->ctx_menu_key != ui_key_zero())
    {
      if(left_mouse_pressed)
      {
        B32 inside_ctx_menu = ui_tree_contains_click(ui_state->ctx_menu_root, ui_state->mouse_pos);

        if(!inside_ctx_menu)
        {
          ui_ctx_menu_close();
        }
      }

      if(escape_key_pressed)
      {
        ui_ctx_menu_close();
      }
    }
    else
    {
      if(escape_key_pressed)
      {
        ui_state->focus_key = ui_key_zero();
      }
    }
  }

  ui_push_parent(&ui_nil_box);

  // hampus: Push default stack values

  ui_push_rect_color(v4f32(0.1f, 0.1f, 0.1f, 1.0f));
  ui_push_border_color(v4f32(0.4f, 0.4f, 0.4f, 1.0f));
  ui_push_border_thickness(1);
  ui_push_corner_radius(v4f32(3, 3, 3, 3));
  ui_push_softness(1);
  ui_push_hover_cursor(OS_Cursor_Arrow);
  ui_push_slice((R_Tex2DSlice){.tex = ui_state->renderer->white_texture});
  ui_push_text_color(v4f32(0.9f, 0.9f, 0.9f, 1.0f));
  ui_push_fixed_pos(v2f32(0, 0));
  ui_push_child_layout_axis(Axis2_Y);
  ui_push_box_flags(0);
  F_Tag ui_font_tag = (F_Tag){.name = str8_lit("res/Roboto-Regular.ttf"), .data = roboto_regular_ttf_data};
  ui_push_font_tag(ui_font_tag);
  ui_push_font_size(15);
  ui_push_text_padding(v2f32(1.0f, 0.5f));
  ui_push_pref_width(ui_text_content(1));
  ui_push_pref_height(ui_text_content(1));
  ui_push_text_align(UI_TextAlign_Center);
  ui_push_alpha(1.0f);
  ui_push_fixed_size(v2f32(50, 50));
  ui_push_seed(12345);

  Vec2U64 window_client_area = os_client_area_from_window(window);
  RectF32 clip_rect = r4f32(0, 0, (F32)window_client_area.x, (F32)window_client_area.y);

  // hampus: Make roots

  ui_next_pref_width(ui_px(clip_rect.x1, 1));
  ui_next_pref_height(ui_px(clip_rect.y1, 1));
  ui_state->root = ui_box_make(UI_BoxFlag_AllowOverflowX | UI_BoxFlag_AllowOverflowY, str8_lit("Root"));

  ui_push_parent(ui_state->root);

#if 0
    ui_next_pref_width(ui_children_sum(1));
    ui_next_pref_height(ui_children_sum(1));
#endif
  ui_next_pref_width(ui_pct(1, 1));
  ui_next_pref_height(ui_pct(1, 1));
  ui_state->ctx_menu_root =
  ui_box_make(UI_BoxFlag_FixedPos, str8_lit("CtxMenuRoot"));

  ui_next_pref_width(ui_pct(1, 1));
  ui_next_pref_height(ui_pct(1, 1));
  ui_state->normal_root = ui_box_make(0, str8_lit("NormalRoot"));

  ui_push_parent(ui_state->normal_root);

  profile_function_end();
}

function void
ui_end_build(void)
{
  profile_function_begin();
  ui_pop_parent(); // pop normal root
  ui_pop_parent(); // pop root

  if(ui_state->ctx_menu_key != ui_key_zero())
  {
    Vec2F32 anchor_pos = {};
    if(ui_state->ctx_menu_anchor_key != ui_key_zero())
    {
      UI_Box *anchor = ui_box_from_key(ui_state->ctx_menu_anchor_key);
      if(!ui_box_is_nil(anchor))
      {
        anchor_pos = v2f32(anchor->fixed_rect.min.x, anchor->fixed_rect.max.y);
      }
      else
      {
        // TODO(hampus): This doesn't solve the problem
        // if the context menu doesn't have an anchor
        ui_ctx_menu_close();
      }
    }

    anchor_pos = anchor_pos + ui_state->anchor_offset;

    ui_state->ctx_menu_root->fixed_pos = anchor_pos;
  }

  UI_Box *hot_box = ui_box_from_key(ui_state->hot_key);
  if(!ui_box_is_nil(hot_box))
  {
    os_set_cursor(hot_box->hover_cursor);
  }
  else
  {
    os_set_cursor(OS_Cursor_Arrow);
  }

  ui_layout(ui_state->root);
  profile_function_end();
}

//////////////////////////////
// NOTE(hampus): Box implementation

function UI_Box *
ui_box_alloc(void)
{
  UI_Box *box = (UI_Box *)ui_state->first_free_box;
  if(!ui_box_is_nil(box))
  {
    ASAN_UNPOISON_MEMORY_REGION(box, sizeof(UI_Box));
    sll_stack_pop(ui_state->first_free_box);
    memory_zero_struct(box);
  }
  else
  {
    box = push_array<UI_Box>(ui_state->arena, 1);
  }
  return box;
}

function void
ui_box_free(UI_Box *box)
{
  UI_FreeBox *free_box = (UI_FreeBox *)box;
  free_box->next = ui_state->first_free_box;
  ui_state->first_free_box = free_box;
  ASAN_POISON_MEMORY_REGION(box, sizeof(UI_Box));
}

function B32
ui_box_is_hot(UI_Box *box)
{
  B32 result = box->key == ui_state->hot_key && ui_state->hot_key != ui_key_zero();
  return result;
}

function B32
ui_box_is_active(UI_Box *box)
{
  B32 result = box->key == ui_state->active_key && ui_state->active_key != ui_key_zero();
  return result;
}

function B32
ui_box_is_focus(UI_Box *box)
{
  B32 result = box->key == ui_state->focus_key && ui_state->focus_key != ui_key_zero();
  return result;
}

function void
ui_focus_box(UI_Box *box)
{
  ui_state->focus_key = box->key;
}

function B32
ui_box_is_nil(UI_Box *box)
{
  B32 result = box == 0 || box == &ui_nil_box;
  return result;
}

function UI_Box *
ui_box_from_key(UI_Key key)
{
  UI_Box *box = &ui_nil_box;
  U64 slot_idx = key.u64[0] % array_count(ui_state->box_map);
  if(key != ui_key_zero())
  {
    box = ui_state->box_map[slot_idx];
    while(!ui_box_is_nil(box))
    {
      if(box->key == key)
      {
        break;
      }
      box = box->hash_next;
    }

    if(ui_box_is_nil(box))
    {
      box = ui_box_alloc();
      sll_stack_push_n(ui_state->box_map[slot_idx], box, hash_next);
      box->key = key;
      box->first_build_touched_idx = ui_state->build_idx;
    }
  }
  else
  {
    box = push_array<UI_Box>(ui_frame_arena(), 1);
    box->key = key;
    box->first_build_touched_idx = ui_state->build_idx;
  }

  return box;
}

function UI_Box *
ui_box_makerom_key(UI_BoxFlags flags, UI_Key key)
{
  UI_Box *box = ui_box_from_key(key);
  ASSERT(box->last_build_touched_idx != ui_state->build_idx);
  box->last_build_touched_idx = ui_state->build_idx;

  // hampus: Insert into tree

  UI_Box *parent = ui_top_parent();
  box->first = box->last = box->next = box->prev = box->parent = &ui_nil_box;
  box->parent = parent;
  if(!ui_box_is_nil(parent))
  {
    dll_push_back_npz(&ui_nil_box, parent->first, parent->last, box, next, prev);
  }

  // hampus: Equip per build info

#define X(name_upper, name_lower, type) \
  box->name_lower = ui_state->name_lower##_stack->val;
  stack_values
#undef X

  box->flags = flags | box->box_flags;

  // hampus: Auto pop stacks

#define X(name_upper, name_lower, type)      \
  if(ui_state->name_lower##_stack->auto_pop) \
    ui_pop_##name_lower();
  stack_values
#undef X

  return box;
}

function String8
ui_get_hash_part_from_string(String8 string)
{
  String8 result = string;
  U64 idx = 0;
  if(str8_find_substr8(string, str8_lit("###"), &idx))
  {
    result = str8_skip(string, idx + 3);
  }
  return result;
}

function String8
ui_get_display_part_from_string(String8 string)
{
  String8 result = string;
  U64 idx = 0;
  if(str8_find_substr8(string, str8_lit("##"), &idx))
  {
    result = str8_chop(string, string.size - idx);
  }
  return result;
}

function UI_Box *
ui_box_make(UI_BoxFlags flags, String8 string)
{
  UI_Key key =
  ui_key_from_string(ui_top_seed(), ui_get_hash_part_from_string(string));
  UI_Box *box = ui_box_makerom_key(flags, key);
  box->string = string;
  return box;
}

function UI_Box *
ui_box_make(UI_BoxFlags flags, char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  String8 string = str8_push(ui_frame_arena(), fmt, args);
  UI_Box *box = ui_box_make(flags, string);
  va_end(args);
  return box;
}

function UI_Box *
ui_box_make(UI_BoxFlags flags, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  String8 string = str8_push(ui_frame_arena(), (char *)fmt, args);
  UI_Box *box = ui_box_make(flags, string);
  va_end(args);
  return box;
}

function UI_Box *
ui_box_make(UI_BoxFlags flags)
{
  UI_Box *box = ui_box_make(flags, str8_lit(""));
  return box;
}

function void
ui_box_equip_display_string(UI_Box *box, String8 string)
{
  String8 display_string = ui_get_display_part_from_string(string);
  box->string = str8_push(ui_frame_arena(), display_string);
}

function void
ui_box_equip_display_string(UI_Box *box, char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  String8 string = str8_push(ui_frame_arena(), fmt, args);
  ui_box_equip_display_string(box, string);
  va_end(args);
}

function void
ui_box_equip_custom_draw_proc(UI_Box *box,
                              UI_CustomDrawFunction *proc,
                              void *user_data)
{
  box->custom_draw = proc;
  box->custom_draw_user_data = user_data;
}

function B32
ui_point_is_inside_context_menu(Vec2F32 point)
{
  B32 result = false;
  for(UI_Box *ctx_menu_child = ui_state->ctx_menu_root->first; !ui_box_is_nil(ctx_menu_child); ctx_menu_child = ctx_menu_child->next)
  {
    result = r4f32_contains_2f32(ui_state->ctx_menu_root->fixed_rect, point);
    if(result)
    {
      break;
    }
  }
  return result;
}

function B32
ui_box_is_part_of_ctx_menu(UI_Box *box)
{
  B32 result = false;
  if(ui_state->ctx_menu_key != ui_key_zero())
  {
    UI_Box *ctx_menu_root = ui_state->ctx_menu_root;
    for(UI_Box *parent = box->parent; !ui_box_is_nil(parent); parent = parent->parent)
    {
      if(parent == ctx_menu_root)
      {
        result = true;
        break;
      }
    }
  }
  return result;
}

function B32
ui_point_is_inside_parent_tree(UI_Box *box, Vec2F32 point)
{
  B32 result = false;
  RectF32 hover_region = box->fixed_rect;
  for(UI_Box *parent = box->parent; !ui_box_is_nil(parent); parent = parent->parent)
  {
    if(parent->flags & UI_BoxFlag_Clip)
    {
      hover_region = r4f32_intersect_r4f32(hover_region, parent->fixed_rect);
    }
  }
  result = r4f32_contains_2f32(hover_region, point);
  return result;
}

function UI_Comm
ui_comm_from_box__touch(UI_Box *box)
{
  // TODO(hampus): Fling began inside and screen and exiting outside
  // should keep scrolling the box.

  UI_Comm result = {};
  ASSERT(box->key != ui_key_zero() && "Tried to gather input from a keyless box!");
  result.box = box;
  if(box->flags & UI_BoxFlag_Disabled)
  {
    return result;
  }

  result.fling_side[Axis2_X] = Side_COUNT;
  result.fling_side[Axis2_Y] = Side_COUNT;

  OS_EventList *event_list = ui_state->os_events;
  Vec2F32 mouse_pos = ui_state->mouse_pos;
  B32 part_of_ctx_menu = ui_box_is_part_of_ctx_menu(box);
  B32 inside_ctx_menu = ui_point_is_inside_context_menu(mouse_pos);
  B32 gather_input = (part_of_ctx_menu || (!inside_ctx_menu)) && ui_point_is_inside_parent_tree(box, mouse_pos);
  if(gather_input)
  {
    for(OS_EventNode *n = ui_state->os_events->first; n != 0; n = n->next)
    {
      OS_Event *event = &n->v;
      switch(event->kind)
      {
        case OS_EventKind_TapPress:
        {
          if(box->flags & UI_BoxFlag_Clickable)
          {
            result.pressed = true;
            ui_state->active_key = box->key;
            if(box->flags & UI_BoxFlag_Focus)
            {
              ui_state->focus_key = box->key;
            }
            dll_remove(event_list->first, event_list->last, n);
          }
        }
        break;
        case OS_EventKind_TapRelease:
        {
          if(box->flags & UI_BoxFlag_Clickable)
          {
            result.released = true;
            dll_remove(event_list->first, event_list->last, n);
            // NOTE(hampus): We are comparing to the prev_active_key because we will
            // zero the current active key in ui_begin_build() if there was a tap
            // release
            if(box->key == ui_state->prev_active_key || box->key == ui_state->active_key)
            {
              result.clicked = true;
              if(box->key == ui_state->active_key)
              {
                box->active_state = 1;
                ui_state->active_key = ui_key_zero();
              }
            }
          }
        }
        break;
        case OS_EventKind_TouchBegin:
        {
          if(box->flags & UI_BoxFlag_ViewScroll)
          {
            ui_state->hot_key = box->key;
            dll_remove(event_list->first, event_list->last, n);
          }
        }
        break;
        case OS_EventKind_TouchEnd:
        {
        }
        break;
        case OS_EventKind_DoubleTap:
        {
          if(box->flags & UI_BoxFlag_Clickable)
          {
            result.double_clicked = true;
            dll_remove(event_list->first, event_list->last, n);
          }
        }
        break;

        case OS_EventKind_Scroll:
        {
          if(box->flags & UI_BoxFlag_ViewScroll)
          {
            result.scroll.x += event->scroll.x;
            result.scroll.y += event->scroll.y;
            dll_remove(event_list->first, event_list->last, n);
          }
        }
        break;
        case OS_EventKind_FlingX:
        {
          if(box->flags & UI_BoxFlag_FlingX)
          {
            result.fling_side[Axis2_X] = event->fling_side;
            dll_remove(event_list->first, event_list->last, n);
          }
        }
        break;

        case OS_EventKind_FlingY:
        {
          if(box->flags & UI_BoxFlag_FlingY)
          {
            result.fling_side[Axis2_Y] = event->fling_side;
            dll_remove(event_list->first, event_list->last, n);
          }
        }
        break;
        default:
          break;
      }
    }
  }

  return result;
}

function UI_Comm
ui_comm_from_box__mouse(UI_Box *box)
{
  UI_Comm result = {};
  result.box = box;
  if(box->flags & UI_BoxFlag_Disabled)
  {
    return result;
  }

  ASSERT(box->key != ui_key_zero() && "Tried to gather input from a keyless box!");

  Vec2F32 mouse_pos = ui_state->mouse_pos;

  result.rel_mouse = mouse_pos - box->fixed_rect.min;

  B32 part_of_ctx_menu = ui_box_is_part_of_ctx_menu(box);
  B32 inside_ctx_menu = ui_point_is_inside_context_menu(mouse_pos);

  B32 gather_input = (part_of_ctx_menu || !inside_ctx_menu);
  if(gather_input)
  {
    if(ui_box_is_active(box))
    {
      result.dragging = true;
      result.drag_delta = ui_state->prev_mouse_pos - mouse_pos;
      ui_state->hot_key = box->key;
    }

    B32 mouse_over = ui_point_is_inside_parent_tree(box, mouse_pos);
    if(mouse_over || ui_box_is_active(box) || ui_box_is_focus(box))
    {
      if(mouse_over)
      {
        if(ui_state->hot_key == ui_key_zero())
        {
          ui_state->hot_key = box->key;
        }
      }

      if(ui_box_is_hot(box))
      {
        result.hovering = true;
      }

      OS_EventList *event_list = ui_state->os_events;
      for(OS_EventNode *node = event_list->first; node != 0; node = node->next)
      {
        OS_Event *event = &node->v;
        switch(event->kind)
        {
          case OS_EventKind_KeyRelease:
          {
            if(event->key == OS_Key_MouseLeft)
            {
              result.released = true;
              dll_remove(event_list->first, event_list->last, node);
              if(box->key == ui_state->prev_active_key)
              {
                result.clicked = true;
              }
              if(ui_box_is_hot(box))
              {
                ui_state->active_key = ui_key_zero();
              }
            }
            else if(event->key == OS_Key_MouseRight)
            {
              result.right_released = true;
              dll_remove(event_list->first, event_list->last, node);
            }
          }
          break;
          case OS_EventKind_KeyDoublePress:
          {
            if(event->key == OS_Key_MouseLeft)
            {
              if(box->flags & UI_BoxFlag_Clickable)
              {
                result.pressed = true;
                ui_state->active_key = box->key;
                if(box->flags & UI_BoxFlag_Focus)
                {
                  ui_state->focus_key = box->key;
                }
                dll_remove(event_list->first, event_list->last, node);
              }
            }
          }
          break;

          case OS_EventKind_KeyPress:
          {
            if(event->key == OS_Key_MouseLeft)
            {
              if(box->flags & UI_BoxFlag_Clickable)
              {
                result.pressed = true;
                ui_state->active_key = box->key;
                if(box->flags & UI_BoxFlag_Focus)
                {
                  ui_state->focus_key = box->key;
                }
                dll_remove(event_list->first, event_list->last, node);
              }
            }
            else if(event->key == OS_Key_MouseRight)
            {
              if(box->flags & UI_BoxFlag_Clickable)
              {
                if(box->flags & UI_BoxFlag_Focus)
                {
                  ui_state->focus_key = box->key;
                }
                dll_remove(event_list->first, event_list->last, node);
              }
            }
            else if(event->key == OS_Key_Return)
            {
              if(ui_box_is_focus(box) && (box->flags & UI_BoxFlag_Commitable))
              {
                result.commit = true;
                dll_remove(event_list->first, event_list->last, node);
              }
            }
          }
          break;
          case OS_EventKind_Scroll:
          {
            if(box->flags & UI_BoxFlag_ViewScroll)
            {
              result.scroll.x += -node->v.scroll.x;
              result.scroll.y += -node->v.scroll.y;
              dll_remove(event_list->first, event_list->last, node);
            }
          }
          break;
          default:
            break;
        }
      }
    }
  }

  return result;
}

//////////////////////////////
// NOTE(hampus): Layouting

function UI_Size
ui_size_make(UI_SizeKind kind, F32 val, F32 strictness)
{
  UI_Size size = {};
  size.kind = kind;
  size.value = val;
  size.strictness = strictness;
  return (size);
}

function void
ui_solve_independent_sizes(UI_Box *root, Axis2 axis)
{
  if(!(root->flags & (UI_BoxFlag_FixedWidth << axis)))
  {
    UI_Size size = ui_size_from_axis(root, axis);
    switch(size.kind)
    {
      default:
      case UI_SizeKind_TextContent:
      {
        Vec2F32 text_dim = {};
        if(axis == Axis2_X)
        {
          F32 advance = f_advance_from_tag_size_string(
          root->font_tag, root->font_size, root->string);
          root->fixed_size[Axis2_X] = floor_f32((F32)advance + root->text_padding[Axis2_X] * (F32)root->font_size);
        }
        else if(axis == Axis2_Y)
        {
          F32 height = f_line_height_from_tag_size(root->font_tag, root->font_size);
          root->fixed_size[Axis2_Y] = floor_f32(height);
        }
      }
      break;
      case UI_SizeKind_Pixels:
      {
        root->fixed_size[axis] = floor_f32(size.value);
      }
      break;
    }
  }
  for(UI_Box *child = root->first; !ui_box_is_nil(child);
      child = child->next)
  {
    ui_solve_independent_sizes(child, axis);
  }
}

function void
ui_solve_upward_dependent_sizes(UI_Box *root, Axis2 axis)
{
  if(!(root->flags & (UI_BoxFlag_FixedWidth << axis)))
  {
    UI_Size size = ui_size_from_axis(root, axis);
    if(size.kind == UI_SizeKind_Pct)
    {
      ASSERT(root->parent && "Percent of parent without a parent");
      ASSERT(size.kind != UI_SizeKind_ChildrenSum && "Cyclic sizing behaviour");
      F32 parent_size = root->parent->fixed_size[axis];
      root->fixed_size[axis] = floor_f32(parent_size * size.value);
    }
  }
  for(UI_Box *child = root->first; !ui_box_is_nil(child); child = child->next)
  {
    ui_solve_upward_dependent_sizes(child, axis);
  }
}

function void
ui_solve_downward_dependent_sizes(UI_Box *root, Axis2 axis)
{
  for(UI_Box *child = root->first; !ui_box_is_nil(child);
      child = child->next)
  {
    ui_solve_downward_dependent_sizes(child, axis);
  }
  if(!(root->flags & (UI_BoxFlag_FixedWidth << axis)))
  {
    UI_Size size = ui_size_from_axis(root, axis);
    if(size.kind == UI_SizeKind_ChildrenSum)
    {
      F32 children_total_size = 0;
      Axis2 child_layout_axis = root->child_layout_axis;
      for(UI_Box *child = root->first; !ui_box_is_nil(child); child = child->next)
      {
        if(!(child->flags & (UI_BoxFlag_FixedX << axis)))
        {
          F32 child_size = child->fixed_size[axis];
          if(axis == child_layout_axis)
          {
            children_total_size += child_size;
          }
          else
          {
            children_total_size = max(child_size, children_total_size);
          }
        }
      }
      root->fixed_size[axis] = floor_f32(children_total_size);
    }
  }
}

function void
ui_solve_other_axis_dependent_sizes(UI_Box *root, Axis2 axis)
{
  if(!(root->flags & (UI_BoxFlag_FixedWidth << axis)))
  {
    UI_Size size = ui_size_from_axis(root, axis);
    if(size.kind == UI_SizeKind_OtherAxis)
    {
      root->fixed_size[axis] = root->fixed_size[axis_flip(axis)];
    }
  }
  for(UI_Box *child = root->first; !ui_box_is_nil(child);
      child = child->next)
  {
    ui_solve_other_axis_dependent_sizes(child, axis);
  }
}

function void
ui_solve_size_violations(UI_Box *root, Axis2 axis)
{
  F32 available_space = root->fixed_size[axis];
  F32 taken_space = 0;
  F32 total_fixup_budget = 0;
  if(!(root->flags & (UI_BoxFlag_AllowOverflowX << axis)))
  {
    for(UI_Box *child = root->first; !ui_box_is_nil(child);
        child = child->next)
    {
      if(!(child->flags & (UI_BoxFlag_FixedX << axis)))
      {
        if(axis == root->child_layout_axis)
        {
          taken_space += child->fixed_size[axis];
        }
        else
        {
          taken_space = max(taken_space, child->fixed_size[axis]);
        }
        F32 fixup_budget_this_child =
        child->fixed_size[axis] *
        (1 - ui_size_from_axis(child, axis).strictness);
        total_fixup_budget += fixup_budget_this_child;
      }
      else
      {
        UI_Size size = ui_size_from_axis(child, axis);
        if(size.kind == UI_SizeKind_Pct)
        {
          child->fixed_size[axis] =
          floor_f32(size.value * root->fixed_size[axis]);
        }
      }
    }
  }

  if(!(root->flags & (UI_BoxFlag_AllowOverflowX << axis)))
  {
    F32 violation = taken_space - available_space;
    if(violation > 0 && total_fixup_budget > 0)
    {
      for(UI_Box *child = root->first; !ui_box_is_nil(child);
          child = child->next)
      {
        if(!(child->flags & (UI_BoxFlag_FixedX << axis)))
        {
          UI_Size size = ui_size_from_axis(child, axis);
          F32 fixup_budget_this_child =
          child->fixed_size[axis] * (1 - size.strictness);
          F32 fixup_size_this_child = 0;
          if(axis == root->child_layout_axis)
          {
            fixup_size_this_child = fixup_budget_this_child * (violation / total_fixup_budget);
          }
          else
          {
            fixup_size_this_child = child->fixed_size[axis] - available_space;
          }
          fixup_size_this_child =
          clamp(0, fixup_size_this_child, fixup_budget_this_child);
          child->fixed_size[axis] -= fixup_size_this_child;
          child->fixed_size[axis] = floor_f32(child->fixed_size[axis]);
          for(UI_Box *child_child = child->first; !ui_box_is_nil(child_child); child_child = child_child->next)
          {
            UI_Size child_child_size = ui_size_from_axis(child_child, axis);
            if(child_child_size.kind == UI_SizeKind_Pct)
            {
              child_child->fixed_size[axis] =
              floor_f32(child->fixed_size[axis] * child_child_size.value);
            }
          }
        }
      }
    }
  }

  for(UI_Box *child = root->first; !ui_box_is_nil(child);
      child = child->next)
  {
    ui_solve_size_violations(child, axis);
  }
}

function void
ui_calculate_final_rect(UI_Box *root, Axis2 axis)
{
  if(root->first_build_touched_idx == root->last_build_touched_idx)
  {
    root->fixed_pos_animated[axis] = root->fixed_pos[axis];
    root->fixed_size_animated[axis] = root->fixed_size[axis];
    root->scroll_animated[axis] = root->scroll[axis];
  }
  else
  {
    F32 animation_delta = (F32)(1.0 - pow_f64(2.0, -30 * ui_state->dt));

    if(abs(root->fixed_pos_animated[axis] - root->fixed_pos[axis]) <= 0.5f)
    {
      root->fixed_pos_animated[axis] = root->fixed_pos[axis];
    }
    else
    {
      root->fixed_pos_animated[axis] += (F32)(root->fixed_pos[axis] - root->fixed_pos_animated[axis]) * animation_delta;
    }

    if(abs(root->fixed_size_animated[axis] - root->fixed_size[axis]) <= 0.5f)
    {
      root->fixed_size_animated[axis] = root->fixed_size[axis];
    }
    else
    {
      root->fixed_size_animated[axis] += (root->fixed_size[axis] - root->fixed_size_animated[axis]) * animation_delta;
    }

    if(abs(root->scroll_animated[axis] - root->scroll[axis]) <= 0.5f)
    {
      root->scroll_animated[axis] = root->scroll[axis];
    }
    else
    {
      root->scroll_animated[axis] += (root->scroll[axis] - root->scroll_animated[axis]) * animation_delta;
    }
  }

  if(root->flags & (UI_BoxFlag_AnimateX << axis))
  {
    root->fixed_rect.min[axis] = root->fixed_pos_animated[axis];
  }
  else
  {
    root->fixed_rect.min[axis] = root->fixed_pos[axis];
  }

  if(root->flags & (UI_BoxFlag_AnimateWidth << axis))
  {
    root->fixed_rect.max[axis] = root->fixed_rect.min[axis] + root->fixed_size_animated[axis];
  }
  else
  {
    root->fixed_rect.max[axis] = root->fixed_rect.min[axis] + root->fixed_size[axis];
  }

  root->fixed_rect.min[axis] = floor_f32(root->fixed_rect.min[axis]);
  root->fixed_rect.max[axis] = floor_f32(root->fixed_rect.max[axis]);

  F32 scroll = 0;
  if(root->flags & (UI_BoxFlag_AnimateScroll << axis))
  {
    scroll -= root->scroll_animated[axis];
  }
  else
  {
    scroll -= root->scroll[axis];
  }

  UI_Size size = ui_size_from_axis(root, axis);

  F32 child_offset = root->fixed_rect.min[axis] + scroll;
  F32 next_rel_child_pos = 0.0f;
  for(UI_Box *child = root->first; !ui_box_is_nil(child); child = child->next)
  {
    if(!(child->flags & (UI_BoxFlag_FixedX << axis)))
    {
      child->fixed_pos[axis] = child_offset + next_rel_child_pos;
      if(axis == root->child_layout_axis)
      {
        next_rel_child_pos += child->fixed_size[axis];
      }
    }
    ui_calculate_final_rect(child, axis);
  }
}

function void
ui_calculate_sizes(UI_Box *root)
{
  for_each_enum_val(Axis2, axis)
  {
    ui_solve_independent_sizes(root, axis);
  }
  for_each_enum_val(Axis2, axis)
  {
    ui_solve_upward_dependent_sizes(root, axis);
  }
  for_each_enum_val(Axis2, axis)
  {
    ui_solve_downward_dependent_sizes(root, axis);
  }
  for_each_enum_val(Axis2, axis)
  {
    ui_solve_other_axis_dependent_sizes(root, axis);
  }
}

function void
ui_layout(UI_Box *root)
{
  ui_calculate_sizes(root);
  for_each_enum_val(Axis2, axis)
  {
    ui_solve_size_violations(root, axis);
  }
  for_each_enum_val(Axis2, axis)
  {
    ui_calculate_final_rect(root, axis);
  }
}

function UI_Size
ui_dp(F32 val, F32 strictness)
{
  UI_Size result = {};
  result.kind = UI_SizeKind_Pixels;
  result.value = val * os_window_dpi(ui_state->os_window).x / 160;
  result.strictness = strictness;
  return result;
}

function UI_Size
ui_px(F32 val, F32 strictness)
{
  UI_Size result = {};
  result.kind = UI_SizeKind_Pixels;
  result.value = val;
  result.strictness = strictness;
  return result;
}

function UI_Size
ui_text_content(F32 strictness)
{
  UI_Size result = {};
  result.kind = UI_SizeKind_TextContent;
  result.strictness = strictness;
  return result;
}

function UI_Size
ui_pct(F32 val, F32 strictness)
{
  UI_Size result = {};
  result.kind = UI_SizeKind_Pct;
  result.value = val;
  result.strictness = strictness;
  return result;
}

function UI_Size
ui_children_sum(F32 strictness)
{
  UI_Size result = {};
  result.kind = UI_SizeKind_ChildrenSum;
  result.strictness = strictness;
  return result;
}

function UI_Size
ui_other_axis(F32 strictness)
{
  UI_Size result = {};
  result.kind = UI_SizeKind_OtherAxis;
  result.strictness = strictness;
  return result;
}

function UI_Size
ui_fill(void)
{
  UI_Size result = {};
  result.kind = UI_SizeKind_Pct;
  result.value = 1;
  result.strictness = 0;
  return result;
}

function UI_Size
ui_em(F32 val, F32 strictness)
{
  UI_Size result = {};
  result.kind = UI_SizeKind_Pixels;
  result.value = ui_top_font_line_height() * val;
  result.strictness = strictness;
  return result;
}

function void
ui_push_parent(UI_Box *box)
{
  UI_ParentStackNode *n = push_array<UI_ParentStackNode>(ui_frame_arena(), 1);
  n->box = box;
  sll_stack_push(ui_state->parent_stack, n);
}

function UI_Box *
ui_pop_parent(void)
{
  UI_Box *box = ui_top_parent();
  sll_stack_pop(ui_state->parent_stack);
  return box;
}

function UI_Box *
ui_top_parent(void)
{
  UI_Box *box = ui_state->parent_stack->box;
  return box;
}

function void
ui_push_seed(U64 seed)
{
  UI_SeedNode *node = push_array<UI_SeedNode>(ui_frame_arena(), 1);
  node->seed = seed;
  sll_stack_push(ui_state->seed_stack, node);
}

function void
ui_pop_seed(void)
{
  sll_stack_pop(ui_state->seed_stack);
}

function U64
ui_top_seed(void)
{
  U64 seed = ui_state->seed_stack->seed;
  return seed;
}

function F32
ui_top_font_line_height(void)
{
  F32 line_height = 0;
  F_Tag tag = ui_top_font_tag();
  U32 font_size = ui_top_font_size();
  line_height = f_line_height_from_tag_size(tag, font_size);
  return line_height;
}

function void
ui_push_pref_size(Axis2 axis, UI_Size size)
{
  if(axis == Axis2_X)
  {
    ui_push_pref_width(size);
  }
  else
  {
    ui_push_pref_height(size);
  }
}

function void
ui_next_pref_size(Axis2 axis, UI_Size size)
{
  if(axis == Axis2_X)
  {
    ui_next_pref_width(size);
  }
  else
  {
    ui_next_pref_height(size);
  }
}

function void
ui_pop_pref_size(Axis2 axis)
{
  if(axis == Axis2_X)
  {
    ui_pop_pref_width();
  }
  else
  {
    ui_pop_pref_height();
  }
}

function void
ui_next_rect_color(Vec4F32 color)
{
  ui_next_rect_color00(color);
  ui_next_rect_color01(color);
  ui_next_rect_color10(color);
  ui_next_rect_color11(color);
}

function void
ui_push_rect_color(Vec4F32 color)
{
  ui_push_rect_color00(color);
  ui_push_rect_color01(color);
  ui_push_rect_color10(color);
  ui_push_rect_color11(color);
}

function void
ui_pop_rect_color(void)
{
  ui_pop_rect_color00();
  ui_pop_rect_color01();
  ui_pop_rect_color10();
  ui_pop_rect_color11();
}

function void
ui_next_fixed_rect(RectF32 rect)
{
  ui_next_fixed_pos(rect.min);
  ui_next_fixed_size(rect.max - rect.min);
}

function void
ui_push_fixed_rect(RectF32 rect)
{
  ui_push_fixed_pos(rect.min);
  ui_push_fixed_size(rect.max - rect.min);
}

function void
ui_pop_fixed_rect(void)
{
  ui_pop_fixed_pos();
  ui_pop_fixed_size();
}

#define X(name_upper, name_lower, type)                                                   \
  function ui_##name_upper##Node *                                                        \
  ui_push_##name_lower(type new_val)                                                      \
  {                                                                                       \
    ui_##name_upper##Node *node = push_array<ui_##name_upper##Node>(ui_frame_arena(), 1); \
    node->val = new_val;                                                                  \
    sll_stack_push(ui_state->name_lower##_stack, node);                                   \
    return ui_state->name_lower##_stack;                                                  \
  }                                                                                       \
                                                                                          \
  function ui_##name_upper##Node *                                                        \
  ui_next_##name_lower(type val)                                                          \
  {                                                                                       \
    ui_##name_upper##Node *node = ui_push_##name_lower(val);                              \
    node->auto_pop = true;                                                                \
    return ui_state->name_lower##_stack;                                                  \
  }                                                                                       \
                                                                                          \
  function ui_##name_upper##Node *                                                        \
  ui_pop_##name_lower(void)                                                               \
  {                                                                                       \
    sll_stack_pop(ui_state->name_lower##_stack);                                          \
    return ui_state->name_lower##_stack;                                                  \
  }                                                                                       \
                                                                                          \
  function type                                                                           \
  ui_top_##name_lower(void)                                                               \
  {                                                                                       \
    return ui_state->name_lower##_stack->val;                                             \
  }

stack_values
#undef X