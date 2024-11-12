static void *os_memory_reserve(U64 size);
static void os_memory_commit(void *ptr, U64 size);
static void os_memory_decommit(void *ptr, U64 size);
static void os_memory_release(void *ptr, U64 size);

template <typename T>
static DArray<T>
dynamic_array_alloc()
{
  ProfileFunction();
  DArray<T> result = {};
  result.cap = DYNAMIC_ARRAY_DEFAULT_RESERVE_SIZE;
  result.base = (T *)os_memory_reserve(result.cap);
  return result;
}

template <typename T>
static DArray<T>
dynamic_array_alloc(U64 size)
{
  ProfileFunction();
  DArray<T> result = {};
  result.cap = DYNAMIC_ARRAY_DEFAULT_RESERVE_SIZE;
  result.base = (T *)os_memory_reserve(result.cap);
  dynamic_array_resize(result, size);
  return result;
}

template <typename T>
static void
dynamic_array_free(DArray<T> &array)
{
  ProfileFunction();
  os_memory_release(array.base, 0);
  array.cap = 0;
  array.commit_size = 0;
  array.pos = 0;
}

template <typename T>
static U64
darray_count(DArray<T> &array)
{
  U64 result = array.pos / sizeof(T);
  return result;
}

template <typename T>
static U64
darray_count(const DArray<T> &array)
{
  U64 result = array.pos / sizeof(T);
  return result;
}

template <typename T>
static void
dynamic_array_insert(DArray<T> &array, U64 idx, T *val, U64 count)
{
  ProfileFunction();
  U64 clamped_dst_idx = Min(darray_count(array), idx);
  U64 clamped_count = Min(darray_count(array) - clamped_dst_idx, count);
  U64 clamped_size = clamped_count * sizeof(T);
  T *dst = &array.base[clamped_dst_idx];
  MemoryCopy(dst, val, clamped_size);
}

template <typename T>
static void
dynamic_array_resize(DArray<T> &array, U64 new_count)
{
  ProfileFunction();

  U64 new_size_in_bytes = new_count * sizeof(T);
  Assert(new_size_in_bytes <= array.cap);
  U64 size_aligned = round_up_to_power_2_u64(new_size_in_bytes, ARENA_COMMIT_BLOCK_SIZE);
  if(new_size_in_bytes > array.pos)
  {
    if(new_size_in_bytes > array.commit_size)
    {
      U64 next_commit_size = Min(size_aligned, array.cap);
      U64 commit_size = next_commit_size - array.commit_size;
      os_memory_commit((U8 *)array.base + array.commit_size, commit_size);
      ASAN_POISON_MEMORY_REGION((U8 *)array.base + array.commit_size, commit_size);
      array.commit_size = next_commit_size;
    }
    ASAN_UNPOISON_MEMORY_REGION((U8 *)array.base + array.pos, new_size_in_bytes - array.pos);
  }
  else if(new_size_in_bytes < array.pos)
  {
    U64 next_commit_size = Min(size_aligned, array.cap);
    if(next_commit_size < array.commit_size)
    {
      U64 decommit_size = array.commit_size - next_commit_size;
      os_memory_decommit((U8 *)array.base + next_commit_size, decommit_size);
      array.commit_size = next_commit_size;
      ASAN_POISON_MEMORY_REGION((U8 *)array.base + next_commit_size, decommit_size);
    }
  }

  array.pos = new_size_in_bytes;
}

template <typename T>
static void
dynamic_array_move_memory(DArray<T> &array, U64 dst_idx, U64 src_idx, U64 count)
{
  ProfileFunction();
  U64 clamped_dst_idx = Min(darray_count(array), dst_idx);
  U64 clamped_src_idx = Min(darray_count(array), src_idx);
  T *dst = &array.base[clamped_dst_idx];
  T *src = &array.base[clamped_src_idx];
  MemoryMove(dst, src, count * sizeof(T));
}