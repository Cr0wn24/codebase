#ifndef ui_BASIC_WIDGETS_H
#define ui_BASIC_WIDGETS_H

struct UI_LineEditState
{
  UI_TextEditState edit_state;
  U8 buffer[256];
  U64 string_length;
};

struct UI_LineEditStatePairNode
{
  UI_LineEditStatePairNode *next;
  UI_LineEditStatePairNode *prev;
  UI_LineEditState v[2];
};

static void ui_spacer(UI_Size size);
static void ui_text(String8 string);
static void ui_text(char *fmt, ...);
static void ui_simple_text(String8 string);
static void ui_simple_text(const char *fmt, ...);
static UI_Comm ui_button(String8 string);
static UI_Comm ui_button(char *fmt, ...);
static UI_Comm ui_check(B32 b32, String8 string);
static UI_Comm ui_check(B32 b32, char *fmt, ...);

static UI_Comm ui_line_edit(UI_TextEditState *edit_state, U8 *buffer, U64 buffer_size, U64 *string_length, String8 string);
static UI_Comm ui_line_edit(UI_TextEditState *edit_state, U8 *buffer, U64 buffer_size, U64 *string_length, char *fmt, ...);

static UI_Box *ui_begin_named_row(String8 string);
static void ui_end_named_row();
static UI_Box *ui_begin_named_row(char *fmt, ...);
static UI_Box *ui_begin_row();
static void ui_end_row();

static UI_Box *ui_begin_named_column(String8 string);
static void ui_end_named_column();
static UI_Box *ui_begin_named_column(char *fmt, ...);
static UI_Box *ui_begin_column();
static void ui_end_column();

static UI_Box *ui_push_scrollable_container(String8 string, Axis2 axis);
static UI_Comm ui_pop_scrollable_container();

#define ui_row() DeferLoop(ui_begin_row(), ui_end_row())
#define ui_row_named(string) DeferLoop(ui_begin_named_row(string), ui_end_named_row())

#define ui_column() DeferLoop(ui_begin_column(), ui_end_column())
#define ui_column_named(string) DeferLoop(ui_begin_named_column(string), ui_end_named_column())

#define ui_scrollable_container(string, axis) DeferLoop(ui_push_scrollable_container(string, axis), ui_pop_scrollable_container())

#define ui_padding(size) DeferLoop(ui_spacer(size), ui_spacer(size))

#endif // ui_BASIC_WIDGETS_H
