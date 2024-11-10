#ifndef BASE_STRING_H
#define BASE_STRING_H

struct Arena;

//////////////////////////////
// NOTE(hampus): Basic string types

struct String8
{
  U8 *data;
  U64 size;

  [[nodiscard]] U8 &
  operator[](U64 idx)
  {
    Assert(idx < size);
    U8 &result = data[idx];
    return result;
  }
};

struct String16
{
  U16 *data;
  U64 size;

  [[nodiscard]] U16 &
  operator[](U64 idx)
  {
    Assert(idx < size);
    U16 &result = data[idx];
    return result;
  }
};

struct String32
{
  U32 *data;
  U64 size;
  [[nodiscard]] U32 &
  operator[](U64 idx)
  {
    Assert(idx < size);
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

struct StringDecodeNode
{
  StringDecodeNode *next;
  StringDecodeNode *prev;
  StringDecode v;
};

//////////////////////////////
// NOTE(hampus): String8

#define Str8Lit(literal) str8((U8 *)(literal), sizeof(literal) - 1)
#define Str8Comp(literal) (U8 *)literal, sizeof(literal) - 1

#define str8_expand(string) safe_s32_from_u64(string.size), string.data
#define Str8Struct(s) \
  (String8) { (U8 *)(s), sizeof(*s) }

[[nodiscard]] static String8 str8(U8 *data, U64 size);
[[nodiscard]] static String8 str8(Arena *arena, String8 string);
[[nodiscard]] static String8 str8_range(U8 *start, U8 *opl);
[[nodiscard]] static String8 str8_cstr(char *data);
[[nodiscard]] static String8 str8_lower(Arena *arena, String8 string);

[[nodiscard]] static String8 str8_prefix(String8 string, U64 size);
[[nodiscard]] static String8 str8_postfix(String8 string, U64 size);
[[nodiscard]] static String8 str8_skip(String8 string, U64 size);
[[nodiscard]] static String8 str8_chop(String8 string, U64 size);
[[nodiscard]] static String8 str8_substr8(String8 string, U64 start, U64 size);
[[nodiscard]] static String8 str8_skip_to_char(String8 string, U8 ch);

[[nodiscard]] static String8 str8_push(Arena *arena, String8 string);
[[nodiscard]] static String8 str8_push(Arena *arena, char *cstr, va_list args);
[[nodiscard]] static String8 str8_push(Arena *arena, char *cstr, ...);
[[nodiscard]] static String8 str8_push(Arena *arena, const char *cstr, ...);

[[nodiscard]] static B32 str8_match(String8 a, String8 b);
static B32 str8_first_index_of(String8 string, U32 codepoint, U64 *result_idx);
static B32 str8_last_index_of(String8 string, U32 codepoint, U64 *result_idx);
static B32 str8_find_substr8(String8 string, String8 substring, U64 *result_idx);
[[nodiscard]] static String8List str8_split_by_codepoints(Arena *arena, String8 string, String8 codepoints);

static void str8_list_push(String8List *list, String8 string, String8Node *node);
static void str8_list_push(Arena *arena, String8List *list, String8 string);
static void str8_list_push(Arena *arena, String8List *list, char *fmt, ...);
[[nodiscard]] static String8 str8_join(Arena *arena, String8List *list);
[[nodiscard]] static String8 str8_append(Arena *arena, String8 first, String8 last);
[[nodiscard]] static String8 str8_chop_last_slash(String8 string);
[[nodiscard]] static String8 str8_skip_last_slash(String8 string);

//////////////////////////////
// NOTE(hampus): String16

[[nodiscard]] static String16 str16(U16 *data, U64 size);
[[nodiscard]] static String16 str16_copy(Arena *arena, String16 string);
[[nodiscard]] static wchar_t *cstr16_from_str16(Arena *arena, String16 string);

//////////////////////////////
// NOTE(hampus): String32

[[nodiscard]] static String32 str32(U32 *data, U64 size);
[[nodiscard]] static String32 str32_range(U32 *start, U32 *opl);
[[nodiscard]] static String32 str32_copy(Arena *arena, String32 string);
[[nodiscard]] static String32 str32_prefix(String32 string, U64 size);
[[nodiscard]] static String32 str32_postfix(String32 string, U64 size);
[[nodiscard]] static String32 str32_skip(String32 string, U64 size);
[[nodiscard]] static String32 str32_chop(String32 string, U64 size);
[[nodiscard]] static B32 str32_match(String32 a, String32 b);

//////////////////////////////
// NOTE(hampus): Encoding & decoding

[[nodiscard]] static StringDecode string_decode_utf8(U8 *string, U64 size);
static U64 string_encode_utf8(U8 *dst, U32 codepoint);
[[nodiscard]] static StringDecode string_decode_utf16(U16 *string, U64 size);
static U64 string_encode_utf16(wchar_t *dst, U32 codepoint);

//////////////////////////////
// NOTE(hampus): Conversion between UTF

[[nodiscard]] static String32 str32_from_str8(Arena *arena, String8 string);
[[nodiscard]] static String8 str8_from_str32(Arena *arena, String32 string);
[[nodiscard]] static String16 str16_from_str8(Arena *arena, String8 string);
[[nodiscard]] static String8 str8_from_str16(Arena *arena, String16 string);
[[nodiscard]] static String8 str8_from_cstr16(Arena *arena, U16 *string);

[[nodiscard]] static char *cstr_from_str8(Arena *arena, String8 string);
[[nodiscard]] static wchar_t *cstr16_from_str8(Arena *arena, String8 string);
[[nodiscard]] static String16 cstr16_from_str32(Arena *arena, String32 string);

[[nodiscard]] static String16 str16_from_str32(Arena *arena, String32 string);

//////////////////////////////
// NOTE(hampus): String to integers

static U64 u64_hex_from_str8(String8 string, U64 *dst);
static U32 u32_hex_from_str8(String8 string, U32 *dst);

static U64 u64_from_str8(String8 string, U64 *dst);
static U64 u32_from_str8(String8 string, U32 *dst);
static U64 u16_from_str8(String8 string, U16 *dst);
static U64 u8_from_str8(String8 string, U8 *dst);

static U64 s64_from_str8(String8 string, S64 *dst);
static U64 s32_from_str8(String8 string, S32 *dst);
static U64 s16_from_str8(String8 string, S16 *dst);
static U64 s8_from_str8(String8 string, S8 *dst);

//////////////////////////////
// NOTE(hampus): String to floating point

static U64 f64_from_str8(String8 string, F64 *dst);

//////////////////////////////
// NOTE(hampus): Character functions

[[nodiscard]] static B32 is_num(U8 ch);

//////////////////////////////
// NOTE(hampus): C-String functions

static String8 cstr_format(U8 *buffer, U64 buffer_size, char *cstr, ...);
[[nodiscard]] static U64 cstr_length(char *cstr);

//////////////////////////////
// NOTE(hampus): Hash

[[nodiscard]] static U64 hash_from_string(String8 string);

#endif // BASE_STRING_H