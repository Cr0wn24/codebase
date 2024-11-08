#ifndef BASE_TEXT_H
#define BASE_TEXT_H

////////////////////////////////////////////////////////////
// hampus: EOL mode types

enum EOLMode
{
 EOLMode_None = -1,

 EOLMode_CRLF,
 EOLMode_LF,
 EOLMode_CR,

 EOLMode_COUNT,
};

////////////////////////////////////////////////////////////
// hampus: Grapheme clusters break types

enum GraphemeClusterBreakKind
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

 GraphemeClusterBreakKind_COUNT
};

#define BLOCK_SIZE 256

struct GraphemeBreakKindTrieBlock
{
 StaticArray<GraphemeClusterBreakKind, BLOCK_SIZE> kinds;
};

struct GraphemeBreakKindTrie
{
 Array<U64> indices;
 Array<GraphemeBreakKindTrieBlock> blocks;
};

////////////////////////////////////////////////////////////
// hampus: EOL mode

[[nodiscard]] static EOLMode get_eol_mode(String8 string);
[[nodiscard]] static String8 get_eol_string(EOLMode mode);
[[nodiscard]] static U64 get_eol_length(EOLMode mode);

////////////////////////////////////////////////////////////
// hampus: Line

[[nodiscard]] static U64 get_next_line_length(String8 string, EOLMode eol_mode);

////////////////////////////////////////////////////////////
// hampus: Grapheme clusters break

[[nodiscard]] static GraphemeClusterBreakKind grapheme_cluster_kind_from_codepoint(U32 cp);
static void grapheme_break_kind_trie_init(Arena *arena, String8 data);

#endif