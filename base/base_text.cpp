static GraphemeBreakKindTrie grapheme_break_kind_trie = {};

////////////////////////////////////////////////////////////
// hampus: EOL mode

static EOLMode
get_eol_mode(String8 string)
{
  EOLMode eol_mode = EOLMode_None;
  ProfileScope("Getting EOL mode");
  for(; string.size != 0;)
  {
    if(string[0] == '\n')
    {
      eol_mode = EOLMode_LF;
    }
    else if(string[0] == '\r')
    {
      if(string.size > 1)
      {
        if(string[1] == '\n')
        {
          eol_mode = EOLMode_CRLF;
        }
      }
      if(eol_mode != EOLMode_CRLF)
      {
        eol_mode = EOLMode_CR;
      }
    }
    if(eol_mode != EOLMode_None)
    {
      break;
    }
    string = str8_skip(string, 1);
  }

  if(eol_mode == EOLMode_None)
  {
    eol_mode = EOLMode_CRLF;
  }

  return eol_mode;
}

static String8
get_eol_string(EOLMode mode)
{
  String8 result = {};
  switch(mode)
  {
    case EOLMode_LF:
    {
      static U8 u8s[] = {'\n'};
      result.data = u8s;
      result.size = 1;
    }
    break;
    case EOLMode_CRLF:
    {
      static U8 u8s[] = {'\r', '\n'};
      result.data = u8s;
      result.size = 2;
    }
    break;
    case EOLMode_CR:
    {
      static U8 u8s[] = {'\r'};
      result.data = u8s;
      result.size = 1;
    }
    break;
      invalid_case;
  }
  return result;
}

static U64
get_eol_length(EOLMode mode)
{
  String8 string = get_eol_string(mode);
  U64 result = string.size;
  return string.size;
}

////////////////////////////////////////////////////////////
// hampus: Line

static U64
get_next_line_length(String8 string, EOLMode eol_mode)
{
  U64 result = string.size;
  String8 at = string;
  while(at.size != 0)
  {
    if(str8_match(str8_prefix(at, get_eol_length(eol_mode)), get_eol_string(eol_mode)))
    {
      result = (at.data - string.data) + get_eol_length(eol_mode);
      break;
    }
    at = str8_skip(at, 1);
  }
  return result;
}

////////////////////////////////////////////////////////////
// hampus: Grapheme clusters break

static GraphemeClusterBreakKind
grapheme_cluster_kind_from_codepoint(U32 cp)
{
  GraphemeClusterBreakKind result = {};
  U64 block_offset = grapheme_break_kind_trie.indices[cp / BLOCK_SIZE];
  result = grapheme_break_kind_trie.blocks[block_offset].kinds[cp % BLOCK_SIZE];
  return result;
}

static void
grapheme_break_kind_trie_init(Arena *arena, String8 data)
{
  TempArena scratch = GetScratch(&arena, 1);

  static StaticArray<String8, GraphemeClusterBreakKind_COUNT> grapheme_cluster_kind_string_table = {};
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
  Array<GraphemeClusterBreakKind> kinds = array_make<GraphemeClusterBreakKind>(arena, 1114112);
  {
    // hampus: get eol mode
    EOLMode eol_mode = get_eol_mode(data);

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

        if(at_cp)
        {
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

  StaticArray<BlockSlot, 256> block_slots = {};

  Array<U64> indices = array_make<U64>(arena, 4352);

  U64 blocks_needed = 0;

  for(U64 block_idx = 0; block_idx < array_count(indices); ++block_idx)
  {
    GraphemeClusterBreakKind *block = &kinds[block_idx * BLOCK_SIZE];

    // hampus: map the block to a block node

    U64 hash = hash_from_string(Str8Struct(block));
    U64 slot_idx = hash % array_count(block_slots);
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

  grapheme_break_kind_trie.indices = indices;
  grapheme_break_kind_trie.blocks = array_make<GraphemeBreakKindTrieBlock>(arena, blocks_needed);

  for(U64 block_slot_idx = 0; block_slot_idx < array_count(block_slots); ++block_slot_idx)
  {
    BlockSlot *slot = &block_slots[block_slot_idx];
    for(BlockNode *n = slot->first; n != 0; n = n->next)
    {
      MemoryCopyTyped(grapheme_break_kind_trie.blocks[n->idx].kinds.val, n->kinds, BLOCK_SIZE);
    }
  }

  for(U32 cp = 0; cp <= 0x10FFFF; ++cp)
  {
    GraphemeClusterBreakKind kind = grapheme_cluster_kind_from_codepoint(cp);
    GraphemeClusterBreakKind correct = kinds[cp];
    Assert(kind == correct);
  }
}