static Atlas
atlas_make(Arena *arena, Vec2U64 dim)
{
  Atlas result = {};
  result.dim = dim;
  result.memory = push_array<U8>(arena, dim.x * dim.y * 4);
  AtlasRegion *root = push_array<AtlasRegion>(arena, 1);
  root->region.min = v2u64(0, 0);
  root->region.max = dim;
  root->max_region_size = dim;
  result.root = root;
  return result;
}

static RectU64
atlas_region_alloc(Arena *arena, Atlas *atlas, Vec2U64 dim)
{
  RectU64 result = {};
  U64 required_dim = max(dim.x, dim.y);

  // hampus: Find the best fit

  AtlasRegion *node = atlas->root;
  for(;;)
  {
    Vec2U64 region_dim = dim_r4u64(node->region);
    if(region_dim.x >= required_dim &&
       region_dim.x < (required_dim * 2) &&
       !(node->flags & AtlasRegionFlag_DescendantTaken)) // TODO(hampus): Is this needed?
    {
      break;
    }

    // hampus: Insert children

    if(node->first == 0)
    {
      Vec2U64 corners[Corner_COUNT][2] =
      {
        v2u64(0, 0),
        v2u64(region_dim.x / 2, region_dim.y / 2),
        v2u64(region_dim.x / 2, 0),
        v2u64(region_dim.x, region_dim.y / 2),
        v2u64(region_dim.x / 2, region_dim.y / 2),
        v2u64(region_dim.x, region_dim.y),
        v2u64(0, region_dim.y / 2),
        v2u64(region_dim.x / 2, region_dim.y),
      };

      for(U64 i = 0; i < Corner_COUNT; ++i)
      {
        AtlasRegion *child = push_array<AtlasRegion>(arena, 1);
        child->parent = node;
        child->region.min = node->region.min + corners[i][0];
        child->region.max = node->region.min + corners[i][1];
        child->max_region_size = dim_r4u64(child->region);
        DLLPushBack(node->first, node->last, child);
      }
    }

    // hampus: Find the next node to check

    for(AtlasRegion *child = node->first; child != 0; child = child->next)
    {
      if(child->flags & AtlasRegionFlag_Taken)
      {
        continue;
      }

      if(child->max_region_size.x >= required_dim)
      {
        node = child;
        break;
      }
    }
  }

  Assert(!(node->flags & AtlasRegionFlag_DescendantTaken));
  Assert(!(node->flags & AtlasRegionFlag_Taken));

  node->flags |= AtlasRegionFlag_Taken;

  // hampus: Mark all parent regions that a descendant has been taken

  for(AtlasRegion *parent = node->parent; parent != 0; parent = parent->parent)
  {
    Vec2U64 max = {};
    for(AtlasRegion *n = parent->first; n != 0; n = n->next)
    {
      if(!(n->flags & AtlasRegionFlag_Taken))
      {
        max.x = max(max.x, n->max_region_size.x);
        max.y = max(max.y, n->max_region_size.y);
      }
    }
    parent->flags |= AtlasRegionFlag_DescendantTaken;
    parent->max_region_size = max;
  }

  Assert(node);

  result.min = node->region.min;
  result.max = node->region.min + dim;
  return result;
}

static void
atlas_region_free(Atlas *atlas, RectU64 region)
{
  AtlasRegion *node = atlas->root;

  // hampus: Find the region inside the atlas

  for(;;)
  {
    if(node->region.min.x == region.min.x &&
       node->region.min.y == region.min.y &&
       node->region.max.x == region.max.x &&
       node->region.max.y == region.max.y)
    {
      Assert(node->flags &= AtlasRegionFlag_Taken);
      node->flags ^= AtlasRegionFlag_Taken;
      break;
    }

    for(AtlasRegion *n = node->first; n != 0; n = n->next)
    {
      if(n->region.min.x <= region.min.x &&
         n->region.min.y <= region.min.y &&
         n->region.max.x >= region.max.x &&
         n->region.max.y >= region.max.y)
      {
        node = n;
        break;
      }
    }
  }

  // hampus: Update descendant flags of the parents and the max region size

  for(AtlasRegion *parent = node->parent; parent != 0; parent = parent->parent)
  {
    Vec2U64 max = {};
    for(AtlasRegion *n = parent->first; n != 0; n = n->next)
    {
      if(!(n->flags & AtlasRegionFlag_Taken))
      {
        max.x = max(max.x, n->max_region_size.x);
        max.y = max(max.y, n->max_region_size.y);
      }
    }
    parent->flags |= AtlasRegionFlag_DescendantTaken;
    parent->max_region_size = max;
  }
}