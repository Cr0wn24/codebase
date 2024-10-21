function void *os_memory_reserve(U64 size);
function void os_memory_commit(void *ptr, U64 size);
function void os_memory_decommit(void *ptr, U64 size);
function void os_memory_release(void *ptr, U64 size);

template <typename T>
Slice<T>
slice_make(T *val, U64 count)
{
  Slice<T> result = {};
  result.val = val;
  result.count = count;
  return result;
}

template <typename T>
Slice<T>
slice_make(Arena *arena, U64 count)
{
  Slice<T> result = {};
  result.val = (T *)arena_push(arena, count * sizeof(T));
  result.count = count;
  return result;
}

template <typename T>
Slice<T>
slice_make_no_zero(Arena *arena, U64 count)
{
  Slice<T> result = {};
  result.val = (T *)arena_push_no_zero(arena, count * sizeof(T));
  result.count = count;
  return result;
}

template <typename T, U64 N>
U64
array_count(Array<T, N> array)
{
  U64 result = N;
  return result;
}

template <typename T>
U64
array_count(Slice<T> array)
{
  U64 result = array.count;
  return result;
}

template <typename T>
DynamicArray<T>
dynamic_array_alloc(void)
{
  DynamicArray<T> result = {};
  result.cap = DYNAMIC_ARRAY_DEFAULT_RESERVE_SIZE;
  result.base = (T *)os_memory_reserve(result.cap);
  return result;
}

template <typename T>
U64
array_count(DynamicArray<T> &array)
{
  U64 result = array.pos / sizeof(T);
  return result;
}

template <typename T>
void
dynamic_array_insert(DynamicArray<T> &array, U64 idx, T *val, U64 count)
{
  U64 clamped_dst_idx = min(array_count(array), idx);
  U64 clamped_count = min(array_count(array) - clamped_dst_idx, count) * sizeof(T);
  U64 clamped_size = clamped_count * sizeof(T);
  T *dst = &array.base[clamped_dst_idx];
  memory_copy(dst, val, clamped_size);
}

template <typename T>
void
dynamic_array_push_back(DynamicArray<T> &array, T *val, U64 count)
{
  ASSERT((array.pos + count * sizeof(T)) < array.cap);
  array.pos += count * sizeof(T);
  if(array.pos > array.commit_size)
  {
    U64 pos_aligned = round_up_to_power_2_u64(array.pos, DYNAMIC_ARRAY_DEFAULT_COMMIT_SIZE);
    U64 next_commit_pos = min(pos_aligned, array.cap);
    U64 commit_size = next_commit_pos - array.commit_pos;
    os_memory_commit(array.memory + array.commit_pos, commit_size);
    ASAN_POISON_MEMORY_REGION((U8 *)array.memory + array.commit_pos, commit_size);
    array.commit_pos = next_commit_pos;
  }
  memory_copy(array.base, val, count * sizeof(T));
}

template <typename T>
void
dynamic_array_push_back(DynamicArray<T> &array, T val)
{
  dynamic_array_push_back(array, &val, 1);
}

template <typename T>
void
dynamic_array_resize(DynamicArray<T> &array, U64 new_count)
{
  U64 new_size_in_bytes = new_count * sizeof(T);
  ASSERT(new_size_in_bytes <= array.cap);
  U64 size_aligned = round_up_to_power_2_u64(new_size_in_bytes, ARENA_COMMIT_BLOCK_SIZE);
  if(new_size_in_bytes > array.pos)
  {
    if(new_size_in_bytes > array.commit_size)
    {
      U64 next_commit_size = min(size_aligned, array.cap);
      U64 commit_size = next_commit_size - array.commit_size;
      os_memory_commit(array.base + array.commit_size, commit_size);
      ASAN_POISON_MEMORY_REGION((U8 *)array.base + array.commit_size, commit_size);
      array.commit_size = next_commit_size;
    }
    ASAN_UNPOISON_MEMORY_REGION(array.base + array.pos, new_size_in_bytes - array.pos);
  }
  else if(new_size_in_bytes < array.pos)
  {
    U64 next_commit_size = min(size_aligned, array.cap);
    if(next_commit_size < array.commit_size)
    {
      U64 decommit_size = array.commit_size - next_commit_size;
      os_memory_decommit(array.base + next_commit_size, decommit_size);
      array.commit_size = next_commit_size;
      ASAN_POISON_MEMORY_REGION(array.base + next_commit_size, decommit_size);
    }
  }

  array.pos = new_size_in_bytes;
}

template <typename T>
void
dynamic_array_move_memory(DynamicArray<T> &array, U64 dst_idx, U64 src_idx, U64 count)
{
  U64 clamped_dst_idx = min(array_count(array), dst_idx);
  U64 clamped_src_idx = min(array_count(array), src_idx);
  T *dst = &array.base[clamped_dst_idx];
  T *src = &array.base[clamped_src_idx];
  memory_move(dst, src, count * sizeof(T));
}