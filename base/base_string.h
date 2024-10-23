#ifndef BASE_STRING_H
#define BASE_STRING_H

struct Arena;

//////////////////////////////
// NOTE(hampus): Basic string types

struct String8
{
 U8 *data;
 U64 size;

 U8 &
 operator[](U64 idx)
 {
  ASSERT(idx < size);
  U8 &result = data[idx];
  return result;
 }
};

struct String16
{
 U16 *data;
 U64 size;

 U16 &
 operator[](U64 idx)
 {
  ASSERT(idx < size);
  U16 &result = data[idx];
  return result;
 }
};

struct String32
{
 U32 *data;
 U64 size;
 U32 &
 operator[](U64 idx)
 {
  ASSERT(idx < size);
  U32 &result = data[idx];
  return result;
 }
};

//////////////////////////////
// NOTE(hampus): String list

struct String8Node
{
 String8Node *next;
 String8Node *prev;
 String8 v;
};

struct String8List
{
 String8Node *first;
 String8Node *last;
 U64 count;
 U64 total_size;
};

struct String32Node
{
 String32Node *next;
 String32Node *prev;
 String32 v;
};

struct String32List
{
 String32Node *first;
 String32Node *last;
 U64 count;
 U64 total_size;
};

//////////////////////////////
// NOTE(hampus): String decoding & encoding types

struct StringDecode
{
 U32 codepoint;
 U64 size;
};

//////////////////////////////
// NOTE(hampus): String8

#define str8_lit(literal) str8((U8 *)(literal), sizeof(literal) - 1)
#define str8_comp(literal)                           \
 {                                                   \
  .data = (U8 *)literal, .size = sizeof(literal) - 1 \
 }
#define str8_expand(string) safe_s32_from_u64(string.size), string.data
#define str8_struct(s) \
 (String8) { (U8 *)(s), sizeof(*s) }

function String8 str8(U8 *data, U64 size);
function String8 str8(Arena *arena, String8 string);
function String8 str8_range(U8 *start, U8 *opl);
function String8 str8_cstr(char *data);
function String8 str8_lower(Arena *arena, String8 string);

function String8 str8_prefix(String8 string, U64 size);
function String8 str8_postfix(String8 string, U64 size);
function String8 str8_skip(String8 string, U64 size);
function String8 str8_chop(String8 string, U64 size);
function String8 str8_substr8(String8 string, U64 start, U64 size);
function String8 str8_skip_to_char(String8 string, U8 ch);

function String8 str8_push(Arena *arena, String8 string);
function String8 str8_push(Arena *arena, char *cstr, va_list args);
function String8 str8_push(Arena *arena, char *cstr, ...);

function B32 str8_match(String8 a, String8 b);
function B32 str8_first_index_of(String8 string, U32 codepoint, U64 *result_idx);
function B32 str8_last_index_of(String8 string, U32 codepoint, U64 *result_idx);
function B32 str8_find_substr8(String8 string, String8 substring, U64 *result_idx);
function String8List str8_split_by_codepoints(Arena *arena, String8 string, String8 codepoints);

function void str8_list_push(String8List *list, String8 string, String8Node *node);
function void str8_list_push(Arena *arena, String8List *list, String8 string);
function void str8_list_push(Arena *arena, String8List *list, char *fmt, ...);
function String8 str8_join(Arena *arena, String8List *list);
function String8 str8_append(Arena *arena, String8 first, String8 last);
function String8 str8_chop_last_slash(String8 string);
function String8 str8_skip_last_slash(String8 string);

//////////////////////////////
// NOTE(hampus): String16

function String16 str16(U16 *data, U64 size);

//////////////////////////////
// NOTE(hampus): String32

function String32 str32(U32 *data, U64 size);
function String32 str32_range(U32 *start, U32 *opl);
function String32 str32_copy(Arena *arena, String32 string);
function String32 str32_prefix(String32 string, U64 size);
function String32 str32_postfix(String32 string, U64 size);
function String32 str32_skip(String32 string, U64 size);
function String32 str32_chop(String32 string, U64 size);
function B32 str32_match(String32 a, String32 b);

//////////////////////////////
// NOTE(hampus): Encoding & decoding

function StringDecode string_decode_utf8(U8 *string, U64 size);
function U64 string_encode_utf8(U8 *dst, U32 codepoint);
function StringDecode string_decode_utf16(U16 *string, U64 size);
function U64 string_encode_utf16(U16 *dst, U32 codepoint);

//////////////////////////////
// NOTE(hampus): Conversion between UTF

function String32 str32_from_str8(Arena *arena, String8 string);
function String8 str8_from_str32(Arena *arena, String32 string);
function String16 str16_from_str8(Arena *arena, String8 string);
function String8 str8_from_str16(Arena *arena, String16 string);
function String8 str8_from_cstr16(Arena *arena, U16 *string);

function char *cstr_from_str8(Arena *arena, String8 string);
function U16 *cstr16_from_str8(Arena *arena, String8 string);

//////////////////////////////
// NOTE(hampus): String to integers

function U64 u64_hex_from_str8(String8 string, U64 *dst);

function U64 u64_from_str8(String8 string, U64 *dst);
function U64 u32_from_str8(String8 string, U32 *dst);
function U64 u16_from_str8(String8 string, U16 *dst);
function U64 u8_from_str8(String8 string, U8 *dst);

function U64 s64_from_str8(String8 string, S64 *dst);
function U64 s32_from_str8(String8 string, S32 *dst);
function U64 s16_from_str8(String8 string, S16 *dst);
function U64 s8_from_str8(String8 string, S8 *dst);

//////////////////////////////
// NOTE(hampus): String to floating point

function U64 f64_from_str8(String8 string, F64 *dst);

//////////////////////////////
// NOTE(hampus): Character functions

function B32 is_num(U8 ch);

//////////////////////////////
// NOTE(hampus): C-String functions

function String8 cstr_format(U8 *buffer, U64 buffer_size, char *cstr, ...);
function U64 cstr_length(char *cstr);

#endif // BASE_STRING_H