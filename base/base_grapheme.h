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

struct GraphemeNode
{
  GraphemeNode *next;
  GraphemeNode *prev;
  String8 string;
};

struct GraphemeList
{
  GraphemeNode *first;
  GraphemeNode *last;
  U64 count;
};

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

[[nodiscard]] static GraphemeClusterBreakKind grapheme_cluster_kind_from_codepoint(U32 cp);
static void init_grapheme_break_trie(U64 *indices, U64 indices_count, GraphemeClusterBreakKind *kinds, U64 kinds_count);
[[nodiscard]] static U64 get_next_grapheme_width_in_bytes(String8 string);
[[nodiscard]] static GraphemeList *grapheme_list_from_str8(Arena *arena, String8 string);

#endif // BASE_GRAHPEME_H