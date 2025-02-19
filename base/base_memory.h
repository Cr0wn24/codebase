#ifndef BASE_MEMORY_H
#define BASE_MEMORY_H

#if COMPILER_CL
#  pragma warning(push, 0)
#elif COMPILER_CLANG
#endif

#include <sanitizer/asan_interface.h>

#if COMPILER_CL
#  pragma warning(pop)
#elif COMPILER_CLANG
#endif

//////////////////////////////
// NOTE(hampus): Defines

#define ARENA_DEFAULT_RESERVE_SIZE Gigabytes(2)
#define ARENA_COMMIT_BLOCK_SIZE Kilobytes(64)

//////////////////////////////
// NOTE(hampus): Arena types

struct Arena
{
  U8 *memory;
  U64 cap;
  U64 pos;
  U64 commit_pos;
};

struct TempArena
{
  Arena *arena;
  U64 pos;
  TempArena(Arena *_arena);
  ~TempArena();
};

//////////////////////////////
// NOTE(hampus): Base functions

[[nodiscard]] static Arena *arena_alloc();
static void arena_free(Arena *arena);

[[nodiscard]] static void *arena_push(Arena *arena, U64 size);
[[nodiscard]] static void *arena_push_no_zero(Arena *arena, U64 size);
static void arena_pop_to(Arena *arena, U64 pos);
static void arena_pop_amount(Arena *arena, U64 amount);

static void arena_align(Arena *arena, U64 power);
static void arena_align_no_zero(Arena *arena, U64 power);

//////////////////////////////
// NOTE(hampus): Ring

#define ring_write_struct(b, sz, o, s) ring_write(b, sz, o, Str8Struct(s))
#define ring_read_struct(b, sz, o, s) ring_read(b, sz, o, Str8Struct(s))

[[nodiscard]] static U64 ring_write(U8 *base, U64 size, U64 offset, String8 string);
[[nodiscard]] static U64 ring_read(U8 *base, U64 size, U64 offset, String8 string);

//////////////////////////////
// NOTE(hampus): Macro wrappers

template <typename T>
[[nodiscard]] static T *
push_array(Arena *arena, U64 count)
{
  T *result = (T *)arena_push(arena, sizeof(T) * count);
  return result;
}

template <typename T>
[[nodiscard]] static T *
push_array_no_zero(Arena *arena, U64 count)
{
  T *result = (T *)arena_push_no_zero(arena, sizeof(T) * count);
  return result;
}

#define MemoryZero(dst, size) memset((dst), 0, (size))
#define MemoryZeroStruct(dst) MemoryZero((dst), sizeof(*(dst)))
#define MemoryZeroArray(dst) MemoryZero((dst), sizeof(dst))
#define MemoryZeroTyped(dst, count) MemoryZero((dst), sizeof(*dst) * count)

#define MemoryMatch(a, b, size) (!memcmp((a), (b), size))
#define MemoryMatchTyped(a, b, count) (!memcmp((a), (b), sizeof(*(a)) * (count)))

#define MemoryMove(dst, source, size) memmove((dst), (source), size)

#define MemoryCopy(dst, source, size) memcpy((U8 *)(dst), (U8 *)(source), size)
#define MemoryCopyStruct(dst, source) MemoryCopy((dst), (source), Min(sizeof(*(dst)), sizeof(*(source))))
#define MemoryCopyArray(dst, source) MemoryCopy((dst), (source), Min(sizeof(dst), sizeof(source)))
#define MemoryCopyTyped(dst, source, count) MemoryCopy((U8 *)(dst), (U8 *)(source), sizeof(*(dst)) * count)

#define ArenaClear(arena) arena_pop_to(arena, 0)

#endif // BASE_MEMORY_H
