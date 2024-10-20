function void *os_memory_reserve(U64 size);
function void os_memory_commit(void *ptr, U64 size);
function void os_memory_decommit(void *ptr, U64 size);
function void os_memory_release(void *ptr, U64 size);

//////////////////////////////
// NOTE(hampus): Base functions
function Arena *
arena_alloc(void)
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

function void
arena_free(Arena *arena)
{
  os_memory_release(arena->memory, arena->cap);
}

function void *
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

function void
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

function void
arena_pop_amount(Arena *arena, U64 amount)
{
  arena_pop_to(arena, arena->pos - amount);
}

function void *
arena_push(Arena *arena, U64 size)
{
  void *result = arena_push_no_zero(arena, size);
  memory_zero(result, size);
  return result;
}

function void
arena_align(Arena *arena, U64 power)
{
  U64 pos_aligned = round_up_to_power_2_u64(arena->pos, power);
  U64 align = pos_aligned - arena->pos;
  if(align)
  {
    arena_push(arena, align);
  }
}

function void
arena_align_no_zero(Arena *arena, U64 power)
{
  U64 pos_aligned = round_up_to_power_2_u64(arena->pos, power);
  U64 align = pos_aligned - arena->pos;
  if(align)
  {
    arena_push_no_zero(arena, align);
  }
}

//////////////////////////////
// NOTE(hampus): Dynamic Array

function DynamicArray
dynamic_array_alloc(void)
{
  DynamicArray result = {};
  result.cap = ARENA_DEFAULT_RESERVE_SIZE;
  result.base = (U8 *)os_memory_reserve(result.cap);
  return result;
}

function U8 *
dynamic_array_get_(DynamicArray *array, U64 idx)
{
  U8 *result = &array->base[idx];
  return result;
}

function void
dynamic_array_resize_(DynamicArray *array, U64 new_size)
{
  if(new_size <= array->cap)
  {
    U64 size_aligned = round_up_to_power_2_u64(new_size, ARENA_COMMIT_BLOCK_SIZE);
    if(new_size > array->size)
    {
      if(new_size > array->commit_size)
      {
        U64 next_commit_size = min(size_aligned, array->cap);
        U64 commit_size = next_commit_size - array->commit_size;
        os_memory_commit(array->base + array->commit_size, commit_size);
        ASAN_POISON_MEMORY_REGION((U8 *)array->base + array->commit_size, commit_size);
        array->commit_size = next_commit_size;
      }
      ASAN_UNPOISON_MEMORY_REGION(array->base + array->size, new_size - array->size);
    }
    else if(new_size < array->size)
    {
      U64 next_commit_size = min(size_aligned, array->cap);
      if(next_commit_size < array->commit_size)
      {
        U64 decommit_size = array->commit_size - next_commit_size;
        os_memory_decommit(array->base + next_commit_size, decommit_size);
        array->commit_size = next_commit_size;
        ASAN_POISON_MEMORY_REGION(array->base + next_commit_size, decommit_size);
      }
    }

    array->size = new_size;
  }
}

function void
dynamic_array_move_memory_(DynamicArray *array, U64 dst_idx, U64 src_idx, U64 size)
{
  // TODO(hampus): bounds checking of size
  U64 clamped_dst_idx = min(array->size, dst_idx);
  U64 clamped_src_idx = min(array->size, src_idx);
  U8 *dst = array->base + clamped_dst_idx;
  U8 *src = array->base + clamped_src_idx;
  memory_move(dst, src, size);
}

function void
dynamic_array_insert_(DynamicArray *array, U64 dst_idx, U8 *src_data, U64 src_size)
{
  U64 clamped_dst_idx = min(array->size, dst_idx);
  U64 clamped_size = min(array->size - clamped_dst_idx, src_size);
  U8 *dst = array->base + clamped_dst_idx;
  memory_copy(dst, src_data, clamped_size);
}

//////////////////////////////
// NOTE(hampus): Ring

function U64
ring_write(U8 *base, U64 size, U64 offset, String8 string)
{
  offset %= size;
  U64 size0 = min(size - offset, string.size);
  U64 size1 = string.size - size0;
  if(size0 != 0)
  {
    memory_copy(base + offset, string.data, size0);
  }
  if(size1 != 0)
  {
    memory_copy(base, string.data + size0, size1);
  }
  return (size0 + size1);
}

function U64
ring_read(U8 *base, U64 size, U64 offset, String8 string)
{
  offset %= size;
  U64 size0 = min(size - offset, string.size);
  U64 size1 = string.size - size0;
  if(size0 != 0)
  {
    memory_copy(string.data, base + offset, size0);
  }
  if(size1 != 0)
  {
    memory_copy(string.data + size0, base, size1);
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