#ifndef BASE_ARRAY_H
#define BASE_ARRAY_H

#define DYNAMIC_ARRAY_DEFAULT_RESERVE_SIZE Gigabytes(1)
#define DYNAMIC_ARRAY_DEFAULT_COMMIT_SIZE Megabytes(1)

template <typename T>
struct DArray
{
  T *base;
  U64 pos;         // NOTE(hampus): In bytes
  U64 cap;         // NOTE(hampus): In bytes
  U64 commit_size; // NOTE(hampus): In bytes

  [[nodiscard]] T &
  operator[](U64 idx)
  {
    Assert(idx < (pos / sizeof(T)));
    return base[idx];
  }

  [[nodiscard]] const T &
  operator[](U64 idx) const
  {
    Assert(idx < (pos / sizeof(T)));
    return base[idx];
  }
};

template <typename T>
[[nodiscard]] static DArray<T> dynamic_array_alloc();

template <typename T>
[[nodiscard]] static DArray<T> dynamic_array_alloc(U64 size);

template <typename T>
static void dynamic_array_free(DArray<T> &array);

template <typename T>
[[nodiscard]] static U64 darray_count(DArray<T> &array);

template <typename T>
[[nodiscard]] static U64 darray_count(const DArray<T> &array);

template <typename T>
static void dynamic_array_insert(DArray<T> &array, U64 idx, T *val, U64 count);

template <typename T>
static void dynamic_array_resize(DArray<T> &array, U64 new_count);

template <typename T>
static void dynamic_array_move_memory(DArray<T> &array, U64 dst_idx, U64 src_idx, U64 count);

#endif // BASE_ARRAY_H