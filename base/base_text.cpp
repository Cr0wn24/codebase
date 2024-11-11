////////////////////////////////////////////////////////////
// hampus: Grapheme clusters break

static GraphemeClusterBreakKind
grapheme_cluster_kind_from_codepoint(const GraphemeBreakKindTrie &trie, U32 cp)
{
  GraphemeClusterBreakKind result = {};
  U64 block_offset = trie.indices[cp / BLOCK_SIZE];
  result = trie.blocks[block_offset].kinds[cp % BLOCK_SIZE];
  return result;
}

static GraphemeBreakKindTrie
grapheme_break_kind_trie_from_static_memory(U64 *indices, U64 indices_count, GraphemeClusterBreakKind *kinds, U64 kinds_count)
{
  GraphemeBreakKindTrie result = {};
  result.indices = indices;
  result.blocks_count = kinds_count / BLOCK_SIZE;
  result.blocks = (GraphemeBreakKindTrieBlock *)kinds;
  return result;
}