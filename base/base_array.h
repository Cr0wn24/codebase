#ifndef BASE_ARRAY_H
#define BASE_ARRAY_H

#define DYNAMIC_ARRAY_DEFAULT_RESERVE_SIZE gigabytes(1)
#define DYNAMIC_ARRAY_DEFAULT_COMMIT_SIZE kilobytes(64)

template <typename T>
struct Slice
{
  T *val;
  U64 count;

  T &
  operator[](U64 idx) const
  {
    ASSERT(idx < count);
    return val[idx];
  }
};

template <typename T>
Slice<T> slice_make(T *val, U64 count);

template <typename T>
Slice<T> slice_make(Arena *arena, U64 count);

template <typename T>
Slice<T> slice_make_no_zero(Arena *arena, U64 count);

template <typename T, U64 N>
struct Array
{
  T val[N];

  T &
  operator[](U64 idx)
  {
    ASSERT(idx < N);
    return val[idx];
  }

  volatile T &
  operator[](U64 idx) volatile
  {
    ASSERT(idx < N);
    return val[idx];
  }

  operator Slice<T>()
  {
    return slice_make<T>(val, N);
  }
};

template <typename T, U64 N>
U64 array_count(Array<T, N> array);

template <typename T>
U64 array_count(Slice<T> array);

template <typename T>
struct DynamicArray
{
  T *base;
  U64 pos;         // NOTE(hampus): In bytes
  U64 cap;         // NOTE(hampus): In bytes
  U64 commit_size; // NOTE(hampus): In bytes

  T &
  operator[](U64 idx)
  {
    ASSERT(idx < (pos / sizeof(T)));
    return base[idx];
  }
};

template <typename T>
DynamicArray<T> dynamic_array_alloc(void);

template <typename T>
U64 array_count(DynamicArray<T> &array);

template <typename T>
void dynamic_array_insert(DynamicArray<T> &array, U64 idx, T *val, U64 count);

template <typename T>
void dynamic_array_push_back(DynamicArray<T> &array, T *val, U64 count);

template <typename T>
void dynamic_array_push_back(DynamicArray<T> &array, T val);

template <typename T>
void dynamic_array_resize(DynamicArray<T> &array, U64 new_count);

template <typename T>
void dynamic_array_move_memory(DynamicArray<T> &array, U64 dst_idx, U64 src_idx, U64 count);

#endif // BASE_ARRAY_H