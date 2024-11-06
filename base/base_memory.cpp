static void *os_memory_reserve(U64 size);
static void os_memory_commit(void *ptr, U64 size);
static void os_memory_decommit(void *ptr, U64 size);
static void os_memory_release(void *ptr, U64 size);

//////////////////////////////
// NOTE(hampus): Base functions
static Arena *
arena_alloc()
{
  U8 *memory = (U8 *)os_memory_reserve(ARENA_DEFAULT_RESERVE_SIZE);
  os_memory_commit(memory, ARENA_COMMIT_BLOCK_SIZE);
  Arena *result = (Arena *)memory;
  result->memory = memory;
  result->cap = ARENA_DEFAULT_RESERVE_SIZE;
  result->pos = sizeof(*result);
  result->commit_pos = ARENA_COMMIT_BLOCK_SIZE;
  arena_align(result, 16);
  ASAN_POISON_MEMORY_REGION(result->memory + result->pos, result->commit_pos - result->pos);
  return result;
}

static void
arena_free(Arena *arena)
{
  os_memory_release(arena->memory, arena->cap);
}

static void *
arena_push_no_zero(Arena *arena, U64 size)
{
  void *result = 0;
  if(arena->pos + size <= arena->cap)
  {
    result = arena->memory + arena->pos;
    arena->pos += size;

    if(arena->pos > arena->commit_pos)
    {
      U64 pos_aligned = round_up_to_power_2_u64(arena->pos, ARENA_COMMIT_BLOCK_SIZE);
      U64 next_commit_pos = min(pos_aligned, arena->cap);
      U64 commit_size = next_commit_pos - arena->commit_pos;
      os_memory_commit(arena->memory + arena->commit_pos, commit_size);
      ASAN_POISON_MEMORY_REGION((U8 *)arena->memory + arena->commit_pos, commit_size);
      arena->commit_pos = next_commit_pos;
    }
    ASAN_UNPOISON_MEMORY_REGION(result, size);
  }
  return result;
}

static void
arena_pop_to(Arena *arena, U64 pos)
{
  pos = max(pos, sizeof(Arena));

  if(pos < arena->pos)
  {
    U64 dpos = arena->pos - pos;
    arena->pos = pos;

    ASAN_POISON_MEMORY_REGION((U8 *)arena->memory + arena->pos, dpos);

    U64 pos_aligned = round_up_to_power_2_u64(arena->pos, ARENA_COMMIT_BLOCK_SIZE);
    U64 next_commit_pos = min(pos_aligned, arena->cap);
    if(next_commit_pos < arena->commit_pos)
    {
      U64 decommit_size = arena->commit_pos - next_commit_pos;
      os_memory_decommit(arena->memory + next_commit_pos, decommit_size);
      arena->commit_pos = next_commit_pos;
    }
  }
}

static void
arena_pop_amount(Arena *arena, U64 amount)
{
  arena_pop_to(arena, arena->pos - amount);
}

static void *
arena_push(Arena *arena, U64 size)
{
  void *result = arena_push_no_zero(arena, size);

  MemoryZero(result, size);
  return result;
}

static void
arena_align(Arena *arena, U64 power)
{
  U64 pos_aligned = round_up_to_power_2_u64(arena->pos, power);
  U64 align = pos_aligned - arena->pos;
  if(align)
  {
    void *push_result = arena_push(arena, align);
  }
}

static void
arena_align_no_zero(Arena *arena, U64 power)
{
  U64 pos_aligned = round_up_to_power_2_u64(arena->pos, power);
  U64 align = pos_aligned - arena->pos;
  if(align)
  {
    void *push_result = arena_push_no_zero(arena, align);
  }
}

//////////////////////////////
// NOTE(hampus): Ring

static U64
ring_write(U8 *base, U64 size, U64 offset, String8 string)
{
  offset %= size;
  U64 size0 = min(size - offset, string.size);
  U64 size1 = string.size - size0;
  if(size0 != 0)
  {
    MemoryCopy(base + offset, string.data, size0);
  }

  if(size1 != 0)
  {
    MemoryCopy(base, string.data + size0, size1);
  }
  return (size0 + size1);
}

static U64
ring_read(U8 *base, U64 size, U64 offset, String8 string)
{
  offset %= size;
  U64 size0 = min(size - offset, string.size);
  U64 size1 = string.size - size0;
  if(size0 != 0)
  {
    MemoryCopy(string.data, base + offset, size0);
  }
  if(size1 != 0)
  {
    MemoryCopy(string.data + size0, base, size1);
  }
  return (size0 + size1);
}

//////////////////////////////
// NOTE(hampus): Temp Arena

TempArena::TempArena(Arena *_arena)
{
  arena = _arena;
  pos = arena->pos;
}

TempArena::~TempArena()
{
  arena_pop_to(arena, pos);
}