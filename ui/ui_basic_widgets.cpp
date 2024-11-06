static void
ui_spacer(UI_Size size)
{
  Axis2 axis = ui_top_parent()->child_layout_axis;
  ui_next_pref_size(axis, size);
  ui_next_pref_size(axis_flip(axis), ui_px(0, 0));
  ui_box_make(0);
}

static void
ui_text(String8 string)
{
  UI_Box *box = ui_box_make(UI_BoxFlag_DrawText);
  ui_box_equip_display_string(box, string);
}

static void
ui_text(char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  String8 string = str8_push(ui_frame_arena(), fmt, args);
  ui_text(string);
  va_end(args);
}

static void
ui_text(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  String8 string = str8_push(ui_frame_arena(), (char *)fmt, args);
  ui_text(string);
  va_end(args);
}

static void
ui_simple_text(String8 string)
{
  UI_Box *box = ui_box_make(UI_BoxFlag_DrawText | UI_BoxFlag_SimpleText);
  ui_box_equip_display_string(box, string);
}

static void
ui_simple_text(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  String8 string = str8_push(ui_frame_arena(), (char *)fmt, args);
  ui_simple_text(string);
  va_end(args);
}

static UI_Comm
ui_button(String8 string)
{
  ui_next_hover_cursor(OS_Cursor_Hand);
  UI_Box *box = ui_box_make(UI_BoxFlag_DrawBackground |
                            UI_BoxFlag_DrawText |
                            UI_BoxFlag_DrawBorder |
                            UI_BoxFlag_HotAnimation |
                            UI_BoxFlag_Clickable |
                            UI_BoxFlag_ActiveAnimation,
                            string);
  ui_box_equip_display_string(box, string);
  UI_Comm comm = ui_comm_from_box(box);
  return (comm);
}

static UI_Comm
ui_button(char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  String8 string = str8_push(ui_frame_arena(), fmt, args);
  UI_Comm comm = ui_button(string);
  va_end(args);
  return (comm);
}

static UI_Comm
ui_check(B32 b32, String8 string)
{
  UI_Box *box = ui_box_make(UI_BoxFlag_DrawBackground |
                            UI_BoxFlag_DrawBorder |
                            UI_BoxFlag_HotAnimation |
                            UI_BoxFlag_Clickable |
                            UI_BoxFlag_ActiveAnimation,
                            string);
  UI_Comm comm = ui_comm_from_box(box);
  ui_parent(box)
  {
    ui_next_pref_width(ui_pct(1, 1));
    ui_next_pref_height(ui_pct(1, 1));
    UI_Box *check_box = ui_box_make(0);
    if(b32)
    {
      check_box->flags |= UI_BoxFlag_DrawText;
    }
    ui_box_equip_display_string(check_box, ui_str8_from_icon(UI_Icon_Check));
  }
  return (comm);
}

static UI_Comm
ui_check(B32 b32, char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  String8 string = str8_push(ui_frame_arena(), fmt, args);
  UI_Comm comm = ui_check(b32, string);
  va_end(args);
  return (comm);
}

static String8
ui_push_replace_string(Arena *arena, String8 edit_str, Vec2S64 range, Array<U8> buffer, String8 replace_str)
{
  U64 min_range = (U64)(range.min);
  U64 max_range = (U64)(range.max);
  min_range = Min(min_range, edit_str.size);
  max_range = Min(max_range, edit_str.size);
  if(min_range > max_range)
  {
    U64 temp = min_range;
    min_range = max_range;
    max_range = temp;
  }
  U64 replace_range_length = max_range - min_range;
  String8 new_buffer = {};
  U64 new_buffer_size = edit_str.size - replace_range_length + replace_str.size;
  new_buffer.data = push_array<U8>(arena, new_buffer_size);
  new_buffer.size = new_buffer_size;
  String8 before_range = str8_prefix(edit_str, min_range);
  String8 after_range = str8_skip(edit_str, max_range);
  if(before_range.size != 0)
  {
    MemoryCopy(new_buffer.data, before_range.data, before_range.size);
  }
  if(replace_str.size != 0)
  {
    MemoryCopy(new_buffer.data + min_range, replace_str.data, replace_str.size);
  }
  if(after_range.size != 0)
  {
    MemoryCopy(new_buffer.data + min_range + replace_str.size, after_range.data, after_range.size);
  }
  new_buffer.size = Min(new_buffer.size, array_count(buffer));
  return (new_buffer);
}

static S64
ui_codepoint_index_from_mouse_pos(UI_Box *box, String8 edit_str)
{
  TempArena scratch = GetScratch(0, 0);
  S64 result = S64_MAX;
  Vec2F32 mouse_pos = ui_state->mouse_pos;
  if(mouse_pos.x < box->fixed_rect.x0)
  {
    result = S64_MIN;
  }

  UI_Size spacing = ui_em(0.3f, 1);
  F32 x = -box->scroll.x + box->fixed_rect.x0 + spacing.value;
  for(U64 i = 0; i < edit_str.size;)
  {
    StringDecode decode = string_decode_utf8(edit_str.data + i, edit_str.size - i);
    Vec2F32 dim = {};
    dim.x = f_get_advance(ui_top_font_tag(), ui_top_font_size(), decode.codepoint);
    dim.y = f_line_height_from_tag_size(ui_top_font_tag(), ui_top_font_size());
    RectF32 character_rect = box->fixed_rect;
    character_rect.min.x = x;
    character_rect.max.x = character_rect.min.x + dim.x;
    if(mouse_pos.x >= character_rect.x0 && mouse_pos.x < character_rect.x1)
    {
      result = (S64)i;
      break;
    }

    x += dim.x;
    i += decode.size;
  }
  return result;
}

static UI_Comm
ui_line_edit(UI_TextEditState *edit_state, Array<U8> buffer, U64 *string_length, String8 string)
{
  UI_Comm comm = {};
  TempArena scratch = GetScratch(0, 0);
  ui_seed(ui_hash_from_seed_string(ui_top_seed(), string))
  {
    String8 buffer_str8 = str8(buffer.val, array_count(buffer));
    ui_next_child_layout_axis(Axis2_X);
    ui_next_hover_cursor(OS_Cursor_Beam);
    UI_Box *box = ui_box_make(UI_BoxFlag_DrawBackground |
                              UI_BoxFlag_HotAnimation |
                              UI_BoxFlag_FocusAnimation |
                              UI_BoxFlag_ActiveAnimation |
                              UI_BoxFlag_Clickable |
                              UI_BoxFlag_DrawBorder |
                              UI_BoxFlag_Clip |
                              UI_BoxFlag_AnimateScroll |
                              UI_BoxFlag_Focus |
                              UI_BoxFlag_Commitable,
                              string);

    String8 edit_str = (String8){buffer.val, *string_length};

    comm = ui_comm_from_box(box);
    if(comm.pressed)
    {
      edit_state->cursor = edit_state->mark = (S64)ui_codepoint_index_from_mouse_pos(box, edit_str);
      edit_state->mark = clamp(0, edit_state->mark, (S64)edit_str.size);
    }
    if(comm.dragging)
    {
      edit_state->cursor = (S64)ui_codepoint_index_from_mouse_pos(box, edit_str);
    }
    edit_state->cursor = clamp(0, edit_state->cursor, (S64)edit_str.size);

    if(ui_box_is_focus(box))
    {
      UI_TextActionList text_actions = ui_text_action_list_from_events(ui_frame_arena(), ui_state->os_events);
      for(UI_TextActionNode *node = text_actions.first; node != 0; node = node->next)
      {
        UI_TextAction action = node->action;
        UI_TextOp op = ui_text_of_from_state_and_action(ui_frame_arena(), edit_str, edit_state, &action);
        if(op.copy_string.size)
        {
          os_set_clipboard(op.copy_string);
        }

        String8 new_str = ui_push_replace_string(scratch.arena, edit_str, op.range, buffer, op.replace_string);
        MemoryCopy(buffer.val, new_str.data, new_str.size);
        *string_length = new_str.size;
        edit_str.size = new_str.size;

        edit_state->cursor = clamp(0, op.new_cursor, (S64)edit_str.size);
        edit_state->mark = clamp(0, op.new_mark, (S64)edit_str.size);
      }

      ui_parent(box)
      {
        F32 advance_to_cursor = f_get_advance(ui_top_font_tag(), ui_top_font_size(), str8_prefix(buffer_str8, (U64)edit_state->cursor));
        F32 cursor_extra_offset = ui_em(0.1f, 1).value;
        ui_next_fixed_pos(v2f32(advance_to_cursor + cursor_extra_offset + ui_em(0.2f, 1).value, 0));
        ui_next_pref_height(ui_pct(1, 1));
        ui_next_pref_width(ui_px(3, 1));
        ui_next_rect_color(v4f32(0.9f, 0.9f, 0.9f, 1));
        UI_Box *cursor_box = ui_box_make(UI_BoxFlag_DrawBackground |
                                         UI_BoxFlag_FixedPos |
                                         UI_BoxFlag_AnimatePos,
                                         Str8Lit("CursorBox"));

        {
          F32 advance_to_mark = f_get_advance(ui_top_font_tag(), ui_top_font_size(), str8_prefix(buffer_str8, (U64)edit_state->mark));
          if(edit_state->mark < edit_state->cursor)
          {
            ui_next_fixed_pos(v2f32(advance_to_mark + cursor_extra_offset + ui_em(0.2f, 1).value, 0));
            ui_next_pref_width(ui_px(advance_to_cursor - advance_to_mark, 1));
          }
          else
          {
            ui_next_fixed_pos(v2f32(advance_to_cursor + cursor_extra_offset + ui_em(0.2f, 1).value, 0));
            ui_next_pref_width(ui_px(advance_to_mark - advance_to_cursor, 1));
          }
          ui_next_rect_color(v4f32(0.5f, 0.5f, 0.9f, 0.4f));
          ui_next_pref_height(ui_pct(1, 1));
          UI_Box *mark_box = ui_box_make(UI_BoxFlag_DrawBackground |
                                         UI_BoxFlag_FixedPos,
                                         Str8Lit("MarkBox"));
        }
        // NOTE(hampus): Make sure the cursor is in view
        F32 edit_str_advance = f_get_advance(ui_top_font_tag(), ui_top_font_size(), edit_str);
        F32 padding = ui_em(0.3f, 1).value;

        // NOTE(hampus): Scroll to the left if there is empty
        // space to the right and there are still characters
        // outside on the left iside
        F32 content_size = edit_str_advance + cursor_box->fixed_size.x + padding + cursor_extra_offset;
        F32 adjustment = box->fixed_size.x - (content_size - box->scroll.x);
        adjustment = clamp(0, adjustment, box->scroll.x);
        box->scroll.x -= adjustment;

        Vec2F32 cursor_visiblity_range = v2f32(cursor_box->fixed_pos.x, cursor_box->fixed_pos.x + cursor_box->fixed_size.x);

        cursor_visiblity_range.min = max(0, cursor_visiblity_range.min);
        cursor_visiblity_range.max = max(0, cursor_visiblity_range.max);

        Vec2F32 box_visibility_range = v2f32(box->scroll.x, box->scroll.x + box->fixed_size.x - padding);
        F32 delta_left = cursor_visiblity_range.min - box_visibility_range.min;
        F32 delta_right = cursor_visiblity_range.max - box_visibility_range.max;
        delta_left = Min(delta_left, 0);
        delta_right = max(delta_right, 0);

        box->scroll.x += delta_left + delta_right;
        box->scroll.x = max(0, box->scroll.x);
      }
    }

    ui_parent(box)
    {
      UI_Size spacing = ui_em(0.3f, 1);
      ui_spacer(spacing);
      ui_next_text_padding(v2f32(0, 0));
      ui_next_pref_height(ui_pct(1, 1));
      ui_text(edit_str);
    }
  }

  return (comm);
}
static UI_Comm
ui_line_edit(UI_TextEditState *edit_state, Array<U8> buffer, U64 *string_length, char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  String8 string = str8_push(ui_frame_arena(), fmt, args);
  UI_Comm comm = ui_line_edit(edit_state, buffer, string_length, string);
  va_end(args);
  return (comm);
}

static UI_Box *
ui_begin_named_row(String8 string)
{
  ui_next_child_layout_axis(Axis2_X);
  UI_Box *box = ui_box_make(0, string);
  ui_push_parent(box);
  return box;
}

static void
ui_end_named_row()
{
  ui_pop_parent();
}

static UI_Box *
ui_begin_named_row(char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  String8 string = str8_push(ui_frame_arena(), fmt, args);
  UI_Box *box = ui_begin_named_row(string);
  va_end(args);
  return box;
}

static UI_Box *
ui_begin_row()
{
  UI_Box *box = ui_begin_named_row(Str8Lit(""));
  return box;
}

static void
ui_end_row()
{
  ui_end_named_row();
}

static UI_Box *
ui_begin_named_column(String8 string)
{
  ui_next_child_layout_axis(Axis2_Y);
  UI_Box *box = ui_box_make(0, string);
  ui_push_parent(box);
  return box;
}

static void
ui_end_named_column()
{
  ui_pop_parent();
}

static UI_Box *
ui_begin_named_column(char *fmt, ...)
{
  va_list args = {};
  va_start(args, fmt);
  String8 string = str8_push(ui_frame_arena(), fmt, args);
  UI_Box *box = ui_begin_named_column(string);
  va_end(args);
  return box;
}

static UI_Box *
ui_begin_column()
{
  UI_Box *box = ui_begin_named_column(Str8Lit(""));
  return box;
}

static void
ui_end_column()
{
  ui_end_named_column();
}

static UI_Box *
ui_push_scrollable_container(String8 string, Axis2 axis)
{
  ui_push_seed(ui_hash_from_seed_string(ui_top_seed(), string));
  UI_Box *view_box = ui_box_make(UI_BoxFlag_ViewScroll | UI_BoxFlag_Clip, Str8Lit("ViewBox"));
  ui_push_parent(view_box);
  ui_next_pref_size(axis_flip(axis), ui_fill());
  ui_next_pref_size(axis, ui_children_sum(1));
  ui_next_child_layout_axis(axis);
  UI_Box *content_box = ui_box_make((UI_BoxFlags)(UI_BoxFlag_AllowOverflowX << axis), Str8Lit("ContentBox"));
  ui_push_parent(content_box);
  return view_box;
}

static UI_Comm
ui_pop_scrollable_container()
{
  UI_Box *content_box = ui_pop_parent();
  UI_Box *view_box = ui_pop_parent();
  Axis2 axis = content_box->child_layout_axis;
  UI_Comm view_box_comm = ui_comm_from_box(view_box);
  view_box->scroll[axis] += view_box_comm.scroll[axis];
  view_box->scroll[axis] = clamp(0, view_box->scroll[axis], content_box->fixed_size[axis] - view_box->fixed_size[axis]);
  ui_pop_seed();
  return view_box_comm;
}