#ifndef BASE_ATLAS_H
#define BASE_ATLAS_H

typedef U32 AtlasRegionFlags;
enum
{
  AtlasRegionFlag_Taken = (1 << 0),
  AtlasRegionFlag_DescendantTaken = (1 << 1),
};

struct AtlasRegion
{
  AtlasRegion *parent;
  AtlasRegion *next;
  AtlasRegion *prev;
  AtlasRegion *first;
  AtlasRegion *last;
  Vec2U64 max_region_size;
  RectU64 region;
  AtlasRegionFlags flags;
};

struct Atlas
{
  void *memory;
  Vec2U64 dim;
  AtlasRegion *root;
};

[[nodiscard]] function Atlas atlas_make(Arena *arena, Vec2U64 dim);
[[nodiscard]] function RectU64 atlas_region_alloc(Arena *arena, Atlas *atlas, Vec2U64 dim);
function void atlas_region_free(Atlas *atlas, RectU64 region);

#endif // BASE_ATLAS_H