static StaticArray<String8, GraphemeClusterBreakKind_COUNT> grapheme_cluster_kind_string_table =
{
 Str8Comp("Other"),
 Str8Comp("Control"),
 Str8Comp("CR"),
 Str8Comp("LF"),
 Str8Comp("Extend"),
 Str8Comp("Prepend"),
 Str8Comp("ZWJ"),
 Str8Comp("Regional_Indicator"),
 Str8Comp("L"),
 Str8Comp("V"),
 Str8Comp("T"),
 Str8Comp("LV"),
 Str8Comp("LVT"),
 Str8Comp("SpacingMark"),
};

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
   result = IntFromPtr(at.data) - IntFromPtr(string.data) + get_eol_length(eol_mode);
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

 Array<GraphemeClusterBreakKind> kinds = array_make<GraphemeClusterBreakKind>(arena, 1114112);

 EOLMode eol_mode = get_eol_mode(data);

 B32 parsing_codepoints = false;
 for(; data.size != 0;)
 {
  U64 line_length = get_next_line_length(data, eol_mode);
  if(str8_match(str8_prefix(data, 1), Str8Lit("#")) || line_length == get_eol_length(eol_mode))
  {
   // hampus: skip comment
  }
  else
  {
   String8List list = str8_split_by_codepoints(scratch.arena, str8_prefix(data, line_length), Str8Lit(" "));

   // hampus: Parse codepoints
   U32 min_cp = 0;
   U32 max_cp = 0;
   {
    String8 codepoints_string = list.first->v;
    U64 dot_idx = 0;
    if(str8_first_index_of(codepoints_string, '.', &dot_idx))
    {
     u32_hex_from_str8(str8_prefix(codepoints_string, dot_idx), &min_cp);
     u32_hex_from_str8(str8_skip(codepoints_string, dot_idx + 2), &max_cp);
    }
    else
    {
     u32_hex_from_str8(codepoints_string, &min_cp);
     max_cp = min_cp;
    }
   }

   // hampus: Parse cluster break kind
   GraphemeClusterBreakKind cluster_break_kind = GraphemeClusterBreakKind_None;
   {
    String8 cluster_break_kind_string = list.first->next->next->v;
    ForEachEnumVal(GraphemeClusterBreakKind, kind)
    {
     if(str8_match(cluster_break_kind_string, grapheme_cluster_kind_string_table[kind]))
     {
      cluster_break_kind = kind;
     }
    }
    Assert(cluster_break_kind != GraphemeClusterBreakKind_None);
   }

   {
    for(U32 cp = min_cp; cp <= max_cp; ++cp)
    {
     kinds[cp] = cluster_break_kind;
    }
   }
  }
  data = str8_skip(data, line_length);
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