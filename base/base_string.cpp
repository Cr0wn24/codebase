#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_STATIC
#if COMPILER_CL
#  pragma warning(push, 0)
#endif
#include "third_party/stb/stb_sprintf.h"

#if COMPILER_CL
#  pragma warning(pop)
#endif

//////////////////////////////
// NOTE(hampus): String8

static String8
str8(U8 *data, U64 size)
{
  String8 result = {};
  result.data = data;
  result.size = size;
  return result;
}

static String8
str8(Arena *arena, String8 string)
{
  String8 result = {};
  result.size = string.size;
  result.data = push_array_no_zero<U8>(arena, result.size);
  memory_copy(result.data, string.data, result.size);
  return result;
}

static String8
str8_range(U8 *start, U8 *opl)
{
  String8 result = {};
  result.data = start;
  result.size = (U64)(opl - start);
  return result;
}

static String8
str8_cstr(char *data)
{
  String8 result = {};
  result.data = (U8 *)data;
  while(*data)
  {
    result.size++;
    data++;
  }
  return result;
}

static String8
str8_lower(Arena *arena, String8 string)
{
  String8 result = {};
  result.size = string.size;
  result.data = push_array_no_zero<U8>(arena, string.size);
  for(U64 i = 0; i < string.size; ++i)
  {
    U8 ch = string[i];
    if(ch >= 'A' && ch <= 'Z')
    {
      ch += ('a' - 'A');
    }
    result.data[i] = ch;
  }
  return result;
}

static String8
str8_upper(Arena *arena, String8 string)
{
  String8 result = {};
  return result;
}

static String8
str8_prefix(String8 string, U64 size)
{
  String8 result = {};
  U64 clamped_size = min(size, string.size);
  result.data = string.data;
  result.size = clamped_size;

  return result;
}

static String8
str8_postfix(String8 string, U64 size)
{
  String8 result = {};
  U64 clamped_size = min(size, string.size);
  result.data = string.data + string.size - clamped_size;
  result.size = clamped_size;
  return result;
}

static String8
str8_skip(String8 string, U64 size)
{
  String8 result = {};
  U64 clamped_size = min(size, string.size);
  result.data = string.data + clamped_size;
  result.size = string.size - clamped_size;
  return result;
}

static String8
str8_chop(String8 string, U64 size)
{
  String8 result = {};
  U64 clamped_size = min(size, string.size);
  result.data = string.data;
  result.size = string.size - clamped_size;
  return result;
}

static String8
str8_substr8(String8 string, U64 start, U64 size)
{
  String8 result = {};
  U64 clamped_start = min(start, string.size);
  U64 clamped_size = min(size, string.size - clamped_start);
  result.data = string.data + clamped_start;
  result.size = clamped_size;
  return result;
}

static String8
str8_skip_to_char(String8 string, U8 ch)
{
  String8 result = string;
  U64 ch_idx = 0;
  if(str8_first_index_of(string, ch, &ch_idx))
  {
    result = str8_prefix(result, ch_idx);
  }
  return result;
}

static String8
str8_push(Arena *arena, String8 string)
{
  String8 result = {};
  U8 *data = push_array_no_zero<U8>(arena, string.size);
  memory_copy(data, string.data, string.size);
  result.data = data;
  result.size = string.size;
  return result;
}

static String8
str8_push(Arena *arena, char *cstr, va_list args)
{
  String8 result = {};
  va_list format_args;
  va_copy(format_args, args);
  U64 needed_size = (U64)stbsp_vsnprintf(0, 0, cstr, args);
  result.data = push_array_no_zero<U8>(arena, needed_size + 1);
  result.size = needed_size;
  stbsp_vsnprintf((char *)result.data, (int)needed_size + 1, cstr, format_args);
  va_end(format_args);
  return result;
}

static String8
str8_push(Arena *arena, char *cstr, ...)
{
  va_list args = {};
  va_start(args, cstr);
  String8 result = str8_push(arena, cstr, args);
  va_end(args);
  return result;
}

static String8
str8_push(Arena *arena, const char *cstr, ...)
{
  va_list args = {};
  va_start(args, cstr);
  String8 result = str8_push(arena, (char *)cstr, args);
  va_end(args);
  return result;
}

static B32
str8_match(String8 a, String8 b)
{
  B32 result = true;
  if(a.size == b.size)
  {
    if(a.data != b.data)
    {
      U8 *a_ptr = a.data;
      U8 *b_ptr = b.data;
      U8 *a_opl = a.data + a.size;
      U8 *b_opl = b.data + b.size;
      while(a_ptr < a_opl && b_ptr < b_opl)
      {
        StringDecode a_decode = string_decode_utf8(a_ptr, (U64)(a_opl - a_ptr));
        StringDecode b_decode = string_decode_utf8(b_ptr, (U64)(b_opl - b_ptr));
        a_ptr += a_decode.size;
        b_ptr += b_decode.size;
        if(a_decode.codepoint != b_decode.codepoint)
        {
          result = false;
          break;
        }
      }
    }
  }
  else
  {
    result = false;
  }
  return result;
}

static B32
str8_first_index_of(String8 string, U32 codepoint, U64 *result_idx)
{
  B32 found = false;
  U8 *ptr = string.data;
  U8 *opl = string.data + string.size;
  while(ptr < opl)
  {
    StringDecode decode = string_decode_utf8(ptr, (U64)(ptr - opl));
    if(decode.codepoint == codepoint)
    {
      found = true;
      *result_idx = (U64)(ptr - string.data);
      break;
    }
    ptr += decode.size;
  }
  return (found);
}

static B32
str8_last_index_of(String8 string, U32 codepoint, U64 *result_idx)
{
  B32 found = false;
  U64 last_idx = 0;
  U8 *ptr = string.data;
  U8 *opl = string.data + string.size;
  while(opl > ptr)
  {
    StringDecode decode = string_decode_utf8(opl, (U64)(ptr - opl));
    if(decode.codepoint == codepoint)
    {
      found = true;
      last_idx = (U64)(ptr - string.data);
    }
    opl -= decode.size;
  }
  if(found)
  {
    *result_idx = last_idx;
  }
  return (found);
}

static B32
str8_find_substr8(String8 string, String8 substring, U64 *result_idx)
{
  B32 result = true;
  S64 first = -1;
  for(U64 i = 0; i < string.size; ++i)
  {
    if(substring.size <= (string.size - i))
    {
      for(U64 j = 0; j < substring.size; ++j)
      {
        if(string[i + j] != substring[j])
        {
          first = -1;
          break;
        }
        else
        {
          if(first == -1)
          {
            first = (S64)i;
          }
        }
      }
      if(first != -1)
      {
        break;
      }
    }
    else
    {
      result = false;
      break;
    }
  }
  if(first != -1)
  {
    *result_idx = (U64)first;
  }

  return result;
}

static String8List
str8_split_by_codepoints(Arena *arena, String8 string, String8 codepoints)
{
  String8List result = {};

  U8 *last_split_point = string.data;
  U8 *string_ptr = string.data;
  U8 *string_opl = string.data + string.size;

  while(string_ptr < string_opl)
  {
    StringDecode string_decode = string_decode_utf8(string_ptr, (U64)(string_ptr - string_opl));

    U8 *codepoint_ptr = codepoints.data;
    U8 *codepoint_opl = codepoints.data + codepoints.size;
    while(codepoint_ptr < codepoint_opl)
    {
      StringDecode codepoint_decode = string_decode_utf8(codepoint_ptr, (U64)(codepoint_opl - codepoint_ptr));

      if(string_decode.codepoint == codepoint_decode.codepoint)
      {
        str8_list_push(arena, &result, str8_range(last_split_point, string_ptr));
        last_split_point = string_ptr + string_decode.size;
        break;
      }

      codepoint_ptr += codepoint_decode.size;
    }

    string_ptr += string_decode.size;
  }

  if(last_split_point < string_opl)
  {
    str8_list_push(arena, &result, str8_range(last_split_point, string_opl));
  }

  return result;
}

static void
str8_list_push(String8List *list, String8 string, String8Node *node)
{
  node->v = string;
  dll_push_back(list->first, list->last, node);
  ++list->count;
  list->total_size += string.size;
}

static void
str8_list_push(Arena *arena, String8List *list, String8 string)
{
  String8Node *node = push_array<String8Node>(arena, 1);
  str8_list_push(list, string, node);
}

static void
str8_list_push(Arena *arena, String8List *list, char *fmt, ...)
{
  String8Node *node = push_array<String8Node>(arena, 1);
  va_list args = {};
  va_start(args, fmt);
  String8 string = str8_push(arena, fmt, args);
  va_end(args);
  str8_list_push(list, string, node);
}

static String8
str8_join(Arena *arena, String8List *list)
{
  U64 size = list->total_size;
  U8 *data = push_array_no_zero<U8>(arena, size);

  U8 *ptr = data;
  for(String8Node *node = list->first; node; node = node->next)
  {
    memory_copy(ptr, node->v.data, node->v.size);
    ptr += node->v.size;
  }

  return (str8(data, size));
}

static String8
str8_append(Arena *arena, String8 first, String8 last)
{
  String8 result = {};
  result.size = first.size + last.size;
  result.data = push_array_no_zero<U8>(arena, result.size);
  memory_copy(result.data, first.data, first.size);
  memory_copy(result.data + first.size, last.data, last.size);
  return result;
}

static String8
str8_chop_last_slash(String8 string)
{
  if(string.size > 0)
  {
    U8 *ptr = string.data + string.size - 1;
    for(; ptr >= string.data; ptr -= 1)
    {
      if(*ptr == '/' || *ptr == '\\')
      {
        break;
      }
    }
    if(ptr >= string.data)
    {
      string.size = (U64)(ptr - string.data);
    }
    else
    {
      string.size = 0;
    }
  }
  return (string);
}

static String8
str8_skip_last_slash(String8 string)
{
  if(string.size > 0)
  {
    U8 *ptr = string.data + string.size - 1;
    for(; ptr >= string.data; ptr -= 1)
    {
      if(*ptr == '/' || *ptr == '\\')
      {
        break;
      }
    }
    if((ptr + 1) >= string.data)
    {
      string.size = (U64)(string.size - ((ptr + 1) - string.data));
      string.data = ptr + 1;
    }
    else
    {
      string.size = 0;
    }
  }
  return (string);
}

//////////////////////////////
// NOTE(hampus): String16

static String16
str16(U16 *data, U64 size)
{
  String16 result = {};
  result.data = data;
  result.size = size;
  return result;
}

//////////////////////////////
// NOTE(hampus): String32

static String32
str32(U32 *data, U64 size)
{
  String32 result = {};
  result.data = data;
  result.size = size;
  return result;
}

static B32
str32_contains_only(String32 string, Array<U32> slice)
{
  B32 result = true;
  for(U64 idx = 0; idx < string.size; ++idx)
  {
    U32 u32 = string[idx];
    B32 is_part_of_array = false;
    for(U64 array_idx = 0; array_idx < slice.count; ++array_idx)
    {
      if(u32 == slice[array_idx])
      {
        is_part_of_array = true;
        break;
      }
    }

    if(!is_part_of_array)
    {
      result = false;
      break;
    }
  }
  return result;
}

static B32
str32_match(String32 a, String32 b)
{
  B32 result = true;

  if(a.size == b.size)
  {
    if(a.data != b.data)
    {
      U32 *ptr_a = a.data;
      U32 *opl_a = a.data + a.size;
      U32 *ptr_b = b.data;
      while(ptr_a < opl_a)
      {
        if(*ptr_a != *ptr_b)
        {
          result = false;
          break;
        }
        ptr_a += 1;
        ptr_b += 1;
      }
    }
  }
  else
  {
    result = false;
  }

  return result;
}

static String32
str32_range(U32 *start, U32 *opl)
{
  String32 result = {};
  result.data = start;
  result.size = (U64)(opl - start);
  return result;
}

static String32
str32_copy(Arena *arena, String32 string)
{
  String32 result = {};
  U32 *data = push_array_no_zero<U32>(arena, string.size);
  memory_copy(data, string.data, string.size * sizeof(U32));
  result.data = data;
  result.size = string.size;
  return result;
}

static String32
str32_append(Arena *arena, String32 first, String32 last)
{
  String32 result = {};
  result.size = first.size + last.size;
  result.data = push_array_no_zero<U32>(arena, result.size);
  memory_copy(result.data, first.data, first.size * sizeof(U32));
  memory_copy(result.data + first.size, last.data, last.size * sizeof(U32));
  return result;
}

static String32
str32_push(Arena *arena, U32 *data, U64 size)
{
  String32 result = {};
  result.size = size;
  result.data = push_array_no_zero<U32>(arena, size);
  memory_copy(result.data, data, size * sizeof(U32));
  return result;
}

static String32
str32_prefix(String32 string, U64 size)
{
  String32 result = {};
  U64 clamped_size = min(size, string.size);
  result.data = string.data;
  result.size = clamped_size;
  return result;
}

static String32
str32_postfix(String32 string, U64 size)
{
  String32 result = {};
  U64 clamped_size = min(size, string.size);
  result.data = string.data + string.size - clamped_size;
  result.size = clamped_size;
  return result;
}

static String32
str32_skip(String32 string, U64 size)
{
  String32 result = {};
  U64 clamped_size = min(size, string.size);
  result.data = string.data + clamped_size;
  result.size = string.size - clamped_size;
  return result;
}

static String32
str32_chop(String32 string, U64 size)
{
  String32 result = {};
  U64 clamped_size = min(size, string.size);
  result.data = string.data;
  result.size = string.size - clamped_size;
  return result;
}

static String32
str32_skip_to_codepoint(String32 string, U32 cp)
{
  String32 result = string;
  for(U64 cp_idx = 0; cp_idx < string.size; ++cp_idx)
  {
    if(string[cp_idx] == cp)
    {
      result = str32_prefix(result, cp_idx);
    }
  }
  return result;
}

static String32
str32_skip_to_string(String32 haystack, String32 needle)
{
  String32 result = haystack;
  for(U64 cp_idx = 0; cp_idx < result.size; ++cp_idx)
  {
    String32 string = str32(haystack.data, needle.size);
    if(str32_match(string, needle))
    {
      result.size = cp_idx;
      break;
    }
    haystack = str32_skip(haystack, 1);
  }
  return result;
}

//////////////////////////////
// NOTE(hampus): Encoding & decoding

static StringDecode
string_decode_utf8(U8 *string, U64 cap)
{
  static U8 length[] =
  {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    2, 2, 2, 2,
    3, 3,
    4,
    5};

  static U32 first_byte_mask[] = {0, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x01FF, 0x03FF};
  static U8 final_shift[] = {0, 18, 12, 6, 0};

  StringDecode result = {};
  if(cap > 0)
  {
    result.codepoint = '#';
    result.size = 1;

    U8 byte = string[0];
    U8 l = length[byte >> 3];
    switch(l)
    {
      case 1:
      {
        result.codepoint = byte;
      }
      break;
      case 2:
      {
        if(2 <= cap)
        {
          U8 cont_byte = string[1];
          if(length[cont_byte >> 3] == 0)
          {
            result.codepoint = (byte & first_byte_mask[5]) << 6;
            result.codepoint |= (cont_byte & first_byte_mask[6]);
            result.size = 2;
          }
        }
      }
      break;
      case 3:
      {
        if(3 <= cap)
        {
          U8 cont_byte[2] = {string[1], string[2]};
          if(length[cont_byte[0] >> 3] == 0 &&
             length[cont_byte[1] >> 3] == 0)
          {
            result.codepoint = (byte & first_byte_mask[4]) << 12;
            result.codepoint |= ((cont_byte[0] & first_byte_mask[6]) << 6);
            result.codepoint |= (cont_byte[1] & first_byte_mask[6]);
            result.size = 3;
          }
        }
      }
      break;
      case 4:
      {
        if(4 <= cap)
        {
          U8 cont_byte[3] = {string[1], string[2], string[3]};
          if(length[cont_byte[0] >> 3] == 0 &&
             length[cont_byte[1] >> 3] == 0 &&
             length[cont_byte[2] >> 3] == 0)
          {
            result.codepoint = (byte & first_byte_mask[3]) << 18;
            result.codepoint |= ((cont_byte[0] & first_byte_mask[6]) << 12);
            result.codepoint |= ((cont_byte[1] & first_byte_mask[6]) << 6);
            result.codepoint |= (cont_byte[2] & first_byte_mask[6]);
            result.size = 4;
          }
        }
      }
      break;
    }
  }

  return result;
}

static U64
string_encode_utf8(U8 *dst, U32 codepoint)
{
  U32 size = 0;
  if(codepoint <= 0x7F)
  {
    dst[0] = (U8)codepoint;
    size = 1;
  }
  else if(codepoint <= 0x7FF)
  {
    dst[0] = (U8)(0xC0 | (codepoint >> 6));
    dst[1] = (U8)(0x80 | (codepoint & 0x3F));
    size = 2;
  }
  else if(codepoint <= 0xFFFF)
  {
    dst[0] = (U8)(0xE0 | (codepoint >> 12));
    dst[1] = (U8)(0x80 | ((codepoint >> 6) & 0x3F));
    dst[2] = (U8)(0x80 | (codepoint & 0x3F));
    size = 3;
  }
  else if(codepoint <= 0x10FFFF)
  {
    dst[0] = (U8)(0xF0 | (codepoint >> 18));
    dst[1] = (U8)(0x80 | ((codepoint >> 12) & 0x3F));
    dst[2] = (U8)(0x80 | ((codepoint >> 6) & 0x3F));
    dst[3] = (U8)(0x80 | (codepoint & 0x3F));
    size = 4;
  }
  else
  {
    dst[0] = '#';
    size = 1;
  }

  return (size);
}

static StringDecode
string_decode_utf16(U16 *string, U64 size)
{
  StringDecode result = {};
  result.codepoint = 0xFFFD;
  result.size = 0;

  if(size == 0)
  {
    return result;
  }

  U16 code_unit = *string++;
  ++result.size;

  if(code_unit < 0xD800 || 0xDFFF < code_unit)
  {
    result.codepoint = code_unit;
  }
  else if(size >= 2)
  {
    U16 lead_surrogate = code_unit;
    code_unit = *string++;

    if(0xD800 <= lead_surrogate && lead_surrogate <= 0xDBFF && 0xDC00 <= code_unit && code_unit <= 0xDFFF)
    {
      result.codepoint = (U32)(0x10000 + ((lead_surrogate - 0xD800) << 10) + (code_unit - 0xDC00));
      ++result.size;
    }
  }

  return result;
}

static U64
string_encode_utf16(U16 *dst, U32 codepoint)
{
  U64 size = 0;
  if(codepoint < 0x10000)
  {
    dst[0] = (U16)codepoint;
    size = 1;
  }
  else
  {
    U32 adjusted_codepoint = codepoint - 0x10000;
    dst[0] = (U16)(0xD800 + (adjusted_codepoint >> 10));
    dst[1] = 0xDC00 + (adjusted_codepoint & 0x03FF);
    size = 2;
  }
  return (size);
}

//////////////////////////////
// NOTE(hampus): Conversion between UTF

static String32
str32_from_str8(Arena *arena, String8 string)
{
  U64 allocated_size = string.size;
  U32 *memory = push_array_no_zero<U32>(arena, allocated_size);

  U32 *dst_ptr = memory;
  U8 *ptr = string.data;
  U8 *opl = string.data + string.size;

  while(ptr < opl)
  {
    StringDecode decode = string_decode_utf8(ptr, (U64)(opl - ptr));
    *dst_ptr++ = decode.codepoint;
    ptr += decode.size;
  }

  U64 string_size = (U64)(dst_ptr - memory);
  U64 unused_size = allocated_size - string_size;
  arena_pop_amount(arena, unused_size * sizeof(*memory));

  String32 result = {};
  result.data = memory;
  result.size = string_size;
  return result;
}

static String8
str8_from_str32(Arena *arena, String32 string)
{
  U64 allocated_size = 4 * string.size;
  U8 *memory = push_array_no_zero<U8>(arena, allocated_size);

  U8 *dst_ptr = memory;
  U32 *ptr = string.data;
  U32 *opl = string.data + string.size;

  while(ptr < opl)
  {
    U32 codepoint = *ptr++;
    U64 size = string_encode_utf8(dst_ptr, codepoint);
    dst_ptr += size;
  }

  U64 string_size = (U64)(dst_ptr - memory);
  U64 unused_size = allocated_size - string_size;
  arena_pop_amount(arena, unused_size * sizeof(*memory));

  String8 result = str8(memory, string_size);
  return result;
}

static String16
str16_from_str8(Arena *arena, String8 string)
{
  U64 allocated_size = string.size;
  U16 *memory = push_array_no_zero<U16>(arena, allocated_size);

  U16 *dst_ptr = memory;
  U8 *ptr = string.data;
  U8 *opl = string.data + string.size;

  while(ptr < opl)
  {
    StringDecode decode = string_decode_utf8(ptr, (U64)(opl - ptr));
    U32 encode_size = (U32)string_encode_utf16(dst_ptr, decode.codepoint);
    dst_ptr += encode_size;
    ptr += decode.size;
  }

  U64 string_size = (U64)(dst_ptr - memory);
  U64 unused_size = allocated_size - string_size;
  arena_pop_amount(arena, unused_size * sizeof(*memory));

  String16 result = str16(memory, string_size);
  return result;
}

static String8
str8_from_str16(Arena *arena, String16 string)
{
  U64 allocated_size = 3 * string.size;
  U8 *memory = push_array_no_zero<U8>(arena, allocated_size);

  U8 *dst_ptr = memory;
  U16 *ptr = string.data;
  U16 *opl = string.data + string.size;

  while(ptr < opl)
  {
    StringDecode decode = string_decode_utf16(ptr, (U64)(opl - ptr));
    U64 encode_size = string_encode_utf8(dst_ptr, decode.codepoint);

    dst_ptr += encode_size;
    ptr += decode.size;
  }

  U64 string_size = (U64)(dst_ptr - memory);
  U64 unused_size = allocated_size - string_size;
  arena_pop_amount(arena, unused_size * sizeof(*memory));

  String8 result = str8(memory, string_size);
  return result;
}

static String8
str8_from_cstr16(Arena *arena, U16 *string)
{
  String16 str16 = {};
  str16.data = string;
  str16.size = 0;
  while(str16.data[str16.size])
  {
    ++str16.size;
  }
  String8 result = str8_from_str16(arena, str16);
  return result;
}

static char *
cstr_from_str8(Arena *arena, String8 string)
{
  U64 allocated_size = string.size + 1;
  U8 *memory = push_array_no_zero<U8>(arena, allocated_size);
  memory_copy(memory, string.data, string.size);
  memory[string.size] = 0;
  return ((char *)memory);
}

static U16 *
cstr16_from_str8(Arena *arena, String8 string)
{
  U64 allocated_size = string.size + 1;
  U16 *memory = push_array_no_zero<U16>(arena, allocated_size);
  U16 *dst_ptr = memory;
  U8 *ptr = string.data;
  U8 *opl = string.data + string.size;
  while(ptr < opl)
  {
    StringDecode decode = string_decode_utf8(ptr, (U64)(opl - ptr));
    U64 encode_size = string_encode_utf16(dst_ptr, decode.codepoint);
    dst_ptr += encode_size;
    ptr += decode.size;
  }
  *dst_ptr = 0;
  U64 string_size = (U64)(dst_ptr - memory);
  U64 unused_size = allocated_size - string_size - 1;
  arena_pop_amount(arena, unused_size * sizeof(*memory));
  return (memory);
}

static String16
cstr16_from_str32(Arena *arena, String32 string)
{
  U64 allocated_size = string.size * 2 + 1;
  U16 *memory = push_array_no_zero<U16>(arena, allocated_size);

  U16 *dst_ptr = memory;
  U32 *ptr = string.data;
  U32 *opl = string.data + string.size;

  while(ptr < opl)
  {
    U64 size = string_encode_utf16(dst_ptr, *ptr);
    dst_ptr += size;
    ptr += 1;
  }

  U64 string_size = (U64)(dst_ptr - memory);
  U64 unused_size = allocated_size - string_size - 1;
  arena_pop_amount(arena, unused_size * sizeof(*memory));

  String16 result = {};
  result.data = memory;
  result.size = string_size;
  return result;
}

//////////////////////////////
// NOTE(hampus): String to integers

static U64
u64_hex_from_str8(String8 string, U64 *dst)
{
  U64 i;
  for(i = 0; i < string.size; ++i)
  {
    U8 ch = string[i];
    if(!(is_num(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')))
    {
      break;
    }
    if(is_num(ch))
    {
      *dst = (*dst) * 16 + ch - '0';
    }
    else if(ch >= 'a' && ch <= 'f')
    {
      *dst = (*dst) * 16 + (ch - 'a' + 10);
    }
    else if(ch >= 'A' && ch <= 'F')
    {
      *dst = (*dst) * 16 + (ch - 'A' + 10);
    }
  }
  return (i);
}

static U64
u64_from_str8(String8 string, U64 *dst)
{
  U64 i;
  for(i = 0; i < string.size; ++i)
  {
    U8 ch = string[i];
    if(!is_num(ch))
    {
      break;
    }
    *dst = (*dst) * 10 + ch - '0';
  }
  return (i);
}

static U64
u32_from_str8(String8 string, U32 *dst)
{
  U64 result = 0;
  U64 temp = 0;
  result = u64_from_str8(string, &temp);
  *dst = (U32)temp;
  return result;
}

static U64
u16_from_str8(String8 string, U16 *dst)
{
  U64 result = 0;
  U64 temp = 0;
  result = u64_from_str8(string, &temp);
  *dst = (U16)temp;
  return result;
}

static U64
u8_from_str8(String8 string, U8 *dst)
{
  U64 result = 0;
  U64 temp = 0;
  result = u64_from_str8(string, &temp);
  *dst = (U8)temp;
  return result;
}

static U64
s64_from_str8(String8 string, S64 *dst)
{
  U64 i = 0;
  B32 negative = false;
  if(string.size > 0)
  {
    if(string[0] == '-')
    {
      negative = true;
      i += 1;
    }
  }
  for(; i < string.size; ++i)
  {
    U8 ch = string[i];
    if(!is_num(ch))
    {
      break;
    }
    *dst = (*dst) * 10 + ch - '0';
  }
  if(negative)
  {
    *dst = -(*dst);
  }
  return (i);
}

static U64
s32_from_str8(String8 string, S32 *dst)
{
  U64 result = 0;
  S64 temp = 0;
  result = s64_from_str8(string, &temp);
  *dst = (S32)temp;
  return result;
}

static U64
s16_from_str8(String8 string, S16 *dst)
{
  U64 result = 0;
  S64 temp = 0;
  result = s64_from_str8(string, &temp);
  *dst = (S16)temp;
  return result;
}

static U64
s8_from_str8(String8 string, S8 *dst)
{
  U64 result = 0;
  S64 temp = 0;
  result = s64_from_str8(string, &temp);
  *dst = (S8)temp;
  return result;
}

//////////////////////////////
// NOTE(hampus): String to floating point

static U64
f64_from_str8(String8 string, F64 *dst)
{
  U64 bytes_read = 0;
  B32 found_sign = false;
  B32 found_dot = false;
  for(; bytes_read < string.size; ++bytes_read)
  {
    U8 ch = string[bytes_read];
    if(ch == '-' || ch == '+')
    {
      if(!found_sign)
      {
        found_sign = true;
      }
      else
      {
        break;
      }
    }
    else if(ch == '.')
    {
      if(!found_dot)
      {
        found_dot = true;
      }
      else
      {
        break;
      }
    }
    else if(ch == 'f')
    {
      bytes_read++;
      break;
    }
    else if(!is_num(ch))
    {
      break;
    }
  }

  TempArena scratch = get_scratch(0, 0);
  char *cstr = cstr_from_str8(scratch.arena, str8_prefix(string, bytes_read));

  *dst = atof(cstr);
  return (bytes_read);
}

//////////////////////////////
// NOTE(hampus): Character functions

static B32
is_num(U8 ch)
{
  B32 result = ('0' <= ch && ch <= '9');
  return result;
}

//////////////////////////////
// NOTE(hampus): C-String functions

static String8
cstr_format(U8 *buffer, U64 buffer_size, char *cstr, va_list args)
{
  TempArena scratch = get_scratch(0, 0);
  String8 string = str8_push(scratch.arena, cstr, args);
  U64 clamped_size = min(string.size, buffer_size);
  memory_copy(buffer, string.data, clamped_size);
  return (str8(buffer, clamped_size));
}

static String8
cstr_format(U8 *buffer, U64 buffer_size, char *cstr, ...)
{
  va_list args;
  va_start(args, cstr);
  String8 string = cstr_format(buffer, buffer_size, cstr, args);
  va_end(args);
  return string;
}

static String8
cstr_format(U8 *buffer, U64 buffer_size, const char *cstr, ...)
{
  va_list args;
  va_start(args, cstr);
  String8 string = cstr_format(buffer, buffer_size, (char *)cstr, args);
  va_end(args);
  return string;
}

static U64
cstr_length(char *cstr)
{
  U64 result = 0;
  while(*cstr++)
  {
    ++result;
  }
  return result;
}