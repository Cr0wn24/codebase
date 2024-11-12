#include "base/base_inc.h"
#include "os/os_inc.h"

#include "base/base_inc.cpp"
#include "os/os_inc.cpp"

struct GetElementValueResult
{
  String8 string;
  U64 bytes_to_skip;
};

static GetElementValueResult
get_element_value(String8 data, String8 element_string)
{
  GetElementValueResult result = {};
  String8 data_copy = data;
  while(data_copy.size != 0)
  {
    if(str8_match(str8_prefix(data_copy, element_string.size), element_string))
    {
      data_copy = str8_skip(data_copy, element_string.size);
      data_copy = str8_skip(data_copy, 2);
      U64 last_quote_idx = 0;
      if(str8_first_index_of(data_copy, '\"', &last_quote_idx))
      {
        result.string = str8_prefix(data_copy, last_quote_idx);
        result.bytes_to_skip = (U64)(result.string.data - data.data) + result.string.size + 1;
        break;
      }
    }
    data_copy = str8_skip(data_copy, 1);
  }
  return result;
}

static GraphemeBreakKindTrie
make_grapheme_trie_from_xml(Arena *arena, String8 data)
{
  TempArena scratch = GetScratch(&arena, 1);

  String8 grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_COUNT] = {};
  grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_Other] = Str8Lit("XX");
  grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_Control] = Str8Lit("CN");
  grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_CR] = Str8Lit("CR");
  grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_LF] = Str8Lit("LF");
  grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_Extend] = Str8Lit("EX");
  grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_Prepend] = Str8Lit("PP");
  grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_ZWJ] = Str8Lit("ZWJ");
  grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_RI] = Str8Lit("RI");
  grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_L] = Str8Lit("L");
  grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_V] = Str8Lit("V");
  grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_T] = Str8Lit("T");
  grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_LV] = Str8Lit("LV");
  grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_LVT] = Str8Lit("LVT");
  grapheme_cluster_kind_string_table[GraphemeClusterBreakKind_SpacingMark] = Str8Lit("SM");

  // hampus: get the grapheme cluster break kinds for all codepoints
  U64 kinds_count = 1114112;
  GraphemeClusterBreakKind *kinds = push_array_no_zero<GraphemeClusterBreakKind>(arena, kinds_count);
  {
    // hampus: fill in cluster break kinds
    for(; data.size != 0;)
    {
      if(str8_match(str8_prefix(data, 1), Str8Lit("<")))
      {
        data = str8_skip(data, 1);

        B32 at_cp = false;

        String8 char_tag = Str8Lit("char");
        String8 reserved_tag = Str8Lit("reserved");
        if(str8_match(str8_prefix(data, char_tag.size), char_tag))
        {
          data = str8_skip(data, char_tag.size + 1);
          at_cp = true;
        }
        else if(str8_match(str8_prefix(data, reserved_tag.size), reserved_tag))
        {
          data = str8_skip(data, reserved_tag.size + 1);
          at_cp = true;
        }

        // hampus: extract GCB out of codepoint

        if(at_cp)
        {
          // hampus: parse codepoint
          Range2U32 cp_range = {};
          {
            B32 got_cp_range = false;
            String8 cp_element_name = Str8Lit("cp");
            String8 first_cp_element_name = Str8Lit("first-cp");
            if(str8_match(str8_prefix(data, cp_element_name.size), cp_element_name))
            {
              data = str8_skip(data, cp_element_name.size);
            }
            else if(str8_match(str8_prefix(data, first_cp_element_name.size), first_cp_element_name))
            {
              data = str8_skip(data, first_cp_element_name.size);
              got_cp_range = true;
            }
            else
            {
              InvalidCodePath;
            }

            Assert(data[0] == '=');
            data = str8_skip(data, 1);
            Assert(data[0] == '\"');
            data = str8_skip(data, 1);

            {
              U64 last_quote_idx = 0;
              if(str8_first_index_of(data, '\"', &last_quote_idx))
              {
                String8 cp_string = str8_prefix(data, last_quote_idx);
                u32_hex_from_str8(cp_string, &cp_range.min);
                data = str8_skip(data, cp_string.size);
                Assert(data[0] == '\"');
                data = str8_skip(data, 1);
                Assert(data[0] == ' ');
                data = str8_skip(data, 1);
              }
              else
              {
                InvalidCodePath;
              }
            }

            if(got_cp_range)
            {
              String8 last_cp_element_name = Str8Lit("last-cp");
              Assert(str8_match(str8_prefix(data, last_cp_element_name.size), last_cp_element_name));
              data = str8_skip(data, last_cp_element_name.size);

              Assert(data[0] == '=');
              data = str8_skip(data, 1);

              Assert(data[0] == '\"');
              data = str8_skip(data, 1);
              U64 last_quote_idx = 0;
              {
                if(str8_first_index_of(data, '\"', &last_quote_idx))
                {
                  String8 cp_string = str8_prefix(data, last_quote_idx);
                  u32_hex_from_str8(cp_string, &cp_range.max);
                  data = str8_skip(data, cp_string.size);
                }
                else
                {
                  InvalidCodePath;
                }
              }
            }
            else
            {
              cp_range.max = cp_range.min;
            }
          }

          // hampus: parse GCB
          GraphemeClusterBreakKind break_kind = GraphemeClusterBreakKind_None;
          {
            String8 gcb_element_name = Str8Lit("GCB");
            GetElementValueResult gcb_element_value = get_element_value(data, gcb_element_name);
            Assert(gcb_element_value.string.size != 0);
            data = str8_skip(data, gcb_element_value.bytes_to_skip);

            ForEachEnumVal(GraphemeClusterBreakKind, kind)
            {
              if(str8_match(grapheme_cluster_kind_string_table[kind], gcb_element_value.string))
              {
                break_kind = kind;
                break;
              }
            }

            Assert(break_kind != GraphemeClusterBreakKind_None);
          }

          {
            String8 gcb_element_name = Str8Lit("ExtPict");
            GetElementValueResult gcb_element_value = get_element_value(data, gcb_element_name);
            Assert(gcb_element_value.string.size != 0);
            data = str8_skip(data, gcb_element_value.bytes_to_skip);

            if(str8_match(gcb_element_value.string, Str8Lit("Y")))
            {
              break_kind = GraphemeClusterBreakKind_ExtPict;
            }
          }

          // hampus: map codepoint -> GCB
          for(U32 cp = cp_range.min; cp <= cp_range.max; ++cp)
          {
            kinds[cp] = break_kind;
          }
        }
      }

      data = str8_skip(data, 1);
    }
  }

  struct BlockNode
  {
    BlockNode *next;
    BlockNode *prev;

    GraphemeClusterBreakKind *kinds;
    U64 idx;
  };

  struct BlockSlot
  {
    BlockNode *first;
    BlockNode *last;
  };

  BlockSlot block_slots[256] = {};
  U64 indices[4352] = {};
  U64 blocks_needed = 0;

  for(U64 block_idx = 0; block_idx < ArrayCount(indices); ++block_idx)
  {
    GraphemeClusterBreakKind *block = &kinds[block_idx * BLOCK_SIZE];

    // hampus: map the block to a block node

    U64 hash = hash_from_string(Str8Struct(block));
    U64 slot_idx = hash % ArrayCount(block_slots);
    BlockSlot *slot = &block_slots[slot_idx];
    BlockNode *n = 0;
    for(n = slot->first; n != 0; n = n->next)
    {
      if(MemoryMatchTyped(n->kinds, block, BLOCK_SIZE))
      {
        break;
      }
    }

    if(n == 0)
    {
      n = push_array<BlockNode>(scratch.arena, 1);
      n->kinds = block;
      n->idx = blocks_needed;
      DLLPushFront(slot->first, slot->last, n);

      blocks_needed += 1;
    }

    indices[block_idx] = n->idx;
  }

  GraphemeBreakKindTrie result = {};

  result.indices_count = ArrayCount(indices);
  result.indices = indices;
  result.blocks_count = blocks_needed;
  result.blocks = push_array_no_zero<GraphemeBreakKindTrieBlock>(arena, result.blocks_count);

  for(U64 block_slot_idx = 0; block_slot_idx < ArrayCount(block_slots); ++block_slot_idx)
  {
    BlockSlot *slot = &block_slots[block_slot_idx];
    for(BlockNode *n = slot->first; n != 0; n = n->next)
    {
      MemoryCopyTyped(result.blocks[n->idx].kinds, n->kinds, BLOCK_SIZE);
    }
  }

  return result;
}

static S32
os_entry_point(String8List args)
{
  TempArena scratch = GetScratch(0, 0);
  String8 grapheme_break_property_file_data = os_file_read(scratch.arena, Str8Lit("../assets/ucd.nounihan.flat.xml"));
  GraphemeBreakKindTrie grapheme_trie = make_grapheme_trie_from_xml(scratch.arena, grapheme_break_property_file_data);
  GraphemeClusterBreakKind kind0 = grapheme_cluster_kind_from_codepoint(0x1F926);
  Assert(kind0 == GraphemeClusterBreakKind_ExtPict);
  GraphemeClusterBreakKind kind1 = grapheme_cluster_kind_from_codepoint(0x1F3FB);
  Assert(kind1 == GraphemeClusterBreakKind_Extend);
  GraphemeClusterBreakKind kind2 = grapheme_cluster_kind_from_codepoint(0x200D);
  Assert(kind2 == GraphemeClusterBreakKind_ZWJ);
  GraphemeClusterBreakKind kind3 = grapheme_cluster_kind_from_codepoint(0x2642);
  Assert(kind3 == GraphemeClusterBreakKind_ExtPict);
  GraphemeClusterBreakKind kind4 = grapheme_cluster_kind_from_codepoint(0xFE0F);
  Assert(kind4 == GraphemeClusterBreakKind_Extend);

  OS_Handle stream_handle = os_file_stream_open(Str8Lit("../src/codebase/grapheme_break_trie_gen/grapheme_break_trie_gen_result.h"));
  if(stream_handle != os_handle_zero())
  {

    {
      String8 string = Str8Lit("#ifndef GRAPHEME_BREAK_TRIE_GEN_RESULT_H\n#define GRAPHEME_BREAK_TRIE_GEN_RESULT_H\n");
      AssertAlways(os_file_stream_write(stream_handle, string));
    }

    U8 buffer[Kilobytes(16)] = {};
    U64 buffer_used_size = 0;
    {
      {
        String8 string = str8_push(scratch.arena, "\nU64 grapheme_trie_indices[%" PRIU64 "] = {", grapheme_trie.indices_count);
        AssertAlways(os_file_stream_write(stream_handle, string));
      }

      for(U64 idx = 0; idx < grapheme_trie.indices_count; ++idx)
      {
        String8 string = {};
        if(idx % 20 == 0)
        {
          string = cstr_format(buffer + buffer_used_size, ArrayCount(buffer) - buffer_used_size, "0x%x,\n", grapheme_trie.indices[idx]);
        }
        else
        {
          string = cstr_format(buffer + buffer_used_size, ArrayCount(buffer) - buffer_used_size, "0x%x, ", grapheme_trie.indices[idx]);
        }

        buffer_used_size += string.size;
        if((buffer_used_size + 6) > ArrayCount(buffer))
        {
          AssertAlways(os_file_stream_write(stream_handle, str8(buffer, buffer_used_size)));
          buffer_used_size = 0;
        }
      }

      AssertAlways(os_file_stream_write(stream_handle, str8(buffer, buffer_used_size)));
      AssertAlways(os_file_stream_write(stream_handle, Str8Lit("\n};")));
    }

    buffer_used_size = 0;

    {
      {
        String8 string = str8_push(scratch.arena, "\nS32 grapheme_trie_blocks_kinds[%" PRIU64 "] = {", grapheme_trie.blocks_count * BLOCK_SIZE);
        AssertAlways(os_file_stream_write(stream_handle, string));
      }

      U64 idx = 0;
      for(U64 block_idx = 0; block_idx < grapheme_trie.blocks_count; ++block_idx)
      {
        for(U64 kind_idx = 0; kind_idx < BLOCK_SIZE; ++kind_idx)
        {
          String8 string = {};
          if(idx % 20 == 0)
          {
            string = cstr_format(buffer + buffer_used_size, ArrayCount(buffer) - buffer_used_size, "0x%x,\n", grapheme_trie.blocks[block_idx].kinds[kind_idx]);
          }
          else
          {
            string = cstr_format(buffer + buffer_used_size, ArrayCount(buffer) - buffer_used_size, "0x%x, ", grapheme_trie.blocks[block_idx].kinds[kind_idx]);
          }

          buffer_used_size += string.size;
          if((buffer_used_size + 6) > ArrayCount(buffer))
          {
            AssertAlways(os_file_stream_write(stream_handle, str8(buffer, buffer_used_size)));
            buffer_used_size = 0;
          }
          idx += 1;
        }
      }

      AssertAlways(os_file_stream_write(stream_handle, str8(buffer, buffer_used_size)));
      buffer_used_size = 0;
      AssertAlways(os_file_stream_write(stream_handle, Str8Lit("\n};")));
    }

    {
      String8 string = Str8Lit("\n#endif // GRAPHEME_BREAK_TRIE_GEN_RESULT_H");
      AssertAlways(os_file_stream_write(stream_handle, string));
    }

    if(!os_file_stream_close(stream_handle))
    {
      InvalidCodePath;
    }
  }
  else
  {
    InvalidCodePath;
  }

  return 0;
}
