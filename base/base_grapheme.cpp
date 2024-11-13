static GraphemeBreakKindTrie grapheme_break_trie;

////////////////////////////////////////////////////////////
// hampus: Grapheme clusters break

static GraphemeClusterBreakKind
grapheme_cluster_kind_from_codepoint(U32 cp)
{
  GraphemeClusterBreakKind result = GraphemeClusterBreakKind_None;
  U64 block_offset = grapheme_break_trie.indices[cp / BLOCK_SIZE];
  result = grapheme_break_trie.blocks[block_offset].kinds[cp % BLOCK_SIZE];
  return result;
}

static void
init_grapheme_break_trie(U64 *indices, U64 indices_count, GraphemeClusterBreakKind *kinds, U64 kinds_count)
{
  grapheme_break_trie.indices = indices;
  grapheme_break_trie.blocks_count = kinds_count / BLOCK_SIZE;
  grapheme_break_trie.blocks = (GraphemeBreakKindTrieBlock *)kinds;
}

static GraphemeList *
grapheme_list_from_str8(Arena *arena, String8 string)
{
  GraphemeList *result = push_array<GraphemeList>(arena, 1);
  while(string.size != 0)
  {
    U64 next_grapheme_byte_width = get_next_grapheme_width_in_bytes(string);
    GraphemeNode *n = push_array<GraphemeNode>(arena, 1);
    n->string = str8_prefix(string, next_grapheme_byte_width);
    DLLPushBack(result->first, result->last, n);
    string = str8_skip(string, next_grapheme_byte_width);
    result->count += 1;
  }
  return result;
}

static U64
get_next_grapheme_width_in_bytes(String8 string)
{
  struct GraphemeClusterBoundaryRule
  {
    GraphemeClusterBreakKind lhs;
    GraphemeClusterBreakKind rhs;
    B32 should_join;
  };

  TempArena scratch = GetScratch(0, 0);

  GraphemeClusterBoundaryRule rules[] =
  {
   // hampus: GB3
   {GraphemeClusterBreakKind_CR, GraphemeClusterBreakKind_LF, true},

   // hampus: GB4
   {GraphemeClusterBreakKind_Control, GraphemeClusterBreakKind_Any, false},
   {GraphemeClusterBreakKind_CR, GraphemeClusterBreakKind_Any, false},
   {GraphemeClusterBreakKind_LF, GraphemeClusterBreakKind_Any, false},

   // hampus: GB5
   {GraphemeClusterBreakKind_Any, GraphemeClusterBreakKind_Control, false},
   {GraphemeClusterBreakKind_Any, GraphemeClusterBreakKind_CR, false},
   {GraphemeClusterBreakKind_Any, GraphemeClusterBreakKind_LF, false},

   // hampus: GB6
   {GraphemeClusterBreakKind_L, GraphemeClusterBreakKind_L, true},
   {GraphemeClusterBreakKind_L, GraphemeClusterBreakKind_V, true},
   {GraphemeClusterBreakKind_L, GraphemeClusterBreakKind_LV, true},
   {GraphemeClusterBreakKind_L, GraphemeClusterBreakKind_LVT, true},

   // hampus: GB7
   {GraphemeClusterBreakKind_LV, GraphemeClusterBreakKind_V, true},
   {GraphemeClusterBreakKind_LV, GraphemeClusterBreakKind_T, true},
   {GraphemeClusterBreakKind_V, GraphemeClusterBreakKind_V, true},
   {GraphemeClusterBreakKind_V, GraphemeClusterBreakKind_T, true},

   // hampus: GB8
   {GraphemeClusterBreakKind_LVT, GraphemeClusterBreakKind_T, true},
   {GraphemeClusterBreakKind_T, GraphemeClusterBreakKind_T, true},

   // hampus: GB9
   {GraphemeClusterBreakKind_Any, GraphemeClusterBreakKind_Extend, true},
   {GraphemeClusterBreakKind_Any, GraphemeClusterBreakKind_ZWJ, true},

   // hampus: GB9a
   {GraphemeClusterBreakKind_Any, GraphemeClusterBreakKind_SpacingMark, true},

   // hampus: GB9b
   {GraphemeClusterBreakKind_Prepend, GraphemeClusterBreakKind_Any, true},

   // hampus: GB11
   {GraphemeClusterBreakKind_Extend, GraphemeClusterBreakKind_ZWJ, true},
   {GraphemeClusterBreakKind_ZWJ, GraphemeClusterBreakKind_ExtPict, true},
  };

  U64 result = 0;

  B32 should_join = true;
  while(should_join)
  {
    should_join = false;

    GraphemeClusterBreakKind leading_break_kind = GraphemeClusterBreakKind_None;
    if(string.size != 0)
    {
      StringDecode leading_decode = string_decode_utf8(string.data, string.size);
      leading_break_kind = grapheme_cluster_kind_from_codepoint(leading_decode.codepoint);
      result += leading_decode.size;
      string = str8_skip(string, leading_decode.size);
    }

    GraphemeClusterBreakKind trailing_break_kind = GraphemeClusterBreakKind_None;
    if(string.size != 0)
    {
      StringDecode trailing_decode = string_decode_utf8(string.data, string.size);
      trailing_break_kind = grapheme_cluster_kind_from_codepoint(trailing_decode.codepoint);
    }

    if(trailing_break_kind != GraphemeClusterBreakKind_None && trailing_break_kind != GraphemeClusterBreakKind_None)
    {
      for(U64 rule_idx = 0; rule_idx < ArrayCount(rules); ++rule_idx)
      {
        const GraphemeClusterBoundaryRule &rule = rules[rule_idx];
        if(rule.lhs == leading_break_kind && rule.rhs == GraphemeClusterBreakKind_Any)
        {
          should_join = rule.should_join;
          break;
        }
        else if(rule.lhs == GraphemeClusterBreakKind_Any && rule.rhs == trailing_break_kind)
        {
          should_join = rule.should_join;
          break;
        }
        else if(rule.lhs == leading_break_kind && rule.rhs == trailing_break_kind)
        {
          should_join = rule.should_join;
          break;
        }
      }
    }
  }

  return result;
}