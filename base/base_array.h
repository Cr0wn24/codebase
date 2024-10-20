#ifndef BASE_ARRAY_H
#define BASE_ARRAY_H

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

#endif // BASE_ARRAY_H