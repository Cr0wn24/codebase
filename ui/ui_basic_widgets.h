#ifndef ui_BASIC_WIDGETS_H
#define ui_BASIC_WIDGETS_H

struct UI_LineEditState
{
  UI_TextEditState edit_state;
  StaticArray<U8, 256> buffer;
  U64 string_length;
};

struct UI_LineEditStatePairNode
{
  UI_LineEditStatePairNode *next;
  UI_LineEditStatePairNode *prev;
  StaticArray<UI_LineEditState, 2> v;
};

function void ui_spacer(UI_Size size);
function void ui_text(String8 string);
function void ui_text(char *fmt, ...);
function UI_Comm ui_button(String8 string);
function UI_Comm ui_button(char *fmt, ...);
function UI_Comm ui_check(B32 b32, String8 string);
function UI_Comm ui_check(B32 b32, char *fmt, ...);

function UI_Comm ui_line_edit(UI_TextEditState *edit_state, Array<U8> buffer, U64 *string_length, String8 string);
function UI_Comm ui_line_edit(UI_TextEditState *edit_state, Array<U8> buffer, U64 *string_length, char *fmt, ...);

function UI_Box *ui_begin_named_row(String8 string);
function void ui_end_named_row(void);
function UI_Box *ui_begin_named_row(char *fmt, ...);
function UI_Box *ui_begin_row(void);
function void ui_end_row(void);

function UI_Box *ui_begin_named_column(String8 string);
function void ui_end_named_column(void);
function UI_Box *ui_begin_named_column(char *fmt, ...);
function UI_Box *ui_begin_column(void);
function void ui_end_column(void);

function UI_Box *ui_push_scrollable_container(String8 string, Axis2 axis);
function UI_Comm ui_pop_scrollable_container(void);

#define ui_row() defer_loop(ui_begin_row(), ui_end_row())
#define ui_row_named(string) defer_loop(ui_begin_named_row(string), ui_end_named_row())

#define ui_column() defer_loop(ui_begin_column(), ui_end_column())
#define ui_column_named(string) defer_loop(ui_begin_named_column(string), ui_end_named_column())

#define ui_scrollable_container(string, axis) defer_loop(ui_push_scrollable_container(string, axis), ui_pop_scrollable_container())

#define ui_padding(size) defer_loop(ui_spacer(size), ui_spacer(size))

#endif // ui_BASIC_WIDGETS_H
