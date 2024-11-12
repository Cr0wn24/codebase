#ifndef BASE_GRAHPEME_H
#define BASE_GRAHPEME_H

////////////////////////////////////////////////////////////
// hampus: Grapheme clusters break types

enum GraphemeClusterBreakKind : S32
{
  GraphemeClusterBreakKind_None = -1,

  GraphemeClusterBreakKind_Other,
  GraphemeClusterBreakKind_Control,
  GraphemeClusterBreakKind_CR,
  GraphemeClusterBreakKind_LF,
  GraphemeClusterBreakKind_Extend,
  GraphemeClusterBreakKind_Prepend,
  GraphemeClusterBreakKind_ZWJ,
  GraphemeClusterBreakKind_RI,
  GraphemeClusterBreakKind_L,
  GraphemeClusterBreakKind_V,
  GraphemeClusterBreakKind_T,
  GraphemeClusterBreakKind_LV,
  GraphemeClusterBreakKind_LVT,
  GraphemeClusterBreakKind_SpacingMark,
  GraphemeClusterBreakKind_ExtPict,
  
  GraphemeClusterBreakKind_Any,

  GraphemeClusterBreakKind_COUNT
};

#define BLOCK_SIZE 256

struct GraphemeBreakKindTrieBlock
{
  GraphemeClusterBreakKind kinds[256];
};

struct GraphemeBreakKindTrie
{
  U64 *indices;
  U64 indices_count;
  U64 blocks_count;
  GraphemeBreakKindTrieBlock *blocks;
};

////////////////////////////////////////////////////////////
// hampus: Grapheme clusters break

[[nodiscard]] static GraphemeClusterBreakKind grapheme_cluster_kind_from_codepoint(const GraphemeBreakKindTrie &trie, U32 cp);
[[nodiscard]] static GraphemeBreakKindTrie grapheme_break_kind_trie_from_static_memory(U64 *indices, U64 indices_count, GraphemeClusterBreakKind *kinds, U64 kinds_count);
[[nodiscard]] static U64 get_next_grapheme_width_in_bytes(const GraphemeBreakKindTrie &trie, String8 string);

#endif // BASE_GRAHPEME_H