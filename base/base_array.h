#ifndef BASE_ARRAY_H
#define BASE_ARRAY_H

#define DYNAMIC_ARRAY_DEFAULT_RESERVE_SIZE gigabytes(1)
#define DYNAMIC_ARRAY_DEFAULT_COMMIT_SIZE megabytes(1)

template <typename T>
struct Array
{
  T *val;
  U64 count;

  [[nodiscard]] T &
  operator[](U64 idx)
  {
    ASSERT(idx < count);
    return val[idx];
  }
};

template <typename T>
[[nodiscard]] static Array<T> array_make(T *val, U64 count);

template <typename T>
[[nodiscard]] static Array<T> array_make(Arena *arena, U64 count);

template <typename T>
[[nodiscard]] static Array<T> array_make_no_zero(Arena *arena, U64 count);

template <typename T, U64 N>
struct StaticArray
{
  T val[N];

  [[nodiscard]] T &
  operator[](U64 idx)
  {
    ASSERT_ALWAYS(idx < N);
    return val[idx];
  }

  [[nodiscard]] volatile T &
  operator[](U64 idx) volatile
  {
    ASSERT_ALWAYS(idx < N);
    return val[idx];
  }

  [[nodiscard]]
  operator Array<T>()
  {
    return array_make<T>(val, N);
  }
};

template <typename T, U64 N>
[[nodiscard]] static U64 array_count(StaticArray<T, N> array);

template <typename T>
[[nodiscard]] static U64 array_count(Array<T> array);

template <typename T>
struct DynamicArray
{
  T *base;
  U64 pos;         // NOTE(hampus): In bytes
  U64 cap;         // NOTE(hampus): In bytes
  U64 commit_size; // NOTE(hampus): In bytes

  [[nodiscard]] T &
  operator[](U64 idx)
  {
    ASSERT(idx < (pos / sizeof(T)));
    return base[idx];
  }

  [[nodiscard]] const T &
  operator[](U64 idx) const
  {
    ASSERT(idx < (pos / sizeof(T)));
    return base[idx];
  }

  [[nodiscard]]
  operator Array<T>()
  {
    return array_make<T>(base, pos / sizeof(T));
  }
};

template <typename T>
[[nodiscard]] static DynamicArray<T> dynamic_array_alloc();

template <typename T>
[[nodiscard]] static DynamicArray<T> dynamic_array_alloc(U64 size);

template <typename T>
static void dynamic_array_free(DynamicArray<T> &array);

template <typename T>
[[nodiscard]] static U64 array_count(DynamicArray<T> &array);

template <typename T>
[[nodiscard]] static U64 array_count(const DynamicArray<T> &array);

template <typename T>
static void dynamic_array_insert(DynamicArray<T> &array, U64 idx, T *val, U64 count);

template <typename T>
static void dynamic_array_resize(DynamicArray<T> &array, U64 new_count);

template <typename T>
static void dynamic_array_move_memory(DynamicArray<T> &array, U64 dst_idx, U64 src_idx, U64 count);

#endif // BASE_ARRAY_H